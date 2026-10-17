#pragma once
#include "simulationcontext.h"
// The iLand-specific subset of the former Qt API. No Qt headers or binaries.
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <codecvt>
#include <condition_variable>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <cwctype>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <tuple>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <variant>
#include <vector>

#define QT_VERSION 0x060800
#define QT_VERSION_CHECK(a, b, c) (((a) << 16) | ((b) << 8) | (c))
#define Q_UNUSED(x) (void)(x);
#define Q_DISABLE_COPY(T)                                                      \
  T(const T &) = delete;                                                       \
  T &operator=(const T &) = delete;
#define Q_DECLARE_TYPEINFO(...)
#define Q_DECLARE_OPAQUE_POINTER(...)
#define Q_ASSERT(x) assert(x)
#define Q_ASSERT_X(x, w, m) assert(x)
#define Q_PROPERTY(...)
#define Q_INVOKABLE
#define Q_ENUM(...)
#define Q_ENUMS(...)
#define Q_INTERFACES(...)
#define Q_PLUGIN_METADATA(...)
#define Q_DECLARE_INTERFACE(...)
#define Q_DECLARE_METATYPE(...)
#define Q_IMPORT_PLUGIN(...)
#define Q_EXPORT_PLUGIN2(...)
#define signals public
#define slots
#define emit
#define foreach(x, ...)                                                        \
  for (auto _iland_each = portableForeach(__VA_ARGS__); _iland_each.more();    \
       _iland_each.advance())                                                  \
    for (x = _iland_each.value(); _iland_each.inner();                         \
         _iland_each.finishInner())
#define forever for (;;)
#define QStringLiteral(x) QString(x)
#define QLatin1String(x) QString(x)
#define QLatin1Char(x) QChar(x)
#define qPrintable(x) (x).toUtf8().constData()
#define qUtf8Printable(x) (x).toUtf8().constData()
using qint8 = int8_t;
using quint8 = uint8_t;
using qint16 = int16_t;
using quint16 = uint16_t;
using qint32 = int32_t;
using quint32 = uint32_t;
using qint64 = int64_t;
using quint64 = uint64_t;
using qsizetype = int;
using uint = unsigned;
using ushort = unsigned short;
using uchar = unsigned char;
namespace Qt {
enum CaseSensitivity { CaseInsensitive, CaseSensitive };
enum SplitBehavior { KeepEmptyParts, SkipEmptyParts };
enum ConnectionType { DirectConnection };
inline constexpr char endl = '\n';
} // namespace Qt
inline void qt_noop() {}
template <class T> constexpr const T &qMin(const T &a, const T &b) {
  return std::min(a, b);
}
template <class T> constexpr const T &qMax(const T &a, const T &b) {
  return std::max(a, b);
}
template <class T> constexpr T qBound(T l, T v, T h) {
  return std::clamp(v, l, h);
}
template <class T> auto qAbs(T v) { return std::abs(v); }
inline int qRound(double x) { return static_cast<int>(std::round(x)); }
inline qint64 qRound64(double x) { return static_cast<qint64>(std::round(x)); }
inline bool qFuzzyCompare(double a, double b) {
  return std::abs(a - b) <= 1e-12 * std::min(std::abs(a), std::abs(b));
}
inline bool qFuzzyIsNull(double a) { return std::abs(a) <= 1e-12; }
template <class T> void qSwap(T &a, T &b) { std::swap(a, b); }
template <class T> size_t qHash(const T &v, size_t seed = 0) {
  return std::hash<T>{}(v) ^ seed;
}
template <class T> int qRegisterMetaType(const char * = nullptr) { return 0; }
inline int qFloor(double n) { return static_cast<int>(std::floor(n)); }
inline int qCeil(double n) { return static_cast<int>(std::ceil(n)); }
template <class C> class PortableForeach {
  C c_;
  decltype(std::declval<const C>().begin()) i_;
  bool inner_ = true, stopped_ = false;

public:
  explicit PortableForeach(const C &c)
      : c_(c), i_(static_cast<const C &>(c_).begin()) {}
  bool more() const {
    return !stopped_ && i_ != static_cast<const C &>(c_).end();
  }
  decltype(auto) value() { return *i_; }
  bool inner() const { return inner_; }
  void finishInner() { inner_ = false; }
  void advance() {
    if (inner_)
      stopped_ = true;
    else {
      ++i_;
      inner_ = true;
    }
  }
};
template <class C> auto portableForeach(const C &c) {
  return PortableForeach<C>(c);
}
template <class T> void qDeleteAll(const T &c) {
  for (auto p : c)
    delete p;
}
template <class I> void qDeleteAll(I a, I b) {
  while (a != b)
    delete *a++;
}
template <class A, class B> using QPair = std::pair<A, B>;

class QChar {
  char16_t v_ = 0;
  char16_t *reference_ = nullptr;

public:
  QChar() = default;
  QChar(char v) : v_(static_cast<unsigned char>(v)) {}
  QChar(char16_t v) : v_(v) {}
  QChar(int v) : v_(static_cast<char16_t>(v)) {}
  static QChar reference(char16_t &v) {
    QChar c(v);
    c.reference_ = &v;
    return c;
  }
  QChar &operator=(QChar c) {
    v_ = c.v_;
    if (reference_)
      *reference_ = v_;
    return *this;
  }
  operator char16_t() const { return v_; }
  ushort unicode() const { return v_; }
  char toLatin1() const { return v_ <= 255 ? static_cast<char>(v_) : 0; }
  bool isSpace() const { return std::iswspace(v_) != 0; }
  bool isDigit() const { return v_ >= '0' && v_ <= '9'; }
  bool isNumber() const { return isDigit(); }
  bool isLetter() const { return std::iswalpha(v_) != 0; }
  bool isLetterOrNumber() const { return isDigit() || isLetter(); }
  bool isNull() const { return v_ == 0; }
  QChar toLower() const {
    return QChar(static_cast<char16_t>(std::towlower(v_)));
  }
  friend bool operator==(QChar a, QChar b) { return a.v_ == b.v_; }
  friend bool operator!=(QChar a, QChar b) { return !(a == b); }
  friend bool operator==(QChar a, char b) {
    return a.v_ == static_cast<unsigned char>(b);
  }
  friend bool operator==(char a, QChar b) { return b == a; }
  friend bool operator!=(QChar a, char b) { return !(a == b); }
  friend bool operator!=(char a, QChar b) { return !(b == a); }
  friend bool operator<(QChar a, QChar b) { return a.v_ < b.v_; }
};

