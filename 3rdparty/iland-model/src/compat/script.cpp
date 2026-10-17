#include "script.h"
#include "io.h"
namespace {
JSModuleDef *loadModule(JSContext *ctx, const char *name, void *) {
  try {
    QFile file{QString(name)};
    if (!file.open(QIODevice::ReadOnly)) {
      JS_ThrowReferenceError(ctx, "Cannot open module '%s'", name);
      return nullptr;
    }
    auto source = file.readAll();
    auto compiled = JS_Eval(ctx, source.data(), source.size(), name,
                            JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
    if (JS_IsException(compiled))
      return nullptr;
    auto module = static_cast<JSModuleDef *>(JS_VALUE_GET_PTR(compiled));
    JS_FreeValue(ctx, compiled);
    return module;
  } catch (const std::exception &e) {
    JS_ThrowInternalError(ctx, "%s", e.what());
    return nullptr;
  }
}
} // namespace
JSValue QJSValue::native(JSContext *ctx) const {
  if (runtime_) {
    PortableScriptRuntime::Guard guard(*runtime_);
    if (ctx != runtime_->ctx)
      throw std::runtime_error(
          "JavaScript values belong to different model runs");
    return JS_DupValue(ctx, value_);
  }
  switch (plain_.index()) {
  case 1:
    return JS_NULL;
  case 2:
    return JS_NewBool(ctx, std::get<bool>(plain_));
  case 3:
    return JS_NewFloat64(ctx, std::get<double>(plain_));
  case 4: {
    auto s = std::get<QString>(plain_).toUtf8();
    return JS_NewStringLen(ctx, s.data(), s.size());
  }
  default:
    return JS_UNDEFINED;
  }
}
QString QJSValue::toString() const {
  if (!runtime_) {
    switch (plain_.index()) {
    case 0:
      return "undefined";
    case 1:
      return "null";
    case 2:
      return std::get<bool>(plain_) ? "true" : "false";
    case 3:
      return QString::number(std::get<double>(plain_), 'g', 16);
    case 4:
      return std::get<QString>(plain_);
    }
  }
  PortableScriptRuntime::Guard l(*runtime_);
  const char *s = JS_ToCString(runtime_->ctx, value_);
  if (!s)
    return {};
  QString r(s);
  JS_FreeCString(runtime_->ctx, s);
  return r;
}
double QJSValue::toNumber() const {
  if (!runtime_) {
    if (plain_.index() == 1)
      return 0;
    if (plain_.index() == 2)
      return std::get<bool>(plain_) ? 1 : 0;
    if (plain_.index() == 3)
      return std::get<double>(plain_);
    if (plain_.index() == 4) {
      bool ok;
      auto s = std::get<QString>(plain_).trimmed();
      if (s.isEmpty())
        return 0;
      auto n = s.toDouble(&ok);
      if (ok)
        return n;
    }
    return std::numeric_limits<double>::quiet_NaN();
  }
  PortableScriptRuntime::Guard l(*runtime_);
  double n = 0;
  JS_ToFloat64(runtime_->ctx, &n, value_);
  return n;
}
bool QJSValue::toBool() const {
  if (!runtime_) {
    if (plain_.index() < 2)
      return false;
    if (plain_.index() == 2)
      return std::get<bool>(plain_);
    if (plain_.index() == 3) {
      double n = std::get<double>(plain_);
      return n != 0 && !std::isnan(n);
    }
    return !std::get<QString>(plain_).isEmpty();
  }
  PortableScriptRuntime::Guard l(*runtime_);
  return JS_ToBool(runtime_->ctx, value_) > 0;
}
int QJSValue::toInt() const {
  if (runtime_) {
    PortableScriptRuntime::Guard l(*runtime_);
    int32_t n = 0;
    JS_ToInt32(runtime_->ctx, &n, value_);
    return n;
  }
  double n = toNumber();
  if (!std::isfinite(n) || n == 0)
    return 0;
  double u = std::fmod(std::trunc(n), 4294967296.);
  if (u < 0)
    u += 4294967296.;
  return u >= 2147483648. ? static_cast<int32_t>(u - 4294967296.)
                          : static_cast<int32_t>(u);
}
uint QJSValue::toUInt() const { return static_cast<uint>(toInt()); }
bool QJSValue::isArray() const {
  if (!runtime_)
    return false;
  PortableScriptRuntime::Guard l(*runtime_);
  return JS_IsArray(value_);
}
bool QJSValue::isCallable() const {
  if (!runtime_)
    return false;
  PortableScriptRuntime::Guard l(*runtime_);
  return JS_IsFunction(runtime_->ctx, value_);
}
bool QJSValue::isError() const {
  if (!runtime_)
    return false;
  PortableScriptRuntime::Guard l(*runtime_);
  return JS_IsError(value_);
}
QObject *QJSValue::toQObject() const {
  if (!runtime_)
    return nullptr;
  PortableScriptRuntime::Guard l(*runtime_);
  auto h = static_cast<PortableObjectHandle *>(
      JS_GetOpaque(value_, runtime_->objectClass));
  if (!h)
    return nullptr;
  auto a = h->alive.lock();
  return a && *a ? h->object : nullptr;
}
QJSValue QJSValue::property(const QString &name) const {
  if (!runtime_)
    return {};
  PortableScriptRuntime::Guard l(*runtime_);
  auto v = JS_GetPropertyStr(runtime_->ctx, value_, name.toUtf8().constData());
  if (JS_IsException(v))
    v = JS_GetException(runtime_->ctx);
  return {runtime_, v};
}
void QJSValue::setProperty(const QString &name, const QJSValue &v) {
  if (!runtime_)
    throw std::runtime_error("Cannot set a property on a primitive value");
  PortableScriptRuntime::Guard l(*runtime_);
  if (JS_SetPropertyStr(runtime_->ctx, value_, name.toUtf8().constData(),
                        v.native(runtime_->ctx)) < 0) {
    QJSValue e(runtime_, JS_GetException(runtime_->ctx));
    throw std::runtime_error(e.toString().toStdString());
  }
}
bool QJSValue::hasProperty(const QString &name) const {
  if (!runtime_)
    return false;
  PortableScriptRuntime::Guard l(*runtime_);
  auto atom = JS_NewAtom(runtime_->ctx, name.toUtf8().constData());
  int r = JS_HasProperty(runtime_->ctx, value_, atom);
  JS_FreeAtom(runtime_->ctx, atom);
  return r > 0;
}
bool QJSValue::hasOwnProperty(const QString &name) const {
  if (!runtime_)
    return false;
  PortableScriptRuntime::Guard l(*runtime_);
  auto atom = JS_NewAtom(runtime_->ctx, name.toUtf8().constData());
  JSPropertyDescriptor d{};
  int r = JS_GetOwnProperty(runtime_->ctx, &d, value_, atom);
  JS_FreeAtom(runtime_->ctx, atom);
  if (r > 0) {
    JS_FreeValue(runtime_->ctx, d.value);
    JS_FreeValue(runtime_->ctx, d.getter);
    JS_FreeValue(runtime_->ctx, d.setter);
  }
  return r > 0;
}
bool QJSValue::deleteProperty(const QString &name) {
  if (!runtime_)
    return false;
  PortableScriptRuntime::Guard l(*runtime_);
  auto atom = JS_NewAtom(runtime_->ctx, name.toUtf8().constData());
  int r = JS_DeleteProperty(runtime_->ctx, value_, atom, 0);
  JS_FreeAtom(runtime_->ctx, atom);
  return r > 0;
}
QJSValue QJSValue::call(const QJSValueList &a) const {
  return callWithInstance(QJSValue(), a);
}
QJSValue QJSValue::callWithInstance(const QJSValue &self,
                                    const QJSValueList &a) const {
  if (!runtime_)
    return {};
  PortableScriptRuntime::Guard l(*runtime_);
  std::vector<JSValue> args;
  for (auto &v : a)
    args.push_back(v.native(runtime_->ctx));
  auto receiver = self.native(runtime_->ctx);
  auto result = JS_Call(runtime_->ctx, value_, receiver,
                        static_cast<int>(args.size()), args.data());
  JS_FreeValue(runtime_->ctx, receiver);
  for (auto v : args)
    JS_FreeValue(runtime_->ctx, v);
  if (JS_IsException(result))
    result = JS_GetException(runtime_->ctx);
  return {runtime_, result};
}
bool QJSValue::strictlyEquals(const QJSValue &other) const {
  auto r = runtime_ ? runtime_ : other.runtime_;
  if (!r)
    return plain_ == other.plain_;
  PortableScriptRuntime::Guard l(*r);
  auto a = native(r->ctx), b = other.native(r->ctx);
  int eq = JS_IsStrictEqual(r->ctx, a, b);
  JS_FreeValue(r->ctx, a);
  JS_FreeValue(r->ctx, b);
  return eq > 0;
}
bool QJSValue::equals(const QJSValue &o) const {
  if (isNull() && o.isUndefined() || isUndefined() && o.isNull())
    return true;
  if (isNumber() || o.isNumber() || isBool() || o.isBool())
    return toNumber() == o.toNumber();
  return strictlyEquals(o);
}
QVariant QJSValue::toVariant() const {
  if (isUndefined() || isNull())
    return {};
  if (isBool())
    return toBool();
  if (isNumber())
    return toNumber();
  if (isString())
    return toString();
  if (isArray()) {
    QVariantList l;
    int n = property("length").toInt();
    for (int i = 0; i < n; ++i)
      l << property(uint(i)).toVariant();
    return l;
  }
  QVariantMap m;
  QJSValueIterator it(*this);
  while (it.hasNext()) {
    it.next();
    m.insert(it.name(), it.value().toVariant());
  }
  return m;
}
QJSValueIterator::QJSValueIterator(const QJSValue &o) : object_(o) {
  if (!o.runtime_ || !o.isObject())
    return;
  auto r = o.runtime_;
  PortableScriptRuntime::Guard l(*r);
  JSPropertyEnum *props = nullptr;
  uint32_t count = 0;
  if (JS_GetOwnPropertyNames(r->ctx, &props, &count, o.value_,
                             JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY) < 0)
    return;
  for (uint32_t i = 0; i < count; ++i) {
    const char *s = JS_AtomToCString(r->ctx, props[i].atom);
    if (s) {
      names_ << QString(s);
      JS_FreeCString(r->ctx, s);
    }
    JS_FreeAtom(r->ctx, props[i].atom);
  }
  js_free(r->ctx, props);
}
void QJSEngine::finalize(JSRuntime *, JSValue v) {
  auto h = static_cast<PortableObjectHandle *>(JS_GetOpaque(v, JS_GetClassID(v)));
  if (!h)
    return;
  auto alive = h->alive.lock();
  if (h->engine) {
    auto i = h->engine->objects_.find(h->object);
    if (i != h->engine->objects_.end() &&
        JS_VALUE_GET_PTR(i->second) == JS_VALUE_GET_PTR(v))
      h->engine->objects_.erase(i);
  }
  if (alive && *alive &&
      (h->object->portableOwnership == 1 ||
       (h->object->portableOwnership < 0 && h->scriptOwned)) &&
      !h->object->parent())
    delete h->object;
  delete h;
}
JSValue QJSEngine::callback(JSContext *ctx, JSValueConst self, int argc,
                            JSValueConst *argv, int, JSValue *data) {
  auto e = static_cast<QJSEngine *>(JS_GetContextOpaque(ctx));
  if (!e)
    return JS_ThrowInternalError(ctx, "The script engine has been destroyed");
  try {
    int32_t id;
    JS_ToInt32(ctx, &id, data[0]);
    QJSValueList a;
    for (int i = 0; i < argc; ++i)
      a << e->adopt(JS_DupValue(ctx, argv[i]));
    auto invoke = e->callbacks_.at(id);
    auto value = invoke(e->adopt(JS_DupValue(ctx, self)), a);
    return value.native(ctx);
  } catch (const std::exception &ex) {
    return JS_ThrowInternalError(ctx, "%s", ex.what());
  } catch (...) {
    return JS_ThrowInternalError(ctx, "Native simulation error");
  }
}
JSValue QJSEngine::function(
    const QString &name, int argc,
    std::function<QJSValue(const QJSValue &, const QJSValueList &)> cb) {
  int id = static_cast<int>(callbacks_.size());
  callbacks_.push_back(std::move(cb));
  auto data = JS_NewInt32(nativeContext(), id);
  auto f = JS_NewCFunctionData(nativeContext(), callback, argc, 0, 1, &data);
  JS_DefinePropertyValueStr(
      nativeContext(), f, "name",
      JS_NewString(nativeContext(), name.toUtf8().constData()),
      JS_PROP_CONFIGURABLE);
  return f;
}
QJSEngine::QJSEngine() : runtime_(std::make_shared<PortableScriptRuntime>()) {
  runtime_->rt = JS_NewRuntime();
  if (!runtime_->rt)
    throw std::bad_alloc();
  JS_SetMaxStackSize(runtime_->rt, 8 * 1024 * 1024);
  runtime_->ctx = JS_NewContext(runtime_->rt);
  if (!runtime_->ctx)
    throw std::bad_alloc();
  JS_SetContextOpaque(nativeContext(), this);
  JS_SetModuleLoaderFunc(runtime_->rt, nullptr, loadModule, nullptr);
  JS_NewClassID(runtime_->rt, &runtime_->objectClass);
  JSClassDef def{};
  def.class_name = "iLandObject";
  def.finalizer = finalize;
  if (JS_NewClass(runtime_->rt, runtime_->objectClass, &def) < 0)
    throw std::runtime_error("Cannot register native script objects");
}
QJSEngine::~QJSEngine() {
  JS_SetContextOpaque(nativeContext(), nullptr);
  std::vector<std::pair<QObject *, std::weak_ptr<bool>>> owned;
  for (auto &p : objects_) {
    auto h = static_cast<PortableObjectHandle *>(
        JS_GetOpaque(p.second, runtime_->objectClass));
    h->engine = nullptr;
    auto alive = h->alive.lock();
    if (alive && *alive &&
        (h->object->portableOwnership == 1 ||
         (h->object->portableOwnership < 0 && h->scriptOwned)) &&
        !h->object->parent())
      owned.emplace_back(h->object, h->alive);
  }
  objects_.clear();
  for (auto &p : owned) {
    auto alive = p.second.lock();
    if (alive && *alive)
      delete p.first;
  }
  for (auto &p : prototypes_)
    JS_FreeValue(nativeContext(), p.second);
  prototypes_.clear();
  callbacks_.clear();
}
QJSValue QJSEngine::evaluate(const QString &s, const QString &filename, int) {
  PortableScriptRuntime::Guard l(*runtime_);
  auto source = s.toUtf8(),
       file = filename.isEmpty() ? QByteArray("<script>") : filename.toUtf8();
  return adopt(JS_Eval(nativeContext(), source.data(), source.size(),
                       file.c_str(), JS_EVAL_TYPE_GLOBAL));
}
QJSValue QJSEngine::importModule(const QString &path) {
  PortableScriptRuntime::Guard l(*runtime_);
  auto name = QDir::cleanPath(QFileInfo(path).absoluteFilePath());
  auto previous = modules_.find(name);
  if (previous != modules_.end())
    return previous->second;
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly))
    return newErrorObject(QJSValue::ReferenceError,
                          "Cannot open module " + path);
  auto src = f.readAll(), file = name.toUtf8();
  auto module = JS_Eval(nativeContext(), src.data(), src.size(), file.c_str(),
                        JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
  if (JS_IsException(module))
    return adopt(module);
  auto definition = static_cast<JSModuleDef *>(JS_VALUE_GET_PTR(module));
  auto retained = adopt(JS_DupValue(nativeContext(), module));
  auto result = JS_EvalFunction(nativeContext(), module);
  if (JS_IsException(result))
    return adopt(result);
  auto promise = adopt(result);
  JSContext *jobContext = nullptr;
  while (JS_IsJobPending(runtime_->rt)) {
    if (JS_ExecutePendingJob(runtime_->rt, &jobContext) < 0)
      return adopt(JS_GetException(jobContext));
  }
  if (JS_IsPromise(promise.value_)) {
    auto state = JS_PromiseState(nativeContext(), promise.value_);
    if (state == JS_PROMISE_REJECTED)
      return adopt(JS_PromiseResult(nativeContext(), promise.value_));
    if (state == JS_PROMISE_PENDING)
      return newErrorObject(
          QJSValue::GenericError,
          "Module awaits an unavailable asynchronous operation");
  }
  auto names = adopt(JS_GetModuleNamespace(nativeContext(), definition));
  modules_.emplace(name, names);
  return names;
}
void QJSEngine::installExtensions(int) {
  PortableScriptRuntime::Guard guard(*runtime_);
  auto console = newObject();
  auto log = function("log", 0, [](const QJSValue &, const QJSValueList &a) {
    std::ostringstream message;
    for (auto &v : a)
      message << v.toString() << ' ';
    portableWriteLog(message.str());
    return QJSValue();
  });
  for (auto name : {"log", "info", "warn", "error", "debug"})
    console.setProperty(name, adopt(JS_DupValue(nativeContext(), log)));
  JS_FreeValue(nativeContext(), log);
  globalObject().setProperty("console", console);
}
QJSValue QJSEngine::newErrorObject(QJSValue::ErrorType, const QString &s) {
  PortableScriptRuntime::Guard guard(*runtime_);
  auto e = adopt(JS_NewError(nativeContext()));
  e.setProperty("message", s);
  return e;
}
QJSValue QJSEngine::newQObject(QObject *o) {
  PortableScriptRuntime::Guard l(*runtime_);
  if (!o)
    return QJSValue(QJSValue::NullValue);
  auto old = objects_.find(o);
  if (old != objects_.end()) {
    auto h = static_cast<PortableObjectHandle *>(
        JS_GetOpaque(old->second, runtime_->objectClass));
    auto life = h->alive.lock();
    if (life && *life)
      return adopt(JS_DupValue(nativeContext(), old->second));
    h->engine = nullptr;
    objects_.erase(old);
  }
  auto v = JS_NewObjectClass(nativeContext(), runtime_->objectClass);
  if (JS_IsException(v))
    return adopt(v);
  auto h = new PortableObjectHandle{o, o->portableLifetime(), false, this};
  JS_SetOpaque(v, h);
  objects_[o] = v; // Non-owning: the finalizer removes this identity entry.
  auto wrapped = adopt(v);
  const auto meta = o->portableMetaObject();
  if (!meta)
    return wrapped;
  auto existing = prototypes_.find(meta);
  if (existing != prototypes_.end()) {
    JS_SetPrototype(nativeContext(), v, existing->second);
    return wrapped;
  }
  auto prototype = JS_NewObject(nativeContext());
  prototypes_[meta] = prototype;
  std::function<void(const QMetaObject *)> bind = [&](auto meta) {
    if (!meta)
      return;
    bind(meta->base);
    std::map<QString, QList<PortableMethod>> groups;
    for (auto &m : meta->methods)
      groups[m.name] << m;
    for (auto &group : groups) {
      auto methods = group.second;
      auto fn = function(
          group.first, methods.first().minimum,
          [this, methods](const QJSValue &self, const QJSValueList &args) {
            auto object = self.toQObject();
            if (!object)
              throw std::runtime_error("Native object has been destroyed");
            for (auto &m : methods)
              if (args.size() >= m.minimum && args.size() <= m.maximum &&
                  (!m.accepts || m.accepts(args)))
                return m.invoke(*this, object, args);
            for (auto &m : methods)
              if (args.size() >= m.minimum && args.size() <= m.maximum)
                return m.invoke(*this, object, args);
            throw std::runtime_error("Wrong number of arguments");
          });
      JS_DefinePropertyValueStr(nativeContext(), prototype,
                                group.first.toUtf8().constData(), fn,
                                JS_PROP_CONFIGURABLE | JS_PROP_WRITABLE);
    }
    for (auto &p : meta->properties) {
      auto get =
          p.get
              ? function(p.name, 0,
                         [this, p](const QJSValue &self, const QJSValueList &) {
                           auto o = self.toQObject();
                           if (!o)
                             throw std::runtime_error(
                                 "Native object has been destroyed");
                           return p.get(*this, o);
                         })
              : JS_UNDEFINED;
      auto set = p.set ? function(p.name, 1,
                                  [this, p](const QJSValue &self,
                                            const QJSValueList &a) {
                                    auto o = self.toQObject();
                                    if (!o)
                                      throw std::runtime_error(
                                          "Native object has been destroyed");
                                    p.set(*this, o, a.value(0));
                                    return QJSValue();
                                  })
                       : JS_UNDEFINED;
      auto atom = JS_NewAtom(nativeContext(), p.name.toUtf8().constData());
      JS_DefinePropertyGetSet(nativeContext(), prototype, atom, get, set,
                              JS_PROP_ENUMERABLE | JS_PROP_CONFIGURABLE);
      JS_FreeAtom(nativeContext(), atom);
    }
    for (auto &item : meta->enums)
      JS_SetPropertyStr(nativeContext(), prototype,
                        item.first.toUtf8().constData(),
                        JS_NewInt32(nativeContext(), item.second));
  };
  bind(meta);
  JS_SetPrototype(nativeContext(), v, prototype);
  return wrapped;
}
QJSValue QJSEngine::newQMetaObject(const QMetaObject *m) {
  PortableScriptRuntime::Guard guard(*runtime_);
  auto f =
      function(m->name, 0, [this, m](const QJSValue &, const QJSValueList &a) {
        if (!m->construct)
          throw std::runtime_error("This class is not script-constructible");
        auto o = m->construct(*this, a);
        auto v = newQObject(o);
        auto h = static_cast<PortableObjectHandle *>(
            JS_GetOpaque(v.value_, runtime_->objectClass));
        h->scriptOwned = true;
        return v;
      });
  JS_SetConstructorBit(nativeContext(), f, true);
  auto v = adopt(f);
  for (auto &p : m->enums)
    v.setProperty(p.first, p.second);
  return v;
}
