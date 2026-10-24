#pragma once
#include "core.h"
#include <sqlite3.h>
#include <utility>
class QSqlQuery;
class QSqlRecord;
class QSqlError {
  QString message_;

public:
  QSqlError() = default;
  QSqlError(QString m) : message_(m) {}
  QString text() const { return message_; }
  QString nativeErrorCode() const { return message_; }
  QString driverText() const { return message_; }
  bool isValid() const { return !message_.isEmpty(); }
};
struct PortableDatabase {
  sqlite3 *handle = nullptr;
  QString file, options, error;
  ~PortableDatabase() {
    if (handle)
      sqlite3_close_v2(handle);
  }
};
class QSqlDatabase {
  std::shared_ptr<PortableDatabase> d_;
  struct Connections {
    std::mutex mutex;
    std::map<QString, std::shared_ptr<PortableDatabase>> names;
  };
  static auto &connections() {
    return SimulationContext::state<Connections>();
  }

public:
  QSqlDatabase() = default;
  explicit QSqlDatabase(std::shared_ptr<PortableDatabase> d)
      : d_(std::move(d)) {}
  static QSqlDatabase
  addDatabase(const QString &driver,
              const QString &name = "qt_sql_default_connection") {
    if (driver != "QSQLITE")
      throw std::runtime_error("Only SQLite is supported");
    auto d = std::make_shared<PortableDatabase>();
    auto &registry = connections();
    std::lock_guard<std::mutex> lock(registry.mutex);
    registry.names[name] = d;
    return QSqlDatabase(d);
  }
  static QSqlDatabase
  database(const QString &name = "qt_sql_default_connection", bool = true) {
    auto &registry = connections();
    std::lock_guard<std::mutex> lock(registry.mutex);
    auto i = registry.names.find(name);
    return i == registry.names.end() ? QSqlDatabase() : QSqlDatabase(i->second);
  }
  static void removeDatabase(const QString &name) {
    auto &registry = connections();
    std::lock_guard<std::mutex> lock(registry.mutex);
    registry.names.erase(name);
  }
  static bool contains(const QString &name) {
    auto &registry = connections();
    std::lock_guard<std::mutex> lock(registry.mutex);
    return registry.names.count(name) > 0;
  }
  void setDatabaseName(const QString &s) { d_->file = s; }
  QString databaseName() const { return d_ ? d_->file : QString(); }
  void setConnectOptions(const QString &s) { d_->options = s; }
  bool open() {
    if (!d_)
      return false;
    if (d_->handle)
      return true;
    int flags = d_->options.contains("READONLY")
                    ? SQLITE_OPEN_READONLY
                    : SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int rc = sqlite3_open_v2(d_->file.toUtf8().constData(), &d_->handle,
                             flags | SQLITE_OPEN_FULLMUTEX, nullptr);
    if (rc != SQLITE_OK) {
      d_->error =
          d_->handle ? sqlite3_errmsg(d_->handle) : "SQLite allocation failed";
      return false;
    }
    sqlite3_busy_timeout(d_->handle, 10000);
    return true;
  }
  void close() {
    if (d_ && d_->handle) {
      sqlite3_close_v2(d_->handle);
      d_->handle = nullptr;
    }
  }
  bool isOpen() const { return d_ && d_->handle; }
  bool isValid() const { return bool(d_); }
  QSqlError lastError() const {
    return d_ ? QSqlError(d_->error) : QSqlError("No database connection");
  }
  sqlite3 *native() const { return d_ ? d_->handle : nullptr; }
  bool execute(const char *sql) {
    if (!native())
      return false;
    char *err = nullptr;
    int rc = sqlite3_exec(native(), sql, nullptr, nullptr, &err);
    if (err) {
      d_->error = err;
      sqlite3_free(err);
    }
    return rc == SQLITE_OK;
  }
  bool transaction() { return execute("BEGIN"); }
  bool commit() { return execute("COMMIT"); }
  bool rollback() { return execute("ROLLBACK"); }
  QSqlQuery exec(const QString &);
  QStringList tables() const;
  QSqlRecord record(const QString &) const;
};
class QSqlRecord {
  QStringList names_;
  std::map<QString, int> indices_;

public:
  QSqlRecord() = default;
  explicit QSqlRecord(QStringList n) : names_(std::move(n)) {
    for (int i = 0; i < names_.size(); ++i)
      indices_.emplace(names_[i].toLower(), i);
  }
  int indexOf(const QString &s) const {
    auto i = indices_.find(s.toLower());
    return i == indices_.end() ? -1 : i->second;
  }
  int count() const { return names_.size(); }
  bool isEmpty() const { return names_.isEmpty(); }
  QString fieldName(int i) const { return names_.value(i); }
};
class QSqlQuery {
  QSqlDatabase db_;
  mutable std::optional<QSqlRecord> record_;
  sqlite3_stmt *stmt_ = nullptr;
  QString error_, sql_;
  int nextBind_ = 0;
  int row_ = -1;
  bool pending_ = false, active_ = false;
  void cleanup() {
    if (stmt_)
      sqlite3_finalize(stmt_);
    stmt_ = nullptr;
    pending_ = false;
    active_ = false;
    row_ = -1;
    nextBind_ = 0;
    record_.reset();
  }
  void bind(int pos, const QVariant &v) {
    if (!stmt_)
      return;
    int rc;
    switch (v.type()) {
    case QVariant::Invalid:
      rc = sqlite3_bind_null(stmt_, pos);
      break;
    case QVariant::Bool:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
      rc = sqlite3_bind_int64(stmt_, pos, v.toLongLong());
      break;
    case QVariant::Double:
      rc = sqlite3_bind_double(stmt_, pos, v.toDouble());
      break;
    case QVariant::ByteArray: {
      auto b = v.toByteArray();
      rc = sqlite3_bind_blob(stmt_, pos, b.data(), static_cast<int>(b.size()),
                             SQLITE_TRANSIENT);
      break;
    }
    default: {
      auto s = v.toString().toUtf8();
      rc = sqlite3_bind_text(stmt_, pos, s.data(), static_cast<int>(s.size()),
                             SQLITE_TRANSIENT);
    }
    }
    if (rc != SQLITE_OK)
      error_ = sqlite3_errmsg(db_.native());
  }

public:
  explicit QSqlQuery(QSqlDatabase db = QSqlDatabase::database())
      : db_(std::move(db)) {}
  QSqlQuery(const QString &s, QSqlDatabase db = QSqlDatabase::database())
      : db_(std::move(db)) {
    exec(s);
  }
  ~QSqlQuery() { cleanup(); }
  QSqlQuery(const QSqlQuery &) = delete;
  QSqlQuery &operator=(const QSqlQuery &) = delete;
  QSqlQuery(QSqlQuery &&o) noexcept
      : db_(o.db_), stmt_(std::exchange(o.stmt_, nullptr)), error_(o.error_),
        sql_(o.sql_), nextBind_(o.nextBind_), row_(o.row_),
        pending_(o.pending_), active_(o.active_) {}
  QSqlQuery &operator=(QSqlQuery &&o) noexcept {
    cleanup();
    db_ = o.db_;
    stmt_ = std::exchange(o.stmt_, nullptr);
    error_ = o.error_;
    sql_ = o.sql_;
    nextBind_ = o.nextBind_;
    row_ = o.row_;
    pending_ = o.pending_;
    active_ = o.active_;
    return *this;
  }
  bool prepare(const QString &s) {
    cleanup();
    sql_ = s;
    error_.clear();
    if (!db_.native()) {
      error_ = "No open SQLite connection";
      return false;
    }
    int rc = sqlite3_prepare_v2(db_.native(), s.toUtf8().constData(), -1,
                                &stmt_, nullptr);
    if (rc != SQLITE_OK)
      error_ = sqlite3_errmsg(db_.native());
    return rc == SQLITE_OK;
  }
  bool exec(const QString &s) { return prepare(s) && exec(); }
  bool exec() {
    if (!stmt_)
      return false;
    sqlite3_reset(stmt_);
    row_ = -1;
    nextBind_ = 0;
    int rc = sqlite3_step(stmt_);
    pending_ = rc == SQLITE_ROW;
    active_ = rc == SQLITE_ROW || rc == SQLITE_DONE;
    if (!active_)
      error_ = sqlite3_errmsg(db_.native());
    return active_;
  }
  void addBindValue(const QVariant &v) {
    if (!stmt_)
      return;
    if (nextBind_ == 0) {
      sqlite3_reset(stmt_);
      sqlite3_clear_bindings(stmt_);
    }
    bind(++nextBind_, v);
  }
  void bindValue(int i, const QVariant &v) {
    if (!stmt_)
      return;
    sqlite3_reset(stmt_);
    bind(i + 1, v);
  }
  void bindValue(const QString &name, const QVariant &v) {
    if (!stmt_)
      return;
    sqlite3_reset(stmt_);
    bind(sqlite3_bind_parameter_index(stmt_, name.toUtf8().constData()), v);
  }
  bool next() {
    if (!active_)
      return false;
    if (pending_) {
      pending_ = false;
      ++row_;
      return true;
    }
    int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) {
      ++row_;
      return true;
    }
    if (rc != SQLITE_DONE)
      error_ = sqlite3_errmsg(db_.native());
    active_ = false;
    return false;
  }
  bool first() {
    if (!stmt_)
      return false;
    sqlite3_reset(stmt_);
    pending_ = false;
    active_ = true;
    row_ = -1;
    return next();
  }
  bool seek(int n, bool relative = false) {
    if (relative)
      n += row_;
    if (n < 0)
      return false;
    if (n < row_ && !first())
      return false;
    while (row_ < n)
      if (!next())
        return false;
    return row_ == n;
  }
  int at() const { return row_; }
  void finish() {
    if (stmt_)
      sqlite3_reset(stmt_);
    active_ = false;
    pending_ = false;
  }
  void clear() { cleanup(); }
  bool isActive() const { return active_; }
  bool isValid() const { return row_ >= 0; }
  void setForwardOnly(bool) {}
  int size() const { return -1; }
  QVariant value(int c) const {
    if (!stmt_ || c < 0 || c >= sqlite3_column_count(stmt_))
      return {};
    switch (sqlite3_column_type(stmt_, c)) {
    case SQLITE_INTEGER:
      return QVariant(qint64(sqlite3_column_int64(stmt_, c)));
    case SQLITE_FLOAT:
      return QVariant(sqlite3_column_double(stmt_, c));
    case SQLITE_TEXT:
      return QVariant(QString::fromUtf8(
          reinterpret_cast<const char *>(sqlite3_column_text(stmt_, c)),
          sqlite3_column_bytes(stmt_, c)));
    case SQLITE_BLOB:
      return QVariant(
          QByteArray(static_cast<const char *>(sqlite3_column_blob(stmt_, c)),
                     sqlite3_column_bytes(stmt_, c)));
    default:
      return {};
    }
  }
  QVariant value(const QString &n) const { return value(record().indexOf(n)); }
  const QSqlRecord &record() const {
    if (record_)
      return *record_;
    QStringList names;
    if (stmt_)
      for (int i = 0; i < sqlite3_column_count(stmt_); ++i)
        names << QString(sqlite3_column_name(stmt_, i));
    record_.emplace(std::move(names));
    return *record_;
  }
  QSqlError lastError() const { return QSqlError(error_); }
  QString lastQuery() const { return sql_; }
  int numRowsAffected() const {
    return db_.native() ? sqlite3_changes(db_.native()) : 0;
  }
};
inline QSqlQuery QSqlDatabase::exec(const QString &s) {
  return QSqlQuery(s, *this);
}
inline QStringList QSqlDatabase::tables() const {
  QSqlQuery q("SELECT name FROM sqlite_master WHERE type='table'", *this);
  QStringList r;
  while (q.next())
    r << q.value(0).toString();
  return r;
}

inline QSqlRecord QSqlDatabase::record(const QString &table) const {
  QString escaped = table;
  escaped.replace("\"", "\"\"");
  QSqlQuery q("SELECT * FROM \"" + escaped + "\" LIMIT 0", *this);
  return q.record();
}