template <class T> class QList;
class QByteArray : public std::string {
public:
  using std::string::append;
  using std::string::string;
  QByteArray() = default;
  QByteArray(const std::string &v) : std::string(v) {}
  const char *constData() const { return c_str(); }
  bool isEmpty() const { return empty(); }
  int count() const { return static_cast<int>(size()); }
  operator const char *() const { return c_str(); }
  QByteArray &append(const QByteArray &a) {
    std::string::append(a);
    return *this;
  }
  QByteArray &append(char c) {
    push_back(c);
    return *this;
  }
  QList<QByteArray> split(char) const;
  QByteArray simplified() const;
};
template <class T> class QList;
class QStringList;
class QRegularExpression;

class QString {
  std::u16string s_;
  static std::u16string decode(const std::string &v) {
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
        .from_bytes(v);
  }

public:
  QString() = default;
  QString(const char *v) : s_(decode(v ? v : "")) {}
  QString(const std::string &v) : s_(decode(v)) {}
  QString(const QByteArray &v) : s_(decode(v)) {}
  QString(const std::u16string &v) : s_(v) {}
  QString(QChar c) : s_(1, c.unicode()) {}
  QString(char c) : s_(1, static_cast<unsigned char>(c)) {}
  QString(int n, QChar c) : s_(std::max(0, n), c.unicode()) {}
  static QString fromUtf8(const char *p, int n = -1) {
    return QString(n < 0 ? std::string(p ? p : "") : std::string(p, n));
  }
  static QString fromLatin1(const char *p, int n = -1) {
    QString s;
    if (!p)
      return s;
    if (n < 0)
      n = static_cast<int>(std::strlen(p));
    for (int i = 0; i < n; ++i)
      s.s_ += static_cast<uchar>(p[i]);
    return s;
  }
  static QString fromStdString(const std::string &s) { return QString(s); }
  std::string toStdString() const {
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}
        .to_bytes(s_);
  }
  QByteArray toUtf8() const { return toStdString(); }
  QByteArray toLocal8Bit() const { return toUtf8(); }
  QByteArray toLatin1() const {
    QByteArray r;
    for (auto c : s_)
      r.push_back(c <= 255 ? static_cast<char>(c) : '?');
    return r;
  }
  const char16_t *utf16() const { return s_.data(); }
  const char16_t *constData() const { return s_.c_str(); }
  const char16_t *data() const { return s_.data(); }
  int size() const { return static_cast<int>(s_.size()); }
  int length() const { return size(); }
  int count() const { return size(); }
  bool isEmpty() const { return s_.empty(); }
  bool isNull() const { return s_.empty(); }
  void clear() { s_.clear(); }
  void reserve(int n) {
    if (n > 0)
      s_.reserve(n);
  }
  void resize(int n) { s_.resize(n); }
  QChar at(int i) const { return QChar(s_.at(i)); }
  QChar operator[](int i) const { return QChar(s_[i]); }
  QChar operator[](int i) { return QChar::reference(s_[i]); }
  auto begin() const { return s_.begin(); }
  auto end() const { return s_.end(); }
  QString &operator+=(const QString &b) {
    s_ += b.s_;
    return *this;
  }
  QString &append(const QString &b) { return *this += b; }
  QString &prepend(const QString &b) {
    s_.insert(0, b.s_);
    return *this;
  }
  QString &insert(int i, const QString &b) {
    s_.insert(i, b.s_);
    return *this;
  }
  friend QString operator+(QString a, const QString &b) { return a += b; }
  friend bool operator==(const QString &a, const QString &b) {
    return a.s_ == b.s_;
  }
  friend bool operator!=(const QString &a, const QString &b) {
    return !(a == b);
  }
  friend bool operator<(const QString &a, const QString &b) {
    return a.s_ < b.s_;
  }
  friend bool operator>(const QString &a, const QString &b) { return b < a; }
  QString mid(int p, int n = -1) const {
    if (p < 0 || p >= size())
      return {};
    return s_.substr(p, n < 0 ? std::u16string::npos : n);
  }
  QString sliced(int p, int n = -1) const { return mid(p, n); }
  QString left(int n) const { return mid(0, n); }
  QString right(int n) const {
    return n < 0 || n >= size() ? *this : mid(size() - n);
  }
  void chop(int n) {
    if (n > 0)
      s_.resize(n >= size() ? 0 : size() - n);
  }
  void truncate(int n) {
    if (n < size())
      s_.resize(std::max(0, n));
  }
  QString toLower() const {
    QString r = *this;
    for (auto &c : r.s_)
      c = static_cast<char16_t>(std::towlower(c));
    return r;
  }
  QString toUpper() const {
    QString r = *this;
    for (auto &c : r.s_)
      c = static_cast<char16_t>(std::towupper(c));
    return r;
  }
  QString trimmed() const {
    int a = 0, b = size();
    while (a < b && QChar(s_[a]).isSpace())
      ++a;
    while (b > a && QChar(s_[b - 1]).isSpace())
      --b;
    return mid(a, b - a);
  }
  QString simplified() const {
    QString r;
    bool space = false;
    for (auto c : trimmed().s_) {
      if (QChar(c).isSpace())
        space = true;
      else {
        if (space)
          r += ' ';
        r += QChar(c);
        space = false;
      }
    }
    return r;
  }
  int compare(const QString &b,
              Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
    if (cs == Qt::CaseInsensitive)
      return toLower().compare(b.toLower());
    return s_ < b.s_ ? -1 : s_ > b.s_ ? 1 : 0;
  }
  static int compare(const QString &a, const QString &b,
                     Qt::CaseSensitivity cs = Qt::CaseSensitive) {
    return a.compare(b, cs);
  }
  int indexOf(const QString &b, int from = 0,
              Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
    if (cs == Qt::CaseInsensitive)
      return toLower().indexOf(b.toLower(), from);
    if (from < 0)
      from = std::max(0, size() + from);
    auto p = s_.find(b.s_, from);
    return p == s_.npos ? -1 : static_cast<int>(p);
  }
  int lastIndexOf(const QString &b, int from = -1) const {
    auto p = s_.rfind(b.s_, from < 0 ? s_.npos : static_cast<size_t>(from));
    return p == s_.npos ? -1 : static_cast<int>(p);
  }
  bool contains(const QString &b,
                Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
    return indexOf(b, 0, cs) >= 0;
  }
  bool startsWith(const QString &b,
                  Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
    return b.size() <= size() && left(b.size()).compare(b, cs) == 0;
  }
  bool endsWith(const QString &b,
                Qt::CaseSensitivity cs = Qt::CaseSensitive) const {
    return b.size() <= size() && right(b.size()).compare(b, cs) == 0;
  }
  int count(const QString &b) const {
    if (b.isEmpty())
      return size() + 1;
    int n = 0, p = 0;
    while ((p = indexOf(b, p)) >= 0) {
      ++n;
      p += b.size();
    }
    return n;
  }
  QString &replace(const QString &a, const QString &b,
                   Qt::CaseSensitivity cs = Qt::CaseSensitive) {
    if (a.isEmpty())
      return *this;
    int p = 0;
    while ((p = indexOf(a, p, cs)) >= 0) {
      s_.replace(p, a.size(), b.s_);
      p += b.size();
    }
    return *this;
  }
  QString &replace(int pos, int len, const QString &b) {
    s_.replace(pos, len, b.s_);
    return *this;
  }
  QString &remove(const QString &a) { return replace(a, ""); }
  QString &remove(int pos, int len) {
    s_.erase(pos, len);
    return *this;
  }
  QString &replace(const QRegularExpression &, const QString &);
  QStringList split(const QString &, Qt::SplitBehavior = Qt::KeepEmptyParts,
                    Qt::CaseSensitivity = Qt::CaseSensitive) const;
  QStringList split(const QRegularExpression &,
                    Qt::SplitBehavior = Qt::KeepEmptyParts) const;
  QString section(const QString &, int, int = -1) const;
  double toDouble(bool *ok = nullptr) const {
    auto v = trimmed().toStdString();
    char *e = nullptr;
    errno = 0;
    double r = std::strtod(v.c_str(), &e);
    bool good = e != v.c_str() && *e == 0 && errno != ERANGE;
    if (ok)
      *ok = good;
    return good ? r : 0.;
  }
  float toFloat(bool *ok = nullptr) const {
    return static_cast<float>(toDouble(ok));
  }
  qint64 toLongLong(bool *ok = nullptr, int base = 10) const {
    auto v = trimmed().toStdString();
    char *e = nullptr;
    errno = 0;
    auto r = std::strtoll(v.c_str(), &e, base);
    bool good = e != v.c_str() && *e == 0 && errno != ERANGE;
    if (ok)
      *ok = good;
    return good ? r : 0;
  }
  int toInt(bool *ok = nullptr, int base = 10) const {
    bool good;
    auto n = toLongLong(&good, base);
    good = good && n >= INT_MIN && n <= INT_MAX;
    if (ok)
      *ok = good;
    return good ? static_cast<int>(n) : 0;
  }
  uint toUInt(bool *ok = nullptr, int base = 10) const {
    bool good;
    auto n = toLongLong(&good, base);
    good = good && n >= 0 && n <= UINT_MAX;
    if (ok)
      *ok = good;
    return good ? static_cast<uint>(n) : 0;
  }
  static QString number(qint64 n, int base = 10) {
    std::ostringstream s;
    if (base == 16)
      s << std::hex;
    else if (base == 8)
      s << std::oct;
    s << n;
    return s.str();
  }
  static QString number(quint64 n, int base = 10) {
    std::ostringstream s;
    if (base == 16)
      s << std::hex;
    else if (base == 8)
      s << std::oct;
    s << n;
    return s.str();
  }
  static QString number(int n, int b = 10) { return number(qint64(n), b); }
  static QString number(uint n, int b = 10) { return number(quint64(n), b); }
  static QString number(double n, char fmt = 'g', int precision = 6) {
    std::ostringstream s;
    s.imbue(std::locale::classic());
    if (fmt == 'f')
      s << std::fixed;
    else if (fmt == 'e' || fmt == 'E')
      s << std::scientific;
    if (fmt == 'E' || fmt == 'G')
      s << std::uppercase;
    s << std::setprecision(precision) << n;
    return s.str();
  }
  QString arg(const QString &v, int width = 0, QChar fill = ' ') const {
    int best = 100;
    for (int i = 0; i < size() - 1; ++i)
      if (s_[i] == '%' && s_[i + 1] >= '1' && s_[i + 1] <= '9') {
        int n = s_[i + 1] - '0';
        if (i + 2 < size() && s_[i + 2] >= '0' && s_[i + 2] <= '9')
          n = n * 10 + s_[i + 2] - '0';
        best = std::min(best, n);
      }
    if (best == 100)
      return *this;
    QString text = v;
    if (std::abs(width) > v.size()) {
      QString pad(std::abs(width) - v.size(), fill);
      text = width > 0 ? pad + v : v + pad;
    }
    QString r;
    for (int i = 0; i < size(); ++i) {
      if (s_[i] == '%' && i + 1 < size() && s_[i + 1] >= '1' &&
          s_[i + 1] <= '9') {
        int len = 2, n = s_[i + 1] - '0';
        if (i + 2 < size() && s_[i + 2] >= '0' && s_[i + 2] <= '9') {
          n = n * 10 + s_[i + 2] - '0';
          len = 3;
        }
        if (n == best) {
          r += text;
          i += len - 1;
          continue;
        }
      }
      r += QChar(s_[i]);
    }
    return r;
  }
  QString arg(const QString &a, const QString &b) const {
    return arg(a).arg(b);
  }
  QString arg(const QString &a, const QString &b, const QString &c) const {
    return arg(a).arg(b).arg(c);
  }
  QString arg(const QString &a, const QString &b, const QString &c,
              const QString &d) const {
    return arg(a).arg(b).arg(c).arg(d);
  }
  template <class T> QString &setNum(T n) {
    *this = number(n);
    return *this;
  }
  static QString asprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    va_list copy;
    va_copy(copy, args);
    int n = std::vsnprintf(nullptr, 0, format, copy);
    va_end(copy);
    std::vector<char> b(std::max(0, n) + 1);
    std::vsnprintf(b.data(), b.size(), format, args);
    va_end(args);
    return QString(b.data());
  }
  template <class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
  QString arg(T n, int width = 0, int base = 10, QChar fill = ' ') const {
    return arg(number(n, base), width, fill);
  }
  QString arg(double n, int width = 0, char fmt = 'g', int precision = -1,
              QChar fill = ' ') const {
    return arg(number(n, fmt, precision < 0 ? 6 : precision), width, fill);
  }
};
inline std::ostream &operator<<(std::ostream &s, const QString &v) {
  return s << v.toStdString();
}
inline std::ostream &operator<<(std::ostream &s, QChar c) {
  return s << QString(c);
}

