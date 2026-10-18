#pragma once
#include "core.h"
class QIODevice {
public:
  enum OpenModeFlag {
    ReadOnly = 1,
    WriteOnly = 2,
    ReadWrite = 3,
    Append = 4,
    Truncate = 8,
    Text = 16
  };
  using OpenMode = int;
};
class QFile : public QIODevice {
  QString path_;
  std::fstream stream_;
  QString error_;

public:
  QFile() = default;
  explicit QFile(const QString &p) : path_(p) {}
  void setFileName(const QString &p) { path_ = p; }
  QString fileName() const { return path_; }
  bool open(int flags) {
    std::ios::openmode m = std::ios::binary;
    if (flags & ReadOnly)
      m |= std::ios::in;
    if (flags & WriteOnly)
      m |= std::ios::out;
    if (flags & Append)
      m |= std::ios::app;
    else if (flags & WriteOnly)
      m |= std::ios::trunc;
    stream_.open(std::filesystem::u8path(path_.toStdString()), m);
    if (!stream_)
      error_ = "Cannot open " + path_;
    return bool(stream_);
  }
  void close() { stream_.close(); }
  bool isOpen() const { return stream_.is_open(); }
  bool atEnd() { return stream_.peek() == EOF; }
  QString errorString() const { return error_; }
  QByteArray readAll() {
    std::ostringstream s;
    s << stream_.rdbuf();
    return s.str();
  }
  QByteArray read(qint64 n) {
    QByteArray a(static_cast<size_t>(n), '\0');
    stream_.read(a.data(), n);
    a.resize(static_cast<size_t>(stream_.gcount()));
    return a;
  }
  qint64 read(char *p, qint64 n) {
    stream_.read(p, n);
    return stream_.gcount();
  }
  qint64 write(const char *p, qint64 n) {
    stream_.write(p, n);
    return stream_ ? n : -1;
  }
  qint64 write(const QByteArray &a) { return write(a.data(), a.size()); }
  bool seek(qint64 p) {
    stream_.clear();
    stream_.seekg(p);
    return bool(stream_);
  }
  qint64 pos() { return static_cast<qint64>(stream_.tellg()); }
  qint64 size() const {
    std::error_code e;
    auto n = std::filesystem::file_size(
        std::filesystem::u8path(path_.toStdString()), e);
    return e ? 0 : static_cast<qint64>(n);
  }
  bool exists() const { return exists(path_); }
  static bool exists(const QString &p) {
    std::error_code e;
    return std::filesystem::exists(std::filesystem::u8path(p.toStdString()), e);
  }
  bool remove() {
    close();
    return remove(path_);
  }
  static bool remove(const QString &p) {
    std::error_code e;
    return std::filesystem::remove(std::filesystem::u8path(p.toStdString()), e);
  }
  static bool copy(const QString &a, const QString &b) {
    std::error_code e;
    return std::filesystem::copy_file(std::filesystem::u8path(a.toStdString()),
                                      std::filesystem::u8path(b.toStdString()),
                                      e);
  }
  void flush() { stream_.flush(); }
  std::fstream &native() { return stream_; }
};
class QTextStream {
  QFile *file_ = nullptr;
  QString *text_ = nullptr;
  std::stringstream memory_;
  std::ostream *out_ = nullptr;
  std::istream *in_ = nullptr;

public:
  QTextStream() {
    out_ = &memory_;
    in_ = &memory_;
  }
  void setDevice(QFile *f) {
    file_ = f;
    out_ = &f->native();
    in_ = &f->native();
  }
  qint64 pos() { return in_ ? static_cast<qint64>(in_->tellg()) : -1; }
  explicit QTextStream(QFile *f)
      : file_(f), out_(&f->native()), in_(&f->native()) {}
  explicit QTextStream(QString *s, int flags = QIODevice::ReadOnly) : text_(s) {
    if (flags & QIODevice::ReadOnly)
      memory_.str(s->toStdString());
    out_ = &memory_;
    in_ = &memory_;
  }
  explicit QTextStream(QByteArray *s, int = QIODevice::ReadOnly) {
    memory_.str(*s);
    out_ = &memory_;
    in_ = &memory_;
  }
  QString readAll() {
    std::ostringstream s;
    s << in_->rdbuf();
    return s.str();
  }
  QString readLine() {
    std::string s;
    std::getline(*in_, s);
    if (!s.empty() && s.back() == '\r')
      s.pop_back();
    return s;
  }
  bool atEnd() { return in_->peek() == EOF; }
  void flush() {
    out_->flush();
    if (text_)
      *text_ = memory_.str();
  }
  ~QTextStream() { flush(); }
  template <class T> QTextStream &operator<<(const T &v) {
    *out_ << v;
    return *this;
  }
  template <class T> QTextStream &operator>>(T &v) {
    *in_ >> v;
    return *this;
  }
  void setRealNumberPrecision(int n) { *out_ << std::setprecision(n); }
  void setCodec(const char *) {}
};
class QDir {
  std::filesystem::path path_;

public:
  QDir() : path_(std::filesystem::current_path()) {}
  QDir(const QString &p) : path_(std::filesystem::u8path(p.toStdString())) {}
  void setPath(const QString &p) {
    path_ = std::filesystem::u8path(p.toStdString());
  }
  static QChar separator() { return '/'; }
  static QString currentPath() {
    return QString(std::filesystem::current_path().u8string());
  }
  static bool setCurrent(const QString &p) {
    std::error_code e;
    std::filesystem::current_path(std::filesystem::u8path(p.toStdString()), e);
    return !e;
  }
  static QString cleanPath(const QString &p) {
    return QString(std::filesystem::u8path(p.toStdString())
                       .lexically_normal()
                       .generic_u8string());
  }
  static QString toNativeSeparators(const QString &p) {
    return QString(
        std::filesystem::u8path(p.toStdString()).make_preferred().u8string());
  }
  static QString fromNativeSeparators(QString p) {
    return p.replace('\\', '/');
  }
  static bool isAbsolutePath(const QString &p) {
    return std::filesystem::u8path(p.toStdString()).is_absolute();
  }
  QString filePath(const QString &p) const {
    return QString(
        (path_ / std::filesystem::u8path(p.toStdString())).generic_u8string());
  }
  QString absoluteFilePath(const QString &p) const {
    return QString(std::filesystem::absolute(
                       path_ / std::filesystem::u8path(p.toStdString()))
                       .generic_u8string());
  }
  QString absolutePath() const {
    return QString(std::filesystem::absolute(path_).generic_u8string());
  }
  QString path() const { return QString(path_.generic_u8string()); }
  bool exists() const { return std::filesystem::is_directory(path_); }
  bool exists(const QString &p) const { return QFile::exists(filePath(p)); }
  bool mkpath(const QString &p) const {
    std::error_code e;
    auto dest = path_ / std::filesystem::u8path(p.toStdString());
    return std::filesystem::is_directory(dest) ||
           std::filesystem::create_directories(dest, e);
  }
  bool cdUp() {
    path_ = path_.parent_path();
    return true;
  }
  bool cd(const QString &p) {
    auto t = path_ / std::filesystem::u8path(p.toStdString());
    if (!std::filesystem::is_directory(t))
      return false;
    path_ = t;
    return true;
  }
};
class QFileInfo {
  std::filesystem::path p_;

public:
  QFileInfo(const QString &p) : p_(std::filesystem::u8path(p.toStdString())) {}
  bool exists() const { return std::filesystem::exists(p_); }
  static bool exists(const QString &p) { return QFile::exists(p); }
  bool isAbsolute() const { return p_.is_absolute(); }
  QString absolutePath() const {
    return QString(
        std::filesystem::absolute(p_).parent_path().generic_u8string());
  }
  QString absoluteFilePath() const {
    return QString(std::filesystem::absolute(p_).generic_u8string());
  }
  QString fileName() const { return QString(p_.filename().u8string()); }
  QString baseName() const { return QString(p_.stem().u8string()); }
  QString completeBaseName() const { return baseName(); }
  QString suffix() const {
    auto s = p_.extension().u8string();
    return s.empty() ? QString() : QString(s.substr(1));
  }
  QDir dir() const { return QDir(QString(p_.parent_path().u8string())); }
};

