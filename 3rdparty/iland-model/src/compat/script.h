#pragma once
#include "geometry.h"
#include <quickjs.h>
#include <typeinfo>
class QJSEngine;
struct PortableScriptRuntime {
  JSRuntime *rt = nullptr;
  JSContext *ctx = nullptr;
  JSClassID objectClass = 0;
  std::recursive_mutex mutex;
  unsigned entryDepth = 0;
  struct Guard {
    PortableScriptRuntime &runtime;
    std::unique_lock<std::recursive_mutex> lock;
    explicit Guard(PortableScriptRuntime &r) : runtime(r), lock(r.mutex) {
      if (runtime.entryDepth++ == 0 && runtime.rt)
        JS_UpdateStackTop(runtime.rt);
    }
    ~Guard() { --runtime.entryDepth; }
  };
  ~PortableScriptRuntime() {
    if (ctx)
      JS_FreeContext(ctx);
    if (rt)
      JS_FreeRuntime(rt);
  }
};
class QJSValue {
  friend class QJSEngine;
  friend class QJSValueIterator;
  std::shared_ptr<PortableScriptRuntime> runtime_;
  JSValue value_ = JS_UNDEFINED;
  std::variant<std::monostate, std::nullptr_t, bool, double, QString> plain_;

public:
  enum SpecialValue { NullValue, UndefinedValue };
  enum ErrorType {
    GenericError,
    EvalError,
    RangeError,
    ReferenceError,
    SyntaxError,
    TypeError,
    URIError
  };
  QJSValue() = default;
  QJSValue(SpecialValue v) {
    if (v == NullValue)
      plain_ = nullptr;
  }
  QJSValue(bool b) : plain_(b) {}
  QJSValue(int n) : plain_(double(n)) {}
  QJSValue(uint n) : plain_(double(n)) {}
  QJSValue(qint64 n) : plain_(double(n)) {}
  QJSValue(double n) : plain_(n) {}
  QJSValue(const char *s) : plain_(QString(s)) {}
  QJSValue(const QString &s) : plain_(s) {}
  QJSValue(std::shared_ptr<PortableScriptRuntime> r, JSValue v)
      : runtime_(std::move(r)), value_(v) {}
  QJSValue(const QJSValue &o) : runtime_(o.runtime_), plain_(o.plain_) {
    if (runtime_) {
      PortableScriptRuntime::Guard l(*runtime_);
      value_ = JS_DupValue(runtime_->ctx, o.value_);
    }
  }
  QJSValue(QJSValue &&o) noexcept
      : runtime_(std::move(o.runtime_)), value_(o.value_),
        plain_(std::move(o.plain_)) {
    o.value_ = JS_UNDEFINED;
  }
  ~QJSValue() {
    if (runtime_) {
      PortableScriptRuntime::Guard l(*runtime_);
      JS_FreeValue(runtime_->ctx, value_);
    }
  }
  QJSValue &operator=(QJSValue o) {
    swap(o);
    return *this;
  }
  void swap(QJSValue &o) noexcept {
    runtime_.swap(o.runtime_);
    std::swap(value_, o.value_);
    plain_.swap(o.plain_);
  }
  JSValue native(JSContext *) const;
  bool isUndefined() const {
    return runtime_ ? JS_IsUndefined(value_) : plain_.index() == 0;
  }
  bool isNull() const {
    return runtime_ ? JS_IsNull(value_) : plain_.index() == 1;
  }
  bool isBool() const {
    return runtime_ ? JS_IsBool(value_) : plain_.index() == 2;
  }
  bool isNumber() const {
    return runtime_ ? JS_IsNumber(value_) : plain_.index() == 3;
  }
  bool isString() const {
    return runtime_ ? JS_IsString(value_) : plain_.index() == 4;
  }
  bool isObject() const { return runtime_ && JS_IsObject(value_); }
  bool isArray() const;
  bool isCallable() const;
  bool isError() const;
  bool isQObject() const { return toQObject() != nullptr; }
  QString toString() const;
  double toNumber() const;
  int toInt() const;
  uint toUInt() const;
  bool toBool() const;
  QObject *toQObject() const;
  QVariant toVariant() const;
  QJSValue property(const QString &) const;
  QJSValue property(uint n) const { return property(QString::number(n)); }
  void setProperty(const QString &, const QJSValue &);
  void setProperty(uint n, const QJSValue &v) {
    setProperty(QString::number(n), v);
  }
  bool hasProperty(const QString &) const;
  bool hasOwnProperty(const QString &) const;
  bool deleteProperty(const QString &);
  QJSValue call(const QList<QJSValue> &args = {}) const;
  QJSValue callWithInstance(const QJSValue &,
                            const QList<QJSValue> &args = {}) const;
  bool equals(const QJSValue &) const;
  bool strictlyEquals(const QJSValue &) const;
};
using QJSValueList = QList<QJSValue>;
struct PortableMethod {
  QString name;
  int minimum = 0, maximum = 0;
  std::function<QJSValue(QJSEngine &, QObject *, const QJSValueList &)> invoke;
  std::function<bool(const QJSValueList &)> accepts;
};
struct PortableProperty {
  QString name;
  std::function<QJSValue(QJSEngine &, QObject *)> get;
  std::function<void(QJSEngine &, QObject *, const QJSValue &)> set;
};
struct QMetaObject {
  const char *name = "QObject";
  const QMetaObject *base = nullptr;
  QList<PortableMethod> methods;
  QList<PortableProperty> properties;
  QList<std::pair<QString, int>> enums;
  std::function<QObject *(QJSEngine &, const QJSValueList &)> construct;
};
struct PortableObjectHandle {
  QObject *object;
  std::weak_ptr<bool> alive;
  bool scriptOwned = false;
  QJSEngine *engine = nullptr;
};
class PortableOpaquePointer : public QObject {
public:
  void *pointer;
  const std::type_info &type;
  PortableOpaquePointer(void *p, const std::type_info &t, QObject *parent)
      : QObject(parent), pointer(p), type(t) {}
};
class QJSEngine : public QObject {
  friend class QJSValue;
  std::shared_ptr<PortableScriptRuntime> runtime_;
  std::vector<std::function<QJSValue(const QJSValue &, const QJSValueList &)>>
      callbacks_;
  std::map<QObject *, JSValue> objects_;
  std::map<const QMetaObject *, JSValue> prototypes_;
  std::map<QString, QJSValue> modules_;
  static void finalize(JSRuntime *, JSValue);
  static JSValue callback(JSContext *, JSValueConst, int, JSValueConst *, int,
                          JSValue *);
  JSValue
  function(const QString &, int,
           std::function<QJSValue(const QJSValue &, const QJSValueList &)>);

public:
  enum Extension {
    ConsoleExtension = 1,
    GarbageCollectionExtension = 2,
    AllExtensions = 3
  };
  QJSEngine();
  ~QJSEngine();
  JSContext *nativeContext() const { return runtime_->ctx; }
  std::shared_ptr<PortableScriptRuntime> runtime() const { return runtime_; }
  QJSValue adopt(JSValue v) {
    PortableScriptRuntime::Guard guard(*runtime_);
    if (JS_IsException(v))
      v = JS_GetException(nativeContext());
    return {runtime_, v};
  }
  QJSValue evaluate(const QString &, const QString &fileName = {},
                    int lineNumber = 1);
  QJSValue importModule(const QString &);
  QJSValue globalObject() { PortableScriptRuntime::Guard guard(*runtime_); return adopt(JS_GetGlobalObject(nativeContext())); }
  QJSValue newObject() { PortableScriptRuntime::Guard guard(*runtime_); return adopt(JS_NewObject(nativeContext())); }
  QJSValue newArray(uint n = 0) {
    PortableScriptRuntime::Guard guard(*runtime_);
    auto a = adopt(JS_NewArray(nativeContext()));
    a.setProperty("length", n);
    return a;
  }
  QJSValue newQObject(QObject *);
  QJSValue newQMetaObject(const QMetaObject *);
  void installExtensions(int);
  void collectGarbage() { PortableScriptRuntime::Guard guard(*runtime_); JS_RunGC(runtime_->rt); }
  void throwError(const QString &s) {
    throw std::runtime_error(s.toStdString());
  }
  void throwError(QJSValue::ErrorType, const QString &s) { throwError(s); }
  QJSValue newErrorObject(QJSValue::ErrorType, const QString &);
  template <class T> QJSValue toScriptValue(const T &);
  template <class T> T fromScriptValue(const QJSValue &);
  static void setObjectOwnership(QObject *o, int ownership) {
    if (o)
      o->portableOwnership = ownership;
  }
};
class QQmlEngine {
public:
  enum ObjectOwnership { CppOwnership, JavaScriptOwnership };
  static void setObjectOwnership(QObject *o, ObjectOwnership ownership) {
    QJSEngine::setObjectOwnership(o, ownership);
  }
};
template <class T> T qjsvalue_cast(const QJSValue &v) {
  if constexpr (std::is_same_v<T, QObject *>)
    return v.toQObject();
  else if constexpr (std::is_same_v<T, QString>)
    return v.toString();
  else if constexpr (std::is_same_v<T, bool>)
    return v.toBool();
  else
    return static_cast<T>(v.toNumber());
}
template <class T> struct PortableIsList : std::false_type {};
template <class T> struct PortableIsList<QList<T>> : std::true_type {
  using Element = T;
};
template <class T> QJSValue QJSEngine::toScriptValue(const T &v) {
  using U = std::decay_t<T>;
  if constexpr (std::is_same_v<U, QJSValue>)
    return v;
  else if constexpr (std::is_same_v<U, QString>)
    return QJSValue(v);
  else if constexpr (std::is_same_v<U, bool>)
    return QJSValue(v);
  else if constexpr (std::is_arithmetic_v<U> || std::is_enum_v<U>)
    return QJSValue(double(v));
  else if constexpr (std::is_pointer_v<U> &&
                     std::is_base_of_v<QObject, std::remove_pointer_t<U>>)
    return newQObject(
        const_cast<std::remove_const_t<std::remove_pointer_t<U>> *>(v));
  else if constexpr (std::is_pointer_v<U>) {
    if (!v)
      return QJSValue(QJSValue::NullValue);
    return newQObject(new PortableOpaquePointer(
        const_cast<void *>(static_cast<const void *>(v)), typeid(U), this));
  } else if constexpr (std::is_same_v<U, QRectF>) {
    auto a = newObject();
    a.setProperty("x", v.x());
    a.setProperty("y", v.y());
    a.setProperty("width", v.width());
    a.setProperty("height", v.height());
    return a;
  } else if constexpr (std::is_same_v<U, QStringList> ||
                       PortableIsList<U>::value) {
    auto a = newArray();
    uint i = 0;
    for (auto &item : v)
      a.setProperty(i++, toScriptValue(item));
    return a;
  } else if constexpr (std::is_same_v<U, QVariant>) {
    switch (v.type()) {
    case QVariant::Invalid:
      return QJSValue(QJSValue::NullValue);
    case QVariant::Bool:
      return v.toBool();
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::UInt:
    case QVariant::Double:
      return v.toDouble();
    case QVariant::List:
      return toScriptValue(v.toList());
    case QVariant::Map:
      return toScriptValue(v.toMap());
    default:
      return v.toString();
    }
  } else if constexpr (std::is_same_v<U, QVariantMap>) {
    auto a = newObject();
    for (auto i = v.begin(); i != v.end(); ++i)
      a.setProperty(i.key(), toScriptValue(i.value()));
    return a;
  } else
    static_assert(!sizeof(T), "Missing C++ to script conversion");
}
template <class T> T QJSEngine::fromScriptValue(const QJSValue &v) {
  using U = std::decay_t<T>;
  if constexpr (std::is_same_v<U, QJSValue>)
    return v;
  else if constexpr (std::is_same_v<U, QString>)
    return v.toString();
  else if constexpr (std::is_same_v<U, bool>)
    return v.toBool();
  else if constexpr (std::is_integral_v<U> || std::is_enum_v<U>)
    return static_cast<U>(v.toNumber());
  else if constexpr (std::is_floating_point_v<U>)
    return static_cast<U>(v.toNumber());
  else if constexpr (std::is_pointer_v<U> &&
                     std::is_base_of_v<QObject, std::remove_pointer_t<U>>) {
    if (v.isNull() || v.isUndefined())
      return nullptr;
    auto o = dynamic_cast<U>(v.toQObject());
    if (!o)
      throw std::runtime_error("Invalid script object argument");
    return o;
  } else if constexpr (std::is_same_v<U, QVariant>)
    return v.toVariant();
  else if constexpr (std::is_same_v<U, QVariantMap>)
    return v.toVariant().toMap();
  else if constexpr (std::is_pointer_v<U>) {
    if (v.isNull() || v.isUndefined())
      return nullptr;
    auto h = dynamic_cast<PortableOpaquePointer *>(v.toQObject());
    if (!h || h->type != typeid(U))
      throw std::runtime_error("Invalid native pointer argument");
    return static_cast<U>(h->pointer);
  } else if constexpr (std::is_same_v<U, QRectF>) {
    return QRectF(v.property("x").toNumber(), v.property("y").toNumber(),
                  v.property("width").toNumber(),
                  v.property("height").toNumber());
  } else if constexpr (std::is_same_v<U, QStringList> ||
                       PortableIsList<U>::value) {
    U list;
    int n = v.property("length").toInt();
    for (int i = 0; i < n; ++i)
      list.append(fromScriptValue<typename U::value_type>(v.property(uint(i))));
    return list;
  } else
    static_assert(!sizeof(T), "Missing script to C++ conversion");
}
class QJSValueIterator {
  QJSValue object_;
  QStringList names_;
  int i_ = -1;

public:
  explicit QJSValueIterator(const QJSValue &);
  bool hasNext() const { return i_ + 1 < names_.size(); }
  void next() { ++i_; }
  QString name() const { return names_.value(i_); }
  QJSValue value() const { return object_.property(name()); }
  void setValue(const QJSValue &v) { object_.setProperty(name(), v); }
};