template <class T> class QList : public std::vector<T> {
  using B = std::vector<T>;

public:
  using B::B;
  using B::erase;
  using B::insert;
  using ConstIterator = typename B::const_iterator;
  using Iterator = typename B::iterator;
  QList() = default;
  QList(const B &b) : B(b) {}
  int size() const { return static_cast<int>(B::size()); }
  int count() const { return size(); }
  int length() const { return size(); }
  bool isEmpty() const { return B::empty(); }
  void append(const T &v) { B::push_back(v); }
  void append(const QList &v) { B::insert(B::end(), v.begin(), v.end()); }
  void prepend(const T &v) { B::insert(B::begin(), v); }
  QList &operator<<(const T &v) {
    append(v);
    return *this;
  }
  QList &operator<<(const QList &v) {
    append(v);
    return *this;
  }
  QList &operator+=(const QList &v) {
    append(v);
    return *this;
  }
  QList &operator+=(const T &v) {
    append(v);
    return *this;
  }
  void pop_front() { removeFirst(); }
  void push_front(const T &v) { prepend(v); }
  template <class Predicate> int removeIf(Predicate p) {
    int n = size();
    B::erase(std::remove_if(B::begin(), B::end(), p), B::end());
    return n - size();
  }
  friend QList operator+(QList a, const QList &b) {
    a += b;
    return a;
  }
  decltype(auto) first() { return B::front(); }
  decltype(auto) first() const { return B::front(); }
  decltype(auto) last() { return B::back(); }
  decltype(auto) last() const { return B::back(); }
  T value(int i, T def = T{}) const {
    return i >= 0 && i < size() ? (*this)[i] : def;
  }
  int indexOf(const T &v, int from = 0) const {
    for (int i = std::max(from, 0); i < size(); ++i)
      if ((*this)[i] == v)
        return i;
    return -1;
  }
  bool contains(const T &v) const { return indexOf(v) >= 0; }
  int count(const T &v) const {
    return static_cast<int>(std::count(B::begin(), B::end(), v));
  }
  void removeAt(int i) { B::erase(B::begin() + i); }
  void removeFirst() { removeAt(0); }
  void removeLast() { B::pop_back(); }
  bool removeOne(const T &v) {
    int i = indexOf(v);
    if (i < 0)
      return false;
    removeAt(i);
    return true;
  }
  int removeAll(const T &v) {
    int n = size();
    B::erase(std::remove(B::begin(), B::end(), v), B::end());
    return n - size();
  }
  void remove(int i, int n = 1) {
    B::erase(B::begin() + i, B::begin() + i + n);
  }
  T takeAt(int i) {
    T t = std::move((*this)[i]);
    removeAt(i);
    return t;
  }
  T takeFirst() { return takeAt(0); }
  T takeLast() { return takeAt(size() - 1); }
  void insert(int i, const T &v) { B::insert(B::begin() + i, v); }
  void replace(int i, const T &v) { (*this)[i] = v; }
  void swapItemsAt(int a, int b) { std::swap((*this)[a], (*this)[b]); }
  void move(int a, int b) {
    T v = takeAt(a);
    insert(b, v);
  }
  QList &fill(const T &v, int n = -1) {
    if (n >= 0)
      B::resize(n);
    std::fill(B::begin(), B::end(), v);
    return *this;
  }
  QList mid(int p, int n = -1) const {
    int e = n < 0 ? size() : std::min(size(), p + n);
    if (p < 0 || p >= e)
      return {};
    return QList(B::begin() + p, B::begin() + e);
  }
  auto constBegin() const { return B::cbegin(); }
  auto constEnd() const { return B::cend(); }
  const T *constData() const { return B::data(); }
  QList toList() const { return *this; }
  QList toVector() const { return *this; }
  void squeeze() { B::shrink_to_fit(); }
};
template <class T> using QVector = QList<T>;
inline QList<QByteArray> QByteArray::split(char sep) const {
  QList<QByteArray> r;
  size_t a = 0, b;
  while ((b = find(sep, a)) != npos) {
    r.append(substr(a, b - a));
    a = b + 1;
  }
  r.append(substr(a));
  return r;
}
inline QByteArray QByteArray::simplified() const {
  return QString(*this).simplified().toUtf8();
}
template <class T> class QQueue : public QList<T> {
public:
  void enqueue(const T &v) { this->append(v); }
  T dequeue() { return this->takeFirst(); }
  T &head() { return this->first(); }
};
class QStringList : public QList<QString> {
public:
  using QList::QList;
  QStringList() = default;
  QStringList(const QList<QString> &v) : QList(v) {}
  QStringList(const QString &s) { append(s); }
  QString join(const QString &sep) const {
    QString r;
    for (int i = 0; i < size(); ++i) {
      if (i)
        r += sep;
      r += (*this)[i];
    }
    return r;
  }
  QStringList &replaceInStrings(const QString &a, const QString &b) {
    for (auto &s : *this)
      s.replace(a, b);
    return *this;
  }
  void sort(Qt::CaseSensitivity cs = Qt::CaseSensitive) {
    std::sort(begin(), end(),
              [cs](auto &a, auto &b) { return a.compare(b, cs) < 0; });
  }
  int removeDuplicates() {
    std::set<QString> s;
    int n = size();
    erase(std::remove_if(begin(), end(),
                         [&](auto &v) { return !s.insert(v).second; }),
          end());
    return n - size();
  }
};
inline QStringList QString::split(const QString &sep,
                                  Qt::SplitBehavior behavior,
                                  Qt::CaseSensitivity cs) const {
  QStringList r;
  if (sep.isEmpty()) {
    if (behavior == Qt::KeepEmptyParts)
      r << "";
    for (auto c : s_)
      r << QString(QChar(c));
    if (behavior == Qt::KeepEmptyParts)
      r << "";
    return r;
  }
  int a = 0, b;
  while ((b = indexOf(sep, a, cs)) >= 0) {
    auto part = mid(a, b - a);
    if (behavior == Qt::KeepEmptyParts || !part.isEmpty())
      r << part;
    a = b + sep.size();
  }
  auto part = mid(a);
  if (behavior == Qt::KeepEmptyParts || !part.isEmpty())
    r << part;
  return r;
}
inline QString QString::section(const QString &sep, int start, int end) const {
  auto p = split(sep);
  if (start < 0)
    start += p.size();
  if (end < 0)
    end += p.size();
  return QStringList(p.mid(std::max(0, start), end - start + 1)).join(sep);
}

