#pragma once
#include "core.h"
#include "geometry.h"
#include "io.h"
#include "script.h"
#include "sql.h"
#include "xml.h"
class QSettings {
  QVariantMap values_;

public:
  enum Format { IniFormat };
  QSettings(const QString &file, Format) {
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly))
      return;
    QTextStream in(&f);
    QString group;
    while (!in.atEnd()) {
      auto line = in.readLine().trimmed();
      if (line.isEmpty() || line.startsWith(';') || line.startsWith('#'))
        continue;
      if (line.startsWith('[') && line.endsWith(']')) {
        group = line.mid(1, line.size() - 2);
        continue;
      }
      auto p = line.indexOf('=');
      if (p >= 0)
        values_.insert((group.isEmpty() ? QString() : group + "/") +
                           line.left(p).trimmed(),
                       line.mid(p + 1).trimmed());
    }
  }
  QVariant value(const QString &k, const QVariant &def = {}) const {
    return values_.value(k, def);
  }
  QStringList allKeys() const { return values_.keys(); }
};
class QProcess {
  QByteArray output_, error_;
  int exit_ = -1;

public:
  void startCommand(const QString &);
  bool waitForFinished(int = -1) { return true; }
  QByteArray readAllStandardOutput() { return output_; }
  QByteArray readAllStandardError() { return error_; }
  int exitCode() const { return exit_; }
};