// QDataStream's default numeric encoding: big-endian, doubles for floating
// point.
class QDataStream {
  QFile *file_ = nullptr;
  QByteArray *buffer_ = nullptr;
  QByteArray owned_;
  size_t pos_ = 0;
  bool good_ = true;
  bool legacyFloat_ = false;
  void writeBytes(const char *p, size_t n) {
    if (file_) {
      if (file_->write(p, n) != static_cast<qint64>(n))
        good_ = false;
    } else if (buffer_)
      buffer_->append(p, n);
    else
      good_ = false;
  }
  void readBytes(char *p, size_t n) {
    if (file_) {
      if (file_->read(p, n) != static_cast<qint64>(n))
        good_ = false;
    } else if (buffer_ && pos_ + n <= buffer_->size()) {
      std::memcpy(p, buffer_->data() + pos_, n);
      pos_ += n;
    } else {
      good_ = false;
      std::memset(p, 0, n);
    }
    if (!good_)
      throw std::runtime_error("Truncated binary model data");
  }

public:
  enum Version { Qt_4_5 = 11 };
  enum Status { Ok, ReadPastEnd };
  enum ByteOrder { BigEndian, LittleEndian };
  explicit QDataStream(QFile *f) : file_(f) {}
  QDataStream(QByteArray *b, int) : buffer_(b) { b->clear(); }
  explicit QDataStream(const QByteArray &b) : owned_(b), buffer_(&owned_) {}
  void setVersion(int v) { legacyFloat_ = v <= Qt_4_5; }
  void setByteOrder(ByteOrder b) {
    if (b != BigEndian)
      throw std::runtime_error("Unsupported binary byte order");
  }
  bool atEnd() { return file_ ? file_->atEnd() : pos_ >= buffer_->size(); }
  Status status() const { return good_ ? Ok : ReadPastEnd; }
  QDataStream &operator<<(float v) {
    if (!legacyFloat_)
      return *this << double(v);
    quint32 bits;
    std::memcpy(&bits, &v, sizeof bits);
    return *this << bits;
  }
  QDataStream &operator>>(float &v) {
    if (!legacyFloat_) {
      double d;
      *this >> d;
      v = static_cast<float>(d);
      return *this;
    }
    quint32 bits;
    *this >> bits;
    std::memcpy(&v, &bits, sizeof v);
    return *this;
  }
  template <class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
  QDataStream &operator<<(T v) {
    using Wire = std::conditional_t<std::is_floating_point_v<T>, double, T>;
    Wire w = v;
    std::array<char, sizeof(Wire)> b{};
    std::memcpy(b.data(), &w, sizeof w);
    if (littleEndian())
      std::reverse(b.begin(), b.end());
    writeBytes(b.data(), b.size());
    return *this;
  }
  template <class T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
  QDataStream &operator>>(T &v) {
    using Wire = std::conditional_t<std::is_floating_point_v<T>, double, T>;
    std::array<char, sizeof(Wire)> b{};
    readBytes(b.data(), b.size());
    if (littleEndian())
      std::reverse(b.begin(), b.end());
    Wire w;
    std::memcpy(&w, b.data(), sizeof w);
    v = static_cast<T>(w);
    return *this;
  }
  QDataStream &operator<<(const QString &s) {
    *this << quint32(s.size() * 2);
    for (auto c : s)
      *this << quint16(c);
    return *this;
  }
  QDataStream &operator>>(QString &s) {
    quint32 n;
    *this >> n;
    if (n == 0xffffffff) {
      s.clear();
      return *this;
    }
    if (n % 2 || n > 100000000)
      throw std::runtime_error("Invalid binary string length");
    std::u16string v;
    v.reserve(n / 2);
    for (quint32 i = 0; i < n / 2; ++i) {
      quint16 c;
      *this >> c;
      v += char16_t(c);
    }
    s = QString(v);
    return *this;
  }
  QDataStream &operator<<(const QByteArray &b) {
    *this << quint32(b.size());
    writeBytes(b.data(), b.size());
    return *this;
  }
  QDataStream &operator>>(QByteArray &b) {
    quint32 n;
    *this >> n;
    if (n == 0xffffffff) {
      b.clear();
      return *this;
    }
    b.resize(n);
    readBytes(b.data(), n);
    return *this;
  }
  int writeRawData(const char *p, int n) {
    writeBytes(p, n);
    return n;
  }
  int readRawData(char *p, int n) {
    readBytes(p, n);
    return n;
  }

private:
  static bool littleEndian() {
    uint16_t n = 1;
    return *reinterpret_cast<char *>(&n) == 1;
  }
};