template <class K, class V, bool Multi = false> class PortableMap {
  using Storage =
      std::conditional_t<Multi, std::multimap<K, V>, std::map<K, V>>;
  Storage items_;

public:
  template <bool Const> class Iter {
    using Raw = std::conditional_t<Const, typename Storage::const_iterator,
                                   typename Storage::iterator>;
    Raw it_;
    friend class PortableMap;
    template <bool> friend class Iter;

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = V;
    using difference_type = std::ptrdiff_t;
    using pointer = std::conditional_t<Const, const V *, V *>;
    using reference = std::conditional_t<Const, const V &, V &>;
    Iter() = default;
    Iter(Raw i) : it_(i) {}
    template <bool C = Const, std::enable_if_t<C, int> = 0>
    Iter(const Iter<false> &i) : it_(i.it_) {}
    const K &key() const { return it_->first; }
    reference value() const { return it_->second; }
    reference operator*() const { return value(); }
    pointer operator->() const { return &value(); }
    Iter &operator++() {
      ++it_;
      return *this;
    }
    Iter operator++(int) {
      auto r = *this;
      ++*this;
      return r;
    }
    Iter &operator--() {
      --it_;
      return *this;
    }
    Iter operator--(int) {
      auto r = *this;
      --*this;
      return r;
    }
    bool operator==(const Iter &b) const { return it_ == b.it_; }
    bool operator!=(const Iter &b) const { return !(*this == b); }
  };
  using iterator = Iter<false>;
  using const_iterator = Iter<true>;
  using Iterator = iterator;
  using ConstIterator = const_iterator;
  iterator begin() { return items_.begin(); }
  iterator end() { return items_.end(); }
  const_iterator begin() const { return items_.begin(); }
  const_iterator end() const { return items_.end(); }
  const_iterator constBegin() const { return items_.cbegin(); }
  const_iterator constEnd() const { return items_.cend(); }
  const_iterator cbegin() const { return constBegin(); }
  const_iterator cend() const { return constEnd(); }
  int size() const { return static_cast<int>(items_.size()); }
  int count() const { return size(); }
  int count(const K &k) const { return static_cast<int>(items_.count(k)); }
  bool isEmpty() const { return items_.empty(); }
  void clear() { items_.clear(); }
  void reserve(int) {}
  void squeeze() {}
  bool contains(const K &k) const { return items_.find(k) != items_.end(); }
  V &operator[](const K &k) {
    if constexpr (!Multi)
      return items_[k];
    else {
      auto i = items_.find(k);
      if (i == items_.end())
        i = items_.emplace(k, V{});
      return i->second;
    }
  }
  V operator[](const K &k) const { return value(k); }
  V value(const K &k, V def = V{}) const {
    auto i = items_.find(k);
    return i == items_.end() ? def : i->second;
  }
  iterator insert(const K &k, const V &v) {
    if constexpr (Multi)
      // Qt prepends new entries to the range of an existing key. This order
      // controls ABE stand execution and therefore consumption of random draws.
      return items_.emplace_hint(items_.lower_bound(k), k, v);
    else
      return items_.insert_or_assign(k, v).first;
  }
  iterator insertMulti(const K &k, const V &v) { return insert(k, v); }
  iterator find(const K &k) { return items_.find(k); }
  const_iterator find(const K &k) const { return items_.find(k); }
  const_iterator constFind(const K &k) const { return items_.find(k); }
  iterator find(const K &k, const V &v) {
    for (auto i = lowerBound(k); i != upperBound(k); ++i)
      if (i.value() == v)
        return i;
    return end();
  }
  iterator lowerBound(const K &k) { return items_.lower_bound(k); }
  const_iterator lowerBound(const K &k) const { return items_.lower_bound(k); }
  iterator upperBound(const K &k) { return items_.upper_bound(k); }
  const_iterator upperBound(const K &k) const { return items_.upper_bound(k); }
  iterator erase(iterator i) { return items_.erase(i.it_); }
  int remove(const K &k) { return static_cast<int>(items_.erase(k)); }
  int remove(const K &k, const V &v) {
    int n = 0;
    for (auto i = items_.lower_bound(k); i != items_.upper_bound(k);)
      if (i->second == v) {
        i = items_.erase(i);
        ++n;
      } else
        ++i;
    return n;
  }
  V take(const K &k) {
    auto i = items_.find(k);
    if (i == items_.end())
      return {};
    V r = std::move(i->second);
    items_.erase(i);
    return r;
  }
  QList<K> keys() const {
    QList<K> r;
    for (auto &p : items_)
      r << p.first;
    return r;
  }
  QList<K> uniqueKeys() const {
    QList<K> r;
    for (auto &p : items_)
      if (r.isEmpty() || r.last() != p.first)
        r << p.first;
    return r;
  }
  QList<V> values() const {
    QList<V> r;
    for (auto &p : items_)
      r << p.second;
    return r;
  }
  QList<V> values(const K &k) const {
    QList<V> r;
    for (auto i = items_.lower_bound(k); i != items_.upper_bound(k); ++i)
      r << i->second;
    return r;
  }
  K key(const V &v, K def = {}) const {
    for (auto &p : items_)
      if (p.second == v)
        return p.first;
    return def;
  }
  V &first() { return items_.begin()->second; }
  const V &first() const { return items_.begin()->second; }
  V &last() { return items_.rbegin()->second; }
  const V &last() const { return items_.rbegin()->second; }
  K firstKey() const { return items_.begin()->first; }
  K lastKey() const { return items_.rbegin()->first; }
};
template <class K, class V> using QMap = PortableMap<K, V>;
template <class K, class V> using QHash = PortableMap<K, V>;
template <class K, class V> using QMultiMap = PortableMap<K, V, true>;
template <class K, class V> using QMultiHash = PortableMap<K, V, true>;
template <class A, class B> using PortablePair = std::pair<A, B>;
template <class A, class B> auto qMakePair(A a, B b) {
  return std::make_pair(a, b);
}
template <class K, class V, bool M = false> class PortableMapIterator {
  const PortableMap<K, V, M> &m_;
  typename PortableMap<K, V, M>::const_iterator next_, current_;

public:
  PortableMapIterator(const PortableMap<K, V, M> &m)
      : m_(m), next_(m.begin()) {}
  bool hasNext() const { return next_ != m_.end(); }
  void next() { current_ = next_++; }
  const K &key() const { return current_.key(); }
  const V &value() const { return current_.value(); }
};
template <class K, class V> using QMapIterator = PortableMapIterator<K, V>;
template <class K, class V>
using QMultiMapIterator = PortableMapIterator<K, V, true>;
template <class K, class V> class QMutableHashIterator {
  QHash<K, V> &m_;
  typename QHash<K, V>::iterator next_, current_;

public:
  QMutableHashIterator(QHash<K, V> &m) : m_(m), next_(m.begin()) {}
  bool hasNext() const { return next_ != m_.end(); }
  void next() { current_ = next_++; }
  const K &key() const { return current_.key(); }
  V &value() const { return current_.value(); }
  void remove() { m_.erase(current_); }
};
template <class T> class QSet : public std::set<T> {
public:
  using std::set<T>::set;
  bool contains(const T &v) const { return this->find(v) != this->end(); }
  bool isEmpty() const { return this->empty(); }
  QList<T> values() const { return {this->begin(), this->end()}; }
};
template <class T> using QSharedPointer = std::shared_ptr<T>;
template <class T> class QScopedPointer : public std::unique_ptr<T> {
public:
  using std::unique_ptr<T>::unique_ptr;
  T *data() const { return this->get(); }
  T *take() { return this->release(); }
};

class QVariant;
using QVariantList = QList<QVariant>;
using QVariantMap = QMap<QString, QVariant>;
class QVariant {
public:
  enum Type {
    Invalid,
    Bool,
    Int,
    UInt,
    LongLong,
    ULongLong,
    Double,
    String,
    ByteArray,
    List,
    Map
  };

private:
  std::variant<std::monostate, bool, qint64, double, QString, QByteArray,
               std::shared_ptr<QVariantList>, std::shared_ptr<QVariantMap>>
      v_;

public:
  QVariant() = default;
  QVariant(const char *s) : v_(QString(s)) {}
  QVariant(const QString &s) : v_(s) {}
  QVariant(const QByteArray &s) : v_(s) {}
  QVariant(bool v) : v_(v) {}
  QVariant(int v) : v_(qint64(v)) {}
  QVariant(uint v) : v_(qint64(v)) {}
  QVariant(qint64 v) : v_(v) {}
  QVariant(quint64 v) : v_(qint64(v)) {}
  QVariant(double v) : v_(v) {}
  QVariant(float v) : v_(double(v)) {}
  QVariant(const QVariantList &v) : v_(std::make_shared<QVariantList>(v)) {}
  QVariant(const QVariantMap &v) : v_(std::make_shared<QVariantMap>(v)) {}
  bool isValid() const { return v_.index() != 0; }
  bool isNull() const { return !isValid(); }
  Type type() const {
    static Type t[] = {Invalid, Bool,      LongLong, Double,
                       String,  ByteArray, List,     Map};
    return t[v_.index()];
  }
  int typeId() const { return type(); }
  template <class T> void setValue(const T &v) { *this = QVariant(v); }
  float toFloat(bool *ok = nullptr) const {
    return static_cast<float>(toDouble(ok));
  }
  QString toString() const {
    if (auto p = std::get_if<QString>(&v_))
      return *p;
    if (auto p = std::get_if<QByteArray>(&v_))
      return QString(*p);
    if (auto p = std::get_if<qint64>(&v_))
      return QString::number(*p);
    if (auto p = std::get_if<double>(&v_))
      return QString::number(*p, 'g', 15);
    if (auto p = std::get_if<bool>(&v_))
      return *p ? "true" : "false";
    return {};
  }
  double toDouble(bool *ok = nullptr) const {
    if (auto p = std::get_if<double>(&v_)) {
      if (ok)
        *ok = true;
      return *p;
    }
    if (auto p = std::get_if<qint64>(&v_)) {
      if (ok)
        *ok = true;
      return double(*p);
    }
    if (auto p = std::get_if<bool>(&v_)) {
      if (ok)
        *ok = true;
      return *p ? 1 : 0;
    }
    return toString().toDouble(ok);
  }
  int toInt(bool *ok = nullptr) const {
    if (type() == String)
      return toString().toInt(ok);
    if (ok)
      *ok = isValid();
    return static_cast<int>(toDouble());
  }
  uint toUInt(bool *ok = nullptr) const { return static_cast<uint>(toInt(ok)); }
  qint64 toLongLong(bool *ok = nullptr) const {
    if (auto p = std::get_if<qint64>(&v_)) {
      if (ok)
        *ok = true;
      return *p;
    }
    if (auto p = std::get_if<bool>(&v_)) {
      if (ok)
        *ok = true;
      return *p ? 1 : 0;
    }
    if (auto p = std::get_if<double>(&v_)) {
      if (ok)
        *ok = std::isfinite(*p);
      return std::isfinite(*p) ? static_cast<qint64>(*p) : 0;
    }
    return toString().toLongLong(ok);
  }
  bool toBool() const {
    if (auto p = std::get_if<bool>(&v_))
      return *p;
    auto s = toString().toLower();
    return !s.isEmpty() && s != "0" && s != "false";
  }
  QByteArray toByteArray() const {
    if (auto p = std::get_if<QByteArray>(&v_))
      return *p;
    return toString().toUtf8();
  }
  QVariantList toList() const {
    if (auto p = std::get_if<std::shared_ptr<QVariantList>>(&v_))
      return **p;
    return {};
  }
  QVariantMap toMap() const {
    if (auto p = std::get_if<std::shared_ptr<QVariantMap>>(&v_))
      return **p;
    return {};
  }
  friend bool operator==(const QVariant &a, const QVariant &b) {
    return a.v_ == b.v_;
  }
};
inline std::ostream &operator<<(std::ostream &s, const QVariant &v) {
  return s << v.toString();
}
template <class T> T qvariant_cast(const QVariant &v) {
  if constexpr (std::is_same_v<T, QVariantMap>)
    return v.toMap();
  else if constexpr (std::is_same_v<T, QVariantList>)
    return v.toList();
  else if constexpr (std::is_same_v<T, QString>)
    return v.toString();
  else
    return static_cast<T>(v.toDouble());
}

class QRegularExpressionMatch {
  QString text_;
  std::smatch match_;
  bool valid_ = false;

public:
  QRegularExpressionMatch() = default;
  QRegularExpressionMatch(const QString &s, const std::regex &r) : text_(s) {
    std::string t = s.toStdString();
    std::smatch m;
    if (std::regex_search(t, m, r)) {
      valid_ = true;
      for (auto &c : m)
        captures_ << QString(c.str());
      start_ = static_cast<int>(m.position());
      len_ = static_cast<int>(m.length());
    }
  }
  bool hasMatch() const { return valid_; }
  QString captured(int n = 0) const { return captures_.value(n); }
  QStringList capturedTexts() const { return captures_; }
  int capturedStart(int = 0) const { return start_; }
  int capturedLength(int = 0) const { return len_; }

private:
  QStringList captures_;
  int start_ = -1, len_ = 0;
};
class QRegularExpressionMatchIterator : public QList<QRegularExpressionMatch> {
  int i_ = 0;

public:
  bool hasNext() const { return i_ < size(); }
  QRegularExpressionMatch next() { return (*this)[i_++]; }
};
class QRegularExpression {
  QString pattern_;
  std::regex regex_;

public:
  enum PatternOption {
    NoPatternOption = 0,
    CaseInsensitiveOption = 1,
    DotMatchesEverythingOption = 2
  };
  QRegularExpression() = default;
  QRegularExpression(const QString &s, int opt = 0)
      : pattern_(s),
        regex_(s.toStdString(),
               std::regex::ECMAScript |
                   ((opt & 1) ? std::regex::icase : std::regex::flag_type{})) {}
  const std::regex &native() const { return regex_; }
  QString pattern() const { return pattern_; }
  QRegularExpressionMatchIterator globalMatch(const QString &s) const {
    QRegularExpressionMatchIterator r;
    auto text = s.toStdString();
    for (std::sregex_iterator i(text.begin(), text.end(), regex_), end;
         i != end; ++i)
      r.append(QRegularExpressionMatch(QString(i->str()), regex_));
    return r;
  }
  QRegularExpressionMatch match(const QString &s) const { return {s, regex_}; }
};
inline QString &QString::replace(const QRegularExpression &r,
                                 const QString &v) {
  *this =
      QString(std::regex_replace(toStdString(), r.native(), v.toStdString()));
  return *this;
}
inline QStringList QString::split(const QRegularExpression &r,
                                  Qt::SplitBehavior b) const {
  QStringList result;
  std::string s = toStdString();
  std::sregex_token_iterator i(s.begin(), s.end(), r.native(), -1), end;
  for (; i != end; ++i)
    if (b == Qt::KeepEmptyParts || !i->str().empty())
      result << QString(i->str());
  return result;
}

struct QMetaObject;
#define Q_OBJECT                                                               \
public:                                                                        \
  static QMetaObject staticMetaObject;                                         \
  virtual const QMetaObject *portableMetaObject() const {                      \
    return &staticMetaObject;                                                  \
  }                                                                            \
                                                                               \
private:
class QThread;
QThread *portableCurrentThread();
class QObject {
  QObject *parent_ = nullptr;
  std::vector<QObject *> children_;
  std::shared_ptr<bool> alive_ = std::make_shared<bool>(true);
  QThread *creatorThread_ = portableCurrentThread();

public:
  int portableOwnership = -1;
  QObject(QObject *p = nullptr) { setParent(p); }
  QObject(const QObject &) = delete;
  QObject &operator=(const QObject &) = delete;
  virtual ~QObject() {
    *alive_ = false;
    while (!children_.empty())
      delete children_.back();
    setParent(nullptr);
  }
  virtual const QMetaObject *portableMetaObject() const { return nullptr; }
  void setParent(QObject *p) {
    if (parent_) {
      auto &c = parent_->children_;
      c.erase(std::remove(c.begin(), c.end(), this), c.end());
    }
    parent_ = p;
    if (p)
      p->children_.push_back(this);
  }
  QObject *parent() const { return parent_; }
  std::weak_ptr<bool> portableLifetime() const { return alive_; }
  void deleteLater() { delete this; }
  QThread *thread() const;
};
class QWidget;
class QImage;
template <class T> T qobject_cast(QObject *o) { return dynamic_cast<T>(o); }
template <class T> T qobject_cast(const QObject *o) {
  return dynamic_cast<T>(o);
}

struct PortableLogFile {
  std::mutex mutex;
  std::ofstream stream;
};
inline void portableSetLogFile(const QString &fileName) {
  auto &log = SimulationContext::state<PortableLogFile>();
  std::lock_guard<std::mutex> lock(log.mutex);
  log.stream.close();
  log.stream.clear();
  if (!fileName.isEmpty()) {
    log.stream.open(std::filesystem::u8path(fileName.toStdString()), std::ios::out);
    if (!log.stream)
      throw std::runtime_error("Cannot open run log: " + fileName.toStdString());
  }
}
inline void portableWriteLog(const std::string &message) {
  if (SimulationContext::active()) {
    auto &log = SimulationContext::state<PortableLogFile>();
    std::lock_guard<std::mutex> lock(log.mutex);
    if (log.stream.is_open()) {
      log.stream << message << '\n';
      log.stream.flush();
    }
    std::clog << message << '\n';
  } else {
    std::clog << message << '\n';
  }
}
class PortableLog {
  std::shared_ptr<std::ostringstream> s_ =
      std::make_shared<std::ostringstream>();

public:
  PortableLog() = default;
  ~PortableLog() {
    if (s_.use_count() == 1)
      portableWriteLog(s_->str());
  }
  template <class T> PortableLog &operator<<(const T &v) {
    *s_ << v << ' ';
    return *this;
  }
  PortableLog &noquote() { return *this; }
  PortableLog &nospace() { return *this; }
};
inline PortableLog qDebug() { return {}; }
inline PortableLog qWarning() { return {}; }
inline PortableLog qCritical() { return {}; }
inline PortableLog qInfo() { return {}; }
inline void qDebug(const char *format, ...) {
  va_list args;
  va_start(args, format);
  va_list copy;
  va_copy(copy, args);
  int size = std::vsnprintf(nullptr, 0, format, copy);
  va_end(copy);
  std::vector<char> buffer(static_cast<size_t>(std::max(0, size)) + 1);
  if (size >= 0)
    std::vsnprintf(buffer.data(), buffer.size(), format, args);
  va_end(args);
  portableWriteLog(buffer.data());
}
template <class T>
std::ostream &operator<<(std::ostream &s, const QList<T> &v) {
  s << '[';
  for (auto &x : v)
    s << x << ',';
  return s << ']';
}
class QLoggingCategory {
public:
  QLoggingCategory(const char *) {}
  static void setFilterRules(const QString &) {}
  const QLoggingCategory &operator()() const { return *this; }
  bool isDebugEnabled() const { return false; }
};
#define Q_DECLARE_LOGGING_CATEGORY(name) extern QLoggingCategory name;
#define Q_LOGGING_CATEGORY(name, ...) QLoggingCategory name(__VA_ARGS__);
#define qCDebug(category)                                                      \
  if (!(category).isDebugEnabled()) {                                          \
  } else                                                                       \
    qDebug()
#define qCInfo(category) qInfo()
#define qCWarning(category) qWarning()
#define qCCritical(category) qCritical()
inline PortableLog qDebug(const QLoggingCategory &) { return {}; }

class QMutex : public std::mutex {};
class QMutexLocker {
  QMutex *m_;

public:
  explicit QMutexLocker(QMutex *m) : m_(m) { m_->lock(); }
  ~QMutexLocker() {
    if (m_)
      m_->unlock();
  }
  void unlock() {
    m_->unlock();
    m_ = nullptr;
  }
};
class QWaitCondition {
  std::condition_variable c_;

public:
  bool wait(QMutex *m, unsigned long ms = ULONG_MAX) {
    std::unique_lock<std::mutex> l(*m, std::adopt_lock);
    bool r = true;
    if (ms == ULONG_MAX)
      c_.wait(l);
    else
      r = c_.wait_for(l, std::chrono::milliseconds(ms)) !=
          std::cv_status::timeout;
    l.release();
    return r;
  }
  void wakeAll() { c_.notify_all(); }
  void wakeOne() { c_.notify_one(); }
};
class QAtomicInt : public std::atomic<int> {
public:
  using std::atomic<int>::atomic;
  QAtomicInt() : std::atomic<int>(0) {}
  int fetchAndAddRelaxed(int n) {
    return fetch_add(n, std::memory_order_relaxed);
  }
  bool testAndSetOrdered(int expected, int desired) {
    return compare_exchange_strong(expected, desired);
  }
  int loadRelaxed() const { return load(std::memory_order_relaxed); }
  void storeRelaxed(int v) { store(v, std::memory_order_relaxed); }
};
class QThread {
public:
  static QThread *currentThread() {
    thread_local QThread t;
    return &t;
  }
  static int idealThreadCount() {
    return std::max(1u, std::thread::hardware_concurrency());
  }
  static void msleep(unsigned long n) {
    std::this_thread::sleep_for(std::chrono::milliseconds(n));
  }
};
inline QThread *portableCurrentThread() { return QThread::currentThread(); }
inline QThread *QObject::thread() const { return creatorThread_; }
class QThreadPool {
  int n_ = QThread::idealThreadCount();

public:
  static QThreadPool *globalInstance() {
    return &SimulationContext::state<QThreadPool>();
  }
  void setMaxThreadCount(int n) { n_ = std::max(1, n); }
  int maxThreadCount() const { return n_; }
  void waitForDone() {}
};
class QCoreApplication : public QObject {
  static QCoreApplication *&instanceSlot() {
    struct State { QCoreApplication *application = nullptr; };
    return SimulationContext::state<State>().application;
  }

public:
  QCoreApplication(int &, char **) {
    if (instanceSlot())
      throw std::runtime_error("Only one application allowed");
    instanceSlot() = this;
  }
  ~QCoreApplication() { instanceSlot() = nullptr; }
  static QCoreApplication *instance() { return instanceSlot(); }
  static void processEvents() {}
  static QString applicationVersion() { return "iland-portable"; }
  static void setApplicationVersion(const QString &) {}
  static QString applicationDirPath();
};
class QElapsedTimer {
  using Clock = std::chrono::steady_clock;
  Clock::time_point t_ = Clock::now();

public:
  void start() { t_ = Clock::now(); }
  qint64 elapsed() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() -
                                                                 t_)
        .count();
  }
  qint64 restart() {
    auto e = elapsed();
    start();
    return e;
  }
  qint64 nsecsElapsed() const {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() -
                                                                t_)
        .count();
  }
};
class QDateTime {
public:
  static qint64 currentMSecsSinceEpoch() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
  }
  static QDateTime currentDateTime() { return {}; }
  QString toString(QString f) const {
    f.replace("yyyy", "%Y")
        .replace("MM", "%m")
        .replace("dd", "%d")
        .replace("hh", "%H")
        .replace("mm", "%M")
        .replace("ss", "%S");
    auto now = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    std::ostringstream s;
    s << std::put_time(&tm, f.toStdString().c_str());
    return s.str();
  }
};
class QTime {
  int ms_ = 0;

public:
  static QTime currentTime() {
    QTime t;
    t.ms_ = static_cast<int>(QDateTime::currentMSecsSinceEpoch() % 86400000);
    return t;
  }
  int msecsTo(const QTime &t) const { return t.ms_ - ms_; }
  int msecsSinceStartOfDay() const { return ms_; }
};

namespace QtConcurrent {
template <class C, class F> void blockingMap(C &c, F f) {
  auto n = c.size();
  if (n == 0)
    return;
  std::atomic<int> next{0};
  std::exception_ptr error;
  std::mutex mutex;
  auto &context = SimulationContext::current();
  auto work = [&]() {
    try {
      SimulationContext::Scope scope(context);
      for (int i; (i = next.fetch_add(1)) < n;)
        f(c[i]);
    } catch (...) {
      std::lock_guard<std::mutex> g(mutex);
      if (!error)
        error = std::current_exception();
      next = n;
    }
  };
  std::vector<std::thread> threads;
  int count = std::min(n, QThreadPool::globalInstance()->maxThreadCount());
  for (int i = 1; i < count; ++i)
    threads.emplace_back(work);
  work();
  for (auto &t : threads)
    t.join();
  if (error)
    std::rethrow_exception(error);
}
template <class F, class... A> auto run(F f, A... a) {
  auto *context = &SimulationContext::current();
  return std::async(std::launch::async,
      [context, f = std::move(f), arguments = std::make_tuple(std::move(a)...)]() mutable {
        SimulationContext::Scope scope(*context);
        return std::apply(f, std::move(arguments));
      });
}
} // namespace QtConcurrent
using QFuture = std::future<void>;
