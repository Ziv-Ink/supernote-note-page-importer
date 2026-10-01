// Supernote conversion plan SHA-256: d4ccb5337c1a6101964c1d1f9b3d93cb10685659838f40157c269e1ae64beccc
#include <supernote/conversion.hpp>
#include <jni.h>
#include <jsi/jsi.h>

#include <android/log.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <functional>
#include <list>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "runtime_services.hpp"
#include "internal.hpp"

namespace supernote::generated::jvm_feature_7e680f69f87879eb {
namespace {

constexpr char kLogTag[] = "SupernoteModuleJvm";
constexpr char kFeatureRegistryGlobal[] =
    "__supernoteModuleFeatureRegistry_63f6999c8c67";
constexpr char kFeatureId[] = "supernote:feature:7e680f69f87879eb";
constexpr char kJvmObjectRegistryProperty[] =
    "__supernoteModuleJvmObjectRegistry_2cfbc9ce6375";

std::string supernote_describe_value(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value) {
  if (value.isUndefined()) return "undefined";
  if (value.isNull()) return "null";
  if (value.isBool()) return "boolean";
  if (value.isNumber()) return "number";
  if (value.isBigInt()) return "bigint";
  if (value.isString()) return "string";
  if (value.isSymbol()) return "symbol";
  if (!value.isObject()) return "unknown";
  auto object = value.getObject(runtime);
  if (object.isArray(runtime)) return "Array";
  if (object.isFunction(runtime)) return "function";
  return "object";
}

facebook::jsi::Value supernote_make_builtin_error(
    facebook::jsi::Runtime &runtime,
    const char *constructor_name,
    const std::string &message,
    const std::string &reason,
    const std::string &path,
    const std::string &expected,
    const std::string &actual) {
  auto constructor =
      runtime.global().getPropertyAsFunction(runtime, constructor_name);
  const facebook::jsi::Value argument(
      facebook::jsi::String::createFromUtf8(runtime, message));
  auto error_value = constructor.callAsConstructor(
      runtime, &argument, static_cast<std::size_t>(1));
  auto error = error_value.getObject(runtime);
  error.setProperty(
      runtime, "reason",
      facebook::jsi::String::createFromAscii(runtime, reason));
  error.setProperty(
      runtime, "path",
      facebook::jsi::String::createFromUtf8(runtime, path));
  error.setProperty(
      runtime, "expected",
      facebook::jsi::String::createFromUtf8(runtime, expected));
  error.setProperty(
      runtime, "actual",
      facebook::jsi::String::createFromUtf8(runtime, actual));
  return facebook::jsi::Value(std::move(error));
}

[[noreturn]] void supernote_throw_builtin_error(
    facebook::jsi::Runtime &runtime,
    const char *constructor_name,
    const std::string &message,
    const std::string &reason = "TYPE_MISMATCH",
    const std::string &path = "",
    const std::string &expected = "",
    const std::string &actual = "unknown") {
  auto error = supernote_make_builtin_error(
      runtime, constructor_name, message, reason, path, expected, actual);
  throw facebook::jsi::JSError(runtime, std::move(error));
}

[[noreturn]] void supernote_throw_type_error(
    facebook::jsi::Runtime &runtime,
    const std::string &message,
    const std::string &reason = "TYPE_MISMATCH",
    const std::string &path = "",
    const std::string &expected = "",
    const std::string &actual = "unknown") {
  supernote_throw_builtin_error(
      runtime, "TypeError", message, reason, path, expected, actual);
}

[[noreturn]] void supernote_throw_range_error(
    facebook::jsi::Runtime &runtime,
    const std::string &message,
    const std::string &reason = "OUT_OF_RANGE",
    const std::string &path = "",
    const std::string &expected = "",
    const std::string &actual = "unknown") {
  supernote_throw_builtin_error(
      runtime, "RangeError", message, reason, path, expected, actual);
}

facebook::jsi::Value supernote_validation_success(
    facebook::jsi::Runtime &runtime) {
  facebook::jsi::Object result(runtime);
  result.setProperty(runtime, "ok", true);
  return facebook::jsi::Value(std::move(result));
}

facebook::jsi::Value supernote_validation_failure(
    facebook::jsi::Runtime &runtime,
    facebook::jsi::Value error) {
  facebook::jsi::Object result(runtime);
  result.setProperty(runtime, "ok", false);
  result.setProperty(runtime, "error", std::move(error));
  return facebook::jsi::Value(std::move(result));
}

facebook::jsi::Function supernote_attach_preflight(
    facebook::jsi::Runtime &runtime,
    facebook::jsi::Function function,
    facebook::jsi::Function accepts,
    facebook::jsi::Function check_arguments) {
  function.setProperty(runtime, "accepts", std::move(accepts));
  function.setProperty(
      runtime, "checkArguments", std::move(check_arguments));
  return function;
}

[[noreturn]] void supernote_throw_error(
    facebook::jsi::Runtime &runtime,
    const char *code,
    const std::string &message) {
  auto registry = runtime.global().getPropertyAsObject(
      runtime, kFeatureRegistryGlobal);
  auto exports = registry.getPropertyAsObject(runtime, kFeatureId);
  auto constructor = exports.getPropertyAsFunction(
      runtime, "__supernoteErrorConstructor");
  const facebook::jsi::Value arguments[] = {
      facebook::jsi::Value(
          facebook::jsi::String::createFromAscii(runtime, code)),
      facebook::jsi::Value(
          facebook::jsi::String::createFromUtf8(runtime, message)),
  };
  auto error = constructor.callAsConstructor(
      runtime, arguments, static_cast<std::size_t>(2));
  throw facebook::jsi::JSError(runtime, std::move(error));
}

facebook::jsi::Function supernote_uint8_array_constructor(
    facebook::jsi::Runtime &runtime) {
  return runtime.global().getPropertyAsFunction(runtime, "Uint8Array");
}

bool supernote_is_uint8_array(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value) {
  if (!value.isObject()) {
    return false;
  }
  auto constructor = supernote_uint8_array_constructor(runtime);
  return value.getObject(runtime).instanceOf(runtime, constructor);
}

bool supernote_array_has_own_index(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Array &array,
    std::size_t index) {
  auto object_constructor =
      runtime.global().getPropertyAsObject(runtime, "Object");
  auto prototype =
      object_constructor.getPropertyAsObject(runtime, "prototype");
  auto has_own =
      prototype.getPropertyAsFunction(runtime, "hasOwnProperty");
  auto key = facebook::jsi::String::createFromUtf8(
      runtime, std::to_string(index));
  auto result = has_own.callWithThis(runtime, array, std::move(key));
  return result.isBool() && result.getBool();
}

bool supernote_object_has_own_property(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Object &object,
    const char *property) {
  auto object_constructor =
      runtime.global().getPropertyAsObject(runtime, "Object");
  auto prototype =
      object_constructor.getPropertyAsObject(runtime, "prototype");
  auto has_own =
      prototype.getPropertyAsFunction(runtime, "hasOwnProperty");
  auto key = facebook::jsi::String::createFromUtf8(runtime, property);
  auto result = has_own.callWithThis(runtime, object, std::move(key));
  return result.isBool() && result.getBool();
}

std::size_t supernote_view_index(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Object &view,
    const char *property) {
  auto value = view.getProperty(runtime, property);
  if (!value.isNumber()) {
    throw facebook::jsi::JSError(
        runtime, std::string("Uint8Array.") + property + " is not numeric");
  }
  const double number = value.asNumber();
  if (!std::isfinite(number) || std::trunc(number) != number || number < 0 ||
      number > static_cast<double>(std::numeric_limits<std::size_t>::max())) {
    throw facebook::jsi::JSError(
        runtime, std::string("Uint8Array.") + property + " is invalid");
  }
  return static_cast<std::size_t>(number);
}

struct SupernoteUint8ArraySnapshot {
  facebook::jsi::ArrayBuffer buffer;
  std::size_t offset;
  std::size_t length;
};

SupernoteUint8ArraySnapshot supernote_snapshot_uint8_array(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value) {
  auto view = value.getObject(runtime);
  const std::size_t offset =
      supernote_view_index(runtime, view, "byteOffset");
  const std::size_t length =
      supernote_view_index(runtime, view, "byteLength");
  auto buffer_object = view.getPropertyAsObject(runtime, "buffer");
  if (!buffer_object.isArrayBuffer(runtime)) {
    throw facebook::jsi::JSError(
        runtime, "Uint8Array.buffer is not an ArrayBuffer");
  }
  auto buffer = buffer_object.getArrayBuffer(runtime);
  const std::size_t buffer_size = buffer.size(runtime);
  if (offset > buffer_size || length > buffer_size - offset) {
    throw facebook::jsi::JSError(
        runtime, "Uint8Array view exceeds its ArrayBuffer");
  }
  return {std::move(buffer), offset, length};
}

void supernote_check_uint8_array_snapshot_limit(
    facebook::jsi::Runtime &runtime,
    const SupernoteUint8ArraySnapshot &snapshot,
    const std::string &path) {
  constexpr std::size_t kMaxByteBufferBytes = 32ULL * 1024ULL * 1024ULL;
  if (snapshot.length > kMaxByteBufferBytes) {
    supernote_throw_range_error(
        runtime,
        "Uint8Array byteLength exceeds the generated conversion limit",
        "LIMIT_EXCEEDED",
        path,
        "at most 33554432 bytes",
        std::to_string(snapshot.length) + " bytes");
  }
}

std::vector<std::byte> supernote_copy_uint8_array(
    facebook::jsi::Runtime &runtime,
    SupernoteUint8ArraySnapshot &snapshot) {
  supernote_check_uint8_array_snapshot_limit(
      runtime, snapshot, "Uint8Array.byteLength");
  std::vector<std::byte> result(snapshot.length);
  if (snapshot.length != 0) {
    std::memcpy(
        result.data(),
        snapshot.buffer.data(runtime) + snapshot.offset,
        snapshot.length);
  }
  return result;
}

std::vector<std::byte> supernote_copy_uint8_array(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value) {
  auto snapshot = supernote_snapshot_uint8_array(runtime, value);
  return supernote_copy_uint8_array(runtime, snapshot);
}

class SupernoteOwnedBytesBuffer final : public facebook::jsi::MutableBuffer {
 public:
  explicit SupernoteOwnedBytesBuffer(const std::vector<std::byte> &value)
      : bytes_(value.size()) {
    if (!value.empty()) {
      std::memcpy(bytes_.data(), value.data(), value.size());
    }
  }

  std::size_t size() const override {
    return bytes_.size();
  }

  std::uint8_t *data() override {
    return bytes_.data();
  }

 private:
  std::vector<std::uint8_t> bytes_;
};

facebook::jsi::Value supernote_make_uint8_array(
    facebook::jsi::Runtime &runtime,
    const std::vector<std::byte> &value) {
  auto storage = std::make_shared<SupernoteOwnedBytesBuffer>(value);
  const facebook::jsi::Value argument(
      facebook::jsi::ArrayBuffer(runtime, std::move(storage)));
  auto constructor = supernote_uint8_array_constructor(runtime);
  return constructor.callAsConstructor(
      runtime, &argument, static_cast<std::size_t>(1));
}

constexpr char kPromiseContinuationsGlobal[] =
    "__supernoteModulePromiseContinuations_a7db36cf3b5e";

facebook::jsi::Object supernote_error_object(
    facebook::jsi::Runtime &runtime,
    const char *code,
    const std::string &message) {
  auto registry = runtime.global().getPropertyAsObject(
      runtime, kFeatureRegistryGlobal);
  auto exports = registry.getPropertyAsObject(runtime, kFeatureId);
  auto constructor = exports.getPropertyAsFunction(
      runtime, "__supernoteErrorConstructor");
  const facebook::jsi::Value arguments[] = {
      facebook::jsi::String::createFromAscii(runtime, code),
      facebook::jsi::String::createFromUtf8(runtime, message),
  };
  auto error = constructor.callAsConstructor(
      runtime, arguments, static_cast<std::size_t>(2));
  return error.getObject(runtime);
}

facebook::jsi::Object supernote_promise_continuations(
    facebook::jsi::Runtime &runtime) {
  auto value = runtime.global().getProperty(
      runtime, kPromiseContinuationsGlobal);
  if (value.isObject()) return value.getObject(runtime);
  auto map = runtime.global().getPropertyAsFunction(runtime, "Map");
  auto continuations = map.callAsConstructor(runtime).getObject(runtime);
  runtime.global().setProperty(
      runtime, kPromiseContinuationsGlobal, continuations);
  return continuations;
}

void supernote_register_continuation(
    facebook::jsi::Runtime &runtime,
    std::uint64_t operation_id,
    const facebook::jsi::Value &resolve,
    const facebook::jsi::Value &reject) {
  facebook::jsi::Object continuation(runtime);
  continuation.setProperty(runtime, "resolve", resolve);
  continuation.setProperty(runtime, "reject", reject);
  auto continuations = supernote_promise_continuations(runtime);
  const auto key = std::to_string(operation_id);
  auto set = continuations.getPropertyAsFunction(runtime, "set");
  set.callWithThis(
      runtime, continuations,
      facebook::jsi::String::createFromAscii(runtime, key),
      std::move(continuation));
}

facebook::jsi::Object supernote_take_continuation(
    facebook::jsi::Runtime &runtime,
    std::uint64_t operation_id) {
  auto continuations = supernote_promise_continuations(runtime);
  const auto key = std::to_string(operation_id);
  auto key_value = facebook::jsi::String::createFromAscii(runtime, key);
  auto get = continuations.getPropertyAsFunction(runtime, "get");
  auto value = get.callWithThis(runtime, continuations, key_value);
  if (!value.isObject()) {
    throw facebook::jsi::JSError(
        runtime, "Supernote async continuation is unavailable");
  }
  auto continuation = value.getObject(runtime);
  auto remove = continuations.getPropertyAsFunction(runtime, "delete");
  auto removed = remove.callWithThis(runtime, continuations, key_value);
  if (!removed.isBool() || !removed.getBool()) {
    throw facebook::jsi::JSError(
        runtime, "Supernote async continuation cannot be removed");
  }
  return continuation;
}

void supernote_resolve_operation(
    facebook::jsi::Runtime &runtime,
    std::uint64_t operation_id,
    facebook::jsi::Value value) {
  auto continuation = supernote_take_continuation(runtime, operation_id);
  auto resolve = continuation.getPropertyAsFunction(runtime, "resolve");
  resolve.call(runtime, std::move(value));
}

void supernote_reject_operation(
    facebook::jsi::Runtime &runtime,
    std::uint64_t operation_id,
    const char *code,
    const std::string &message) {
  auto continuation = supernote_take_continuation(runtime, operation_id);
  auto reject = continuation.getPropertyAsFunction(runtime, "reject");
  reject.call(runtime, supernote_error_object(runtime, code, message));
}

void supernote_reject_new_promise(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &reject_value,
    const char *code,
    const std::string &message) {
  auto reject = reject_value.getObject(runtime).asFunction(runtime);
  reject.call(runtime, supernote_error_object(runtime, code, message));
}

class JvmImplementationFailure final : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

template <typename Callback, typename Result>
void deliver_internal_callback(Callback &callback, Result result) noexcept {
  try {
    callback(std::move(result));
  } catch (const std::exception &error) {
    __android_log_print(
        ANDROID_LOG_ERROR, kLogTag,
        "internal JVM completion callback threw: %s", error.what());
  } catch (...) {
    __android_log_print(
        ANDROID_LOG_ERROR, kLogTag,
        "internal JVM completion callback threw an unknown exception");
  }
}

class AttachedEnv {
 public:
  AttachedEnv() {
    vm_ = static_cast<JavaVM *>(
        supernote::runtime::process_services().java_vm());
    if (vm_ == nullptr) return;
    const auto status = vm_->GetEnv(
        reinterpret_cast<void **>(&env_), JNI_VERSION_1_6);
    if (status == JNI_EDETACHED && attach_current_thread() == JNI_OK) {
      attached_ = true;
    }
  }
  ~AttachedEnv() { if (attached_) vm_->DetachCurrentThread(); }
  JNIEnv *get() const noexcept { return env_; }

 private:
  jint attach_current_thread() noexcept {
#if defined(__ANDROID__)
    return vm_->AttachCurrentThread(&env_, nullptr);
#else
    return vm_->AttachCurrentThread(
        reinterpret_cast<void **>(&env_), nullptr);
#endif
  }
  JavaVM *vm_{nullptr};
  JNIEnv *env_{nullptr};
  bool attached_{false};
};

class LocalFrame {
 public:
  explicit LocalFrame(JNIEnv *env) : env_(env) {
    if (env_ == nullptr || env_->PushLocalFrame(32) != JNI_OK) {
      throw std::runtime_error("cannot create JNI local-reference frame");
    }
  }
  ~LocalFrame() { if (env_ != nullptr) env_->PopLocalFrame(nullptr); }

 private:
  JNIEnv *env_;
};

class LocalReference {
 public:
  LocalReference(JNIEnv *env, jobject value) : env_(env), value_(value) {}
  LocalReference(const LocalReference &) = delete;
  LocalReference &operator=(const LocalReference &) = delete;
  ~LocalReference() {
    if (env_ != nullptr && value_ != nullptr) env_->DeleteLocalRef(value_);
  }
  jobject get() const noexcept { return value_; }

 private:
  JNIEnv *env_;
  jobject value_;
};

void clear_exception(JNIEnv *env) {
  if (env != nullptr && env->ExceptionCheck()) env->ExceptionClear();
}

std::string implementation_exception_message(
    JNIEnv *env, jthrowable failure) {
  if (env == nullptr || failure == nullptr) return {};
  LocalReference failure_class(env, env->GetObjectClass(failure));
  if (env->ExceptionCheck() || failure_class.get() == nullptr) {
    clear_exception(env);
    return {};
  }
  auto get_message = env->GetMethodID(
      static_cast<jclass>(failure_class.get()),
      "getMessage", "()Ljava/lang/String;");
  if (env->ExceptionCheck() || get_message == nullptr) {
    clear_exception(env);
    return {};
  }
  LocalReference message(
      env, env->CallObjectMethod(failure, get_message));
  if (env->ExceptionCheck() || message.get() == nullptr) {
    clear_exception(env);
    return {};
  }
  const char *utf8 = env->GetStringUTFChars(
      static_cast<jstring>(message.get()), nullptr);
  if (env->ExceptionCheck() || utf8 == nullptr) {
    clear_exception(env);
    return {};
  }
  std::string result;
  try {
    result.assign(utf8);
  } catch (...) {
    env->ReleaseStringUTFChars(
        static_cast<jstring>(message.get()), utf8);
    clear_exception(env);
    throw;
  }
  env->ReleaseStringUTFChars(
      static_cast<jstring>(message.get()), utf8);
  if (env->ExceptionCheck()) clear_exception(env);
  return result;
}

std::shared_ptr<void> retain_global(JNIEnv *env, jobject value) {
  if (env == nullptr || value == nullptr) {
    throw std::runtime_error("cannot retain a null JVM object");
  }
  auto *global = env->NewGlobalRef(value);
  if (global == nullptr) {
    clear_exception(env);
    throw std::runtime_error("cannot allocate a JNI global reference");
  }
  auto cleanup = supernote::runtime::process_services().cleanup();
  return std::shared_ptr<void>(global, [cleanup](void *raw) {
    auto release = [raw] {
      AttachedEnv attached;
      if (auto *env = attached.get()) {
        env->DeleteGlobalRef(static_cast<jobject>(raw));
      }
    };
    if (!cleanup || !cleanup->submit(release)) release();
  });
}


std::shared_ptr<void> retain_weak_global(JNIEnv *env, jobject value) {
  if (env == nullptr || value == nullptr) {
    throw std::runtime_error("cannot weakly retain a null JVM object");
  }
  auto weak = env->NewWeakGlobalRef(value);
  if (weak == nullptr) {
    clear_exception(env);
    throw std::runtime_error("cannot allocate a JNI weak global reference");
  }
  auto cleanup = supernote::runtime::process_services().cleanup();
  return std::shared_ptr<void>(weak, [cleanup](void *raw) {
    auto release = [raw] {
      AttachedEnv attached;
      if (auto *env = attached.get()) {
        env->DeleteWeakGlobalRef(static_cast<jweak>(raw));
      }
    };
    if (!cleanup || !cleanup->submit(release)) release();
  });
}

class ManagedJvmRef final {
 public:
  ManagedJvmRef() = default;
  ManagedJvmRef(std::string type_id, std::shared_ptr<void> global)
      : type_id_(std::move(type_id)), global_(std::move(global)) {
    if (type_id_.empty() || !global_) {
      throw std::invalid_argument(
          "a JVM managed reference requires nominal identity and a global reference");
    }
  }

  explicit operator bool() const noexcept { return static_cast<bool>(global_); }
  std::string_view type_id() const noexcept { return type_id_; }
  jobject get() const noexcept { return static_cast<jobject>(global_.get()); }
  const std::shared_ptr<void> &global_ref() const noexcept { return global_; }

 private:
  std::string type_id_;
  std::shared_ptr<void> global_;
};

class ManagedJvmValue final {
 public:
  ManagedJvmValue() = default;
  explicit ManagedJvmValue(std::shared_ptr<void> global)
      : global_(std::move(global)) {}

  explicit operator bool() const noexcept { return static_cast<bool>(global_); }
  jobject get() const noexcept { return static_cast<jobject>(global_.get()); }
  const std::shared_ptr<void> &global_ref() const noexcept { return global_; }

 private:
  std::shared_ptr<void> global_;
};

class JvmObjectHandleBase : public facebook::jsi::HostObject {
 public:
  ~JvmObjectHandleBase() override = default;
  virtual std::string_view type_id() const noexcept = 0;
  virtual ManagedJvmRef managed_ref() const = 0;
};

class JvmObjectRegistry final
    : public std::enable_shared_from_this<JvmObjectRegistry> {
 public:
  JvmObjectRegistry() = default;
  JvmObjectRegistry(const JvmObjectRegistry &) = delete;
  JvmObjectRegistry &operator=(const JvmObjectRegistry &) = delete;

  template <typename Factory>
  facebook::jsi::Object wrap(
      facebook::jsi::Runtime &runtime,
      JNIEnv *env,
      std::string_view type_id,
      jobject instance,
      jint identity_hash,
      std::shared_ptr<void> strong_global,
      Factory &&factory) {
    assert_runtime(runtime);
    if (env == nullptr || instance == nullptr || !strong_global || type_id.empty()) {
      throw std::invalid_argument(
          "a JVM object result requires an environment, type, and live instance");
    }
    const auto hash = identity_hash;
    for (auto current = entries_.begin(); current != entries_.end();) {
      const auto weak = static_cast<jweak>(current->weak_global.get());
      const bool native_dead =
          weak == nullptr || env->IsSameObject(weak, nullptr) == JNI_TRUE;
      if (env->ExceptionCheck()) {
        clear_exception(env);
        throw std::runtime_error("cannot inspect JVM weak object identity");
      }
      if (native_dead) {
        current = entries_.erase(current);
        continue;
      }
      if (current->identity_hash != hash || current->type_id != type_id ||
          env->IsSameObject(weak, instance) != JNI_TRUE) {
        if (env->ExceptionCheck()) {
          clear_exception(env);
          throw std::runtime_error("cannot compare JVM object identity");
        }
        ++current;
        continue;
      }
      auto locked = current->javascript.lock(runtime);
      if (locked.isObject()) return locked.getObject(runtime);
      current = entries_.erase(current);
      break;
    }

    auto weak_global = retain_weak_global(env, instance);
    ManagedJvmRef managed(std::string(type_id), std::move(strong_global));
    auto host = std::invoke(
        std::forward<Factory>(factory), std::move(managed));
    static_assert(
        std::is_convertible_v<decltype(host),
                              std::shared_ptr<facebook::jsi::HostObject>>);
    auto object = facebook::jsi::Object::createFromHostObject(
        runtime, std::move(host));
    entries_.emplace_back(
        std::string(type_id), hash, std::move(weak_global),
        facebook::jsi::WeakObject(runtime, object));
    return object;
  }

  void purge(facebook::jsi::Runtime &runtime, JNIEnv *env) {
    assert_runtime(runtime);
    if (env == nullptr) throw std::invalid_argument("JNIEnv is required");
    for (auto current = entries_.begin(); current != entries_.end();) {
      const auto weak = static_cast<jweak>(current->weak_global.get());
      const bool native_dead =
          weak == nullptr || env->IsSameObject(weak, nullptr) == JNI_TRUE;
      if (env->ExceptionCheck()) {
        clear_exception(env);
        throw std::runtime_error("cannot inspect JVM weak object identity");
      }
      if (native_dead || !current->javascript.lock(runtime).isObject()) {
        current = entries_.erase(current);
      } else {
        ++current;
      }
    }
  }

  std::size_t size_for_testing() const noexcept { return entries_.size(); }

 private:
  struct Entry final {
    Entry(
        std::string type_id,
        jint identity_hash,
        std::shared_ptr<void> weak_global,
        facebook::jsi::WeakObject javascript)
        : type_id(std::move(type_id)),
          identity_hash(identity_hash),
          weak_global(std::move(weak_global)),
          javascript(std::move(javascript)) {}
    std::string type_id;
    jint identity_hash;
    std::shared_ptr<void> weak_global;
    facebook::jsi::WeakObject javascript;
  };

  void assert_runtime(facebook::jsi::Runtime &runtime) {
    if (runtime_ == nullptr) {
      runtime_ = &runtime;
    } else if (runtime_ != &runtime) {
      throw std::logic_error(
          "a JVM object registry cannot cross JavaScript runtimes");
    }
  }

  facebook::jsi::Runtime *runtime_ = nullptr;
  std::list<Entry> entries_;
};

class JvmObjectRegistryOwner final : public facebook::jsi::HostObject {
 public:
  explicit JvmObjectRegistryOwner(std::shared_ptr<JvmObjectRegistry> registry)
      : registry_(std::move(registry)) {
    if (!registry_) throw std::invalid_argument("JVM object registry is required");
  }

  const std::shared_ptr<JvmObjectRegistry> &registry() const noexcept {
    return registry_;
  }

 private:
  std::shared_ptr<JvmObjectRegistry> registry_;
};

std::string jvm_object_type_id(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value) {
  if (!value.isObject()) return {};
  auto object = value.getObject(runtime);
  if (!object.isHostObject<JvmObjectHandleBase>(runtime)) return {};
  return std::string(
      object.getHostObject<JvmObjectHandleBase>(runtime)->type_id());
}

ManagedJvmRef try_extract_jvm_object(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    std::string_view expected_type_id) {
  if (!value.isObject()) return {};
  auto object = value.getObject(runtime);
  if (!object.isHostObject<JvmObjectHandleBase>(runtime)) return {};
  auto handle = object.getHostObject<JvmObjectHandleBase>(runtime);
  if (handle->type_id() != expected_type_id) return {};
  return handle->managed_ref();
}


struct JvmRoute {
  std::shared_ptr<void> adapter_class;
  jmethodID method{nullptr};
};

class LazyJvmRoute {
 public:
  LazyJvmRoute(std::string adapter_class, std::string descriptor,
               std::string method_name = "invoke")
      : adapter_class_(std::move(adapter_class)),
        descriptor_(std::move(descriptor)),
        method_name_(std::move(method_name)) {}

  std::shared_ptr<JvmRoute> get(
      const std::shared_ptr<supernote::runtime::FeatureSession> &feature) {
    std::lock_guard lock(mutex_);
    if (route_) return route_;
    auto runtime = feature ? feature->runtime() : nullptr;
    if (!runtime || !runtime->active()) {
      throw std::runtime_error("feature runtime is closed");
    }
    auto loader = runtime->plugin_class_loader();
    if (!loader) throw std::runtime_error("plugin ClassLoader is unavailable");
    AttachedEnv attached;
    auto *env = attached.get();
    if (env == nullptr) throw std::runtime_error("cannot attach to JavaVM");
    LocalFrame frame(env);
    auto loader_class = env->GetObjectClass(static_cast<jobject>(loader.get()));
    auto load_class = loader_class == nullptr
        ? nullptr
        : env->GetMethodID(
              loader_class, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    auto class_name = env->NewStringUTF(adapter_class_.c_str());
    auto local_class = load_class == nullptr || class_name == nullptr
        ? nullptr
        : env->CallObjectMethod(
              static_cast<jobject>(loader.get()), load_class, class_name);
    if (env->ExceptionCheck() || local_class == nullptr) {
      clear_exception(env);
      throw std::runtime_error("cannot resolve generated JVM adapter class");
    }
    auto method = env->GetStaticMethodID(
        static_cast<jclass>(local_class), method_name_.c_str(),
        descriptor_.c_str());
    if (env->ExceptionCheck() || method == nullptr) {
      clear_exception(env);
      throw std::runtime_error("cannot resolve generated JVM adapter method");
    }
    route_ = std::make_shared<JvmRoute>(JvmRoute{
        retain_global(env, local_class), method});
    return route_;
  }

 private:
  std::mutex mutex_;
  std::string adapter_class_;
  std::string descriptor_;
  std::string method_name_;
  std::shared_ptr<JvmRoute> route_;
};

struct JvmOwner {
  explicit JvmOwner(std::shared_ptr<void> value) : value(std::move(value)) {}
  std::shared_ptr<void> value;
};

std::vector<std::byte> read_byte_array(JNIEnv *env, jbyteArray value) {
  if (value == nullptr) throw std::runtime_error("JVM adapter returned null");
  const auto length = env->GetArrayLength(value);
  if (env->ExceptionCheck() || length < 0) {
    clear_exception(env);
    throw std::runtime_error("cannot read JVM byte-array length");
  }
  std::vector<std::byte> result(static_cast<std::size_t>(length));
  if (length != 0) {
    env->GetByteArrayRegion(
        value, 0, length, reinterpret_cast<jbyte *>(result.data()));
    if (env->ExceptionCheck()) {
      clear_exception(env);
      throw std::runtime_error("cannot copy JVM byte-array result");
    }
  }
  return result;
}

jbyteArray write_byte_array(
    JNIEnv *env, const std::byte *data, std::size_t size) {
  if (size > static_cast<std::size_t>(std::numeric_limits<jsize>::max())) {
    throw std::runtime_error("JVM byte-array argument is too large");
  }
  auto result = env->NewByteArray(static_cast<jsize>(size));
  if (result == nullptr) {
    clear_exception(env);
    throw std::runtime_error("cannot allocate JVM byte-array argument");
  }
  if (size != 0) {
    env->SetByteArrayRegion(
        result, 0, static_cast<jsize>(size),
        reinterpret_cast<const jbyte *>(data));
    if (env->ExceptionCheck()) {
      clear_exception(env);
      throw std::runtime_error("cannot copy JVM byte-array argument");
    }
  }
  return result;
}

void require_no_implementation_exception(JNIEnv *env) {
  if (!env->ExceptionCheck()) return;
  LocalReference failure(env, env->ExceptionOccurred());
  env->ExceptionClear();
  auto message = implementation_exception_message(
      env, static_cast<jthrowable>(failure.get()));
  throw JvmImplementationFailure(
      message.empty()
          ? "Kotlin/Java implementation failed"
          : "Kotlin/Java implementation failed: " + message);
}



}  // namespace




void register_jvm_feature(
    facebook::jsi::Runtime &runtime,
    facebook::jsi::Object &feature_registry,
    const std::shared_ptr<supernote::runtime::FeatureSession> &feature_session) {
  using facebook::jsi::Function;
  using facebook::jsi::Object;
  using facebook::jsi::PropNameID;
  using facebook::jsi::String;
  using facebook::jsi::Value;

  auto exports = feature_registry.getPropertyAsObject(runtime, kFeatureId);
  {
    auto route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.adapters.Adapter_c88de5ed49abe06d2574", "(Lcom/ziv/snfiletools/ExportBitmap;[B[BIIJ)Lkotlinx/coroutines/Job;");
    auto cancel_route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.runtime.SupernoteCoroutineBridge",
        "(Lkotlinx/coroutines/Job;)V", "cancel");
    auto owner_route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.adapters.Adapter_72f3cb2919f47e9f0eb9", "(Lcom/facebook/react/bridge/ReactApplicationContext;)Lcom/ziv/snfiletools/ExportBitmap;");
    auto function = Function::createFromHostFunction(
        runtime,
        PropNameID::forAscii(runtime, "encodeExportedBitmap"),
        4,
        [route, cancel_route, feature_session, owner_route](facebook::jsi::Runtime &runtime,
           const Value &,
           const Value *arguments,
           std::size_t argument_count) -> Value {
          if (argument_count != 4) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.encodeExportedBitmap: expected 4 arguments (string pngPath, string rlePath, number width, number height); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.encodeExportedBitmap",
                "4 arguments (string pngPath, string rlePath, number width, number height)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 1 (pngPath) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[0](pngPath)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
          if (!arguments[1].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 2 (rlePath) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[1](rlePath)",
                "string",
                supernote_describe_value(runtime, arguments[1]));
          }
          if (!arguments[2].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 3 (width) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[2](width)",
                "number",
                supernote_describe_value(runtime, arguments[2]));
          }
          const double supernote_argument_2 = arguments[2].asNumber();
          if (!std::isfinite(supernote_argument_2) ||
              std::trunc(supernote_argument_2) != supernote_argument_2 ||
              supernote_argument_2 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_2 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 3 (width) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.encodeExportedBitmap.argument[2](width)", "int32",
                supernote_describe_value(runtime, arguments[2]));
          }
          if (!arguments[3].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 4 (height) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[3](height)",
                "number",
                supernote_describe_value(runtime, arguments[3]));
          }
          const double supernote_argument_3 = arguments[3].asNumber();
          if (!std::isfinite(supernote_argument_3) ||
              std::trunc(supernote_argument_3) != supernote_argument_3 ||
              supernote_argument_3 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_3 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 4 (height) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.encodeExportedBitmap.argument[3](height)", "int32",
                supernote_describe_value(runtime, arguments[3]));
          }
          auto supernote_input_0 = arguments[0].asString(runtime).utf8(runtime);
          auto supernote_input_1 = arguments[1].asString(runtime).utf8(runtime);
          auto supernote_input_2 = static_cast<std::int32_t>(arguments[2].asNumber());
          auto supernote_input_3 = static_cast<std::int32_t>(arguments[3].asNumber());
          struct SuspendState {
            bool success{false};
            std::optional<std::string> value;
            std::string code;
            std::string error;
          };
          auto state = std::make_shared<SuspendState>();
          auto executor = Function::createFromHostFunction(
              runtime,
              PropNameID::forAscii(runtime, "SupernoteSuspendExecutor"),
              2,
              [route, cancel_route, feature_session, owner_route, state, supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2), supernote_input_3 = std::move(supernote_input_3)](
                  facebook::jsi::Runtime &runtime,
                  const Value &,
                  const Value *continuation_arguments,
                  std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = feature_session
                    ? feature_session->accept_factory(
                          [](supernote::runtime::SessionId operation_id) {
                            return [operation_id](void *runtime_pointer) {
                              auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                  runtime_pointer);
                              supernote_reject_operation(
                                  runtime, operation_id, "FEATURE_CLOSED",
                                  "feature closed before async completion");
                            };
                          })
                    : nullptr;
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature =
                    feature_session;
                const auto completion_id =
                    supernote::runtime::process_services()
                        .register_jvm_async_completion(
                            [operation, operation_id, weak_feature, state](
                                void *environment, void *result,
                                std::string error_code,
                                std::string error_message) {
                              if (operation->cancellation_token().is_cancelled()) return;
                              if (!error_code.empty()) {
                                state->code = std::move(error_code);
                                state->error = std::move(error_message);
                              } else {
                                try {
                                  auto *env = static_cast<JNIEnv *>(environment);
                                  auto object = static_cast<jobject>(result);
                                  if (env == nullptr || object == nullptr) {
                                    throw std::runtime_error("Kotlin coroutine returned null");
                                  }
                                  LocalFrame frame(env);
                                  const auto bytes = read_byte_array(env, static_cast<jbyteArray>(object));
                                  state->value = std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size());
                                  state->success = true;
                                } catch (const std::exception &error) {
                                  state->code = "INTERNAL";
                                  state->error = error.what();
                                } catch (...) {
                                  state->code = "INTERNAL";
                                  state->error = "cannot decode Kotlin coroutine result";
                                }
                              }
                              if (operation->cancellation_token().is_cancelled()) return;
                              auto feature = weak_feature.lock();
                              if (!feature) return;
                              feature->schedule_completion(
                                  operation,
                                  [state, operation_id](void *runtime_pointer) {
                                    auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                        runtime_pointer);
                                    if (!state->success) {
                                      supernote_reject_operation(
                                          runtime, operation_id,
                                          state->code.empty()
                                              ? "INTERNAL"
                                              : state->code.c_str(),
                                          state->error.empty()
                                              ? "Kotlin coroutine failed"
                                              : state->error);
                                      return;
                                    }
                                    try {
                auto value = Value(String::createFromUtf8(runtime, *state->value));
                supernote_resolve_operation(
                    runtime, operation_id, std::move(value));
                                    } catch (const std::exception &error) {
                                      supernote_reject_operation(
                                          runtime, operation_id, "INTERNAL", error.what());
                                    }
                                  });
                            });
                operation->set_cancel_hook([completion_id] {
                  supernote::runtime::process_services()
                      .discard_jvm_async_completion(completion_id);
                });
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, weak_feature, route, cancel_route, completion_id, owner_route, supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2), supernote_input_3 = std::move(supernote_input_3)](
                        supernote::runtime::CancellationToken executor_cancel) mutable {
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature ||
                          implementation_feature->state() !=
                              supernote::runtime::FeatureState::ACTIVE) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(
                          implementation_feature);
                      try {
            auto owner = implementation_feature->service<JvmOwner>(
                "jvm:com.ziv.snfiletools.ExportBitmap", [&] {
                  auto constructor = owner_route->get(implementation_feature);
                  auto runtime_session = implementation_feature->runtime();
                  auto context = runtime_session
                      ? runtime_session->platform_context() : nullptr;
                  if (!context) {
                    throw std::runtime_error("platform Context is unavailable");
                  }
                  AttachedEnv attached;
                  auto *env = attached.get();
                  if (env == nullptr) {
                    throw std::runtime_error("cannot attach to JavaVM");
                  }
                  LocalFrame frame(env);
                  jvalue arguments[1]{};
                  arguments[0].l = static_cast<jobject>(context.get());
                  auto local = env->CallStaticObjectMethodA(
                      static_cast<jclass>(constructor->adapter_class.get()),
                      constructor->method, arguments);
                  require_no_implementation_exception(env);
                  if (local == nullptr) {
                    throw std::runtime_error("JVM owner constructor returned null");
                  }
                  return std::make_shared<JvmOwner>(retain_global(env, local));
                });
              auto resolved = route->get(implementation_feature);
                        auto cancel_resolved = cancel_route->get(
                            implementation_feature);
                        AttachedEnv attached;
                        auto *env = attached.get();
                        if (env == nullptr) {
                          throw std::runtime_error("cannot attach to JavaVM");
                        }
                        LocalFrame frame(env);
                        jvalue jvm_arguments[6]{};
              jvm_arguments[0].l = static_cast<jobject>(owner->value.get());
              jvm_arguments[1].l = write_byte_array(env, reinterpret_cast<const std::byte *>(supernote_input_0.data()), supernote_input_0.size());
              jvm_arguments[2].l = write_byte_array(env, reinterpret_cast<const std::byte *>(supernote_input_1.data()), supernote_input_1.size());
              jvm_arguments[3].i = static_cast<jint>(supernote_input_2);
              jvm_arguments[4].i = static_cast<jint>(supernote_input_3);
              jvm_arguments[5].j = static_cast<jlong>(completion_id);
                        auto local_job = env->CallStaticObjectMethodA(
                            static_cast<jclass>(resolved->adapter_class.get()),
                            resolved->method, jvm_arguments);
                        if (env->ExceptionCheck()) {
                          require_no_implementation_exception(env);
                        }
                        if (local_job == nullptr) {
                          throw std::runtime_error(
                              "cannot launch generated Kotlin coroutine adapter");
                        }
                        auto job = retain_global(env, local_job);
                        operation->set_cancel_hook(
                            [completion_id, job, cancel_resolved] {
                              supernote::runtime::process_services()
                                  .discard_jvm_async_completion(completion_id);
                              try {
                                AttachedEnv attached;
                                auto *env = attached.get();
                                if (env == nullptr) return;
                                LocalFrame frame(env);
                                jvalue arguments[1]{};
                                arguments[0].l = static_cast<jobject>(job.get());
                                env->CallStaticVoidMethodA(
                                    static_cast<jclass>(
                                        cancel_resolved->adapter_class.get()),
                                    cancel_resolved->method, arguments);
                                clear_exception(env);
                              } catch (...) {}
                            });
                      } catch (const std::exception &error) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL", error.what());
                      } catch (...) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL",
                            "cannot launch Kotlin coroutine adapter");
                      }
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  supernote::runtime::process_services().complete_jvm_async(
                      completion_id, nullptr, nullptr, "RESOURCE_EXHAUSTED",
                      "Supernote worker queue is full");
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        });
    auto accepts = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "encodeExportedBitmap.accepts"),
        4,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 4) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.encodeExportedBitmap: expected 4 arguments (string pngPath, string rlePath, number width, number height); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.encodeExportedBitmap",
                "4 arguments (string pngPath, string rlePath, number width, number height)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 1 (pngPath) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[0](pngPath)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
          if (!arguments[1].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 2 (rlePath) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[1](rlePath)",
                "string",
                supernote_describe_value(runtime, arguments[1]));
          }
          if (!arguments[2].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 3 (width) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[2](width)",
                "number",
                supernote_describe_value(runtime, arguments[2]));
          }
          const double supernote_argument_2 = arguments[2].asNumber();
          if (!std::isfinite(supernote_argument_2) ||
              std::trunc(supernote_argument_2) != supernote_argument_2 ||
              supernote_argument_2 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_2 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 3 (width) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.encodeExportedBitmap.argument[2](width)", "int32",
                supernote_describe_value(runtime, arguments[2]));
          }
          if (!arguments[3].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 4 (height) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[3](height)",
                "number",
                supernote_describe_value(runtime, arguments[3]));
          }
          const double supernote_argument_3 = arguments[3].asNumber();
          if (!std::isfinite(supernote_argument_3) ||
              std::trunc(supernote_argument_3) != supernote_argument_3 ||
              supernote_argument_3 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_3 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 4 (height) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.encodeExportedBitmap.argument[3](height)", "int32",
                supernote_describe_value(runtime, arguments[3]));
          }
            return facebook::jsi::Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return facebook::jsi::Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    auto check_arguments = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "encodeExportedBitmap.checkArguments"),
        4,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 4) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.encodeExportedBitmap: expected 4 arguments (string pngPath, string rlePath, number width, number height); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.encodeExportedBitmap",
                "4 arguments (string pngPath, string rlePath, number width, number height)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 1 (pngPath) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[0](pngPath)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
          if (!arguments[1].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 2 (rlePath) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[1](rlePath)",
                "string",
                supernote_describe_value(runtime, arguments[1]));
          }
          if (!arguments[2].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 3 (width) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[2](width)",
                "number",
                supernote_describe_value(runtime, arguments[2]));
          }
          const double supernote_argument_2 = arguments[2].asNumber();
          if (!std::isfinite(supernote_argument_2) ||
              std::trunc(supernote_argument_2) != supernote_argument_2 ||
              supernote_argument_2 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_2 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 3 (width) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.encodeExportedBitmap.argument[2](width)", "int32",
                supernote_describe_value(runtime, arguments[2]));
          }
          if (!arguments[3].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 4 (height) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.encodeExportedBitmap.argument[3](height)",
                "number",
                supernote_describe_value(runtime, arguments[3]));
          }
          const double supernote_argument_3 = arguments[3].asNumber();
          if (!std::isfinite(supernote_argument_3) ||
              std::trunc(supernote_argument_3) != supernote_argument_3 ||
              supernote_argument_3 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_3 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.encodeExportedBitmap: argument 4 (height) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.encodeExportedBitmap.argument[3](height)", "int32",
                supernote_describe_value(runtime, arguments[3]));
          }
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
              runtime, facebook::jsi::Value(runtime, error.value()));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    function = supernote_attach_preflight(
        runtime, std::move(function), std::move(accepts),
        std::move(check_arguments));
    exports.setProperty(runtime, "encodeExportedBitmap", std::move(function));
  }
  {
    auto route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.adapters.Adapter_4c155f9e040e4f0c13b0", "([B)[B");
    auto function = Function::createFromHostFunction(
        runtime,
        PropNameID::forAscii(runtime, "greetFromJvm"),
        1,
        [route, feature_session](facebook::jsi::Runtime &runtime,
           const Value &,
           const Value *arguments,
           std::size_t argument_count) -> Value {
          if (argument_count != 1) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.greetFromJvm: expected 1 argument (string name); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.greetFromJvm",
                "1 argument (string name)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.greetFromJvm: argument 1 (name) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.greetFromJvm.argument[0](name)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
          try {
            if (!feature_session ||
                feature_session->state() != supernote::runtime::FeatureState::ACTIVE) {
              supernote_throw_error(
                  runtime, "FEATURE_CLOSED", "feature is closed");
            }
            auto resolved = route->get(feature_session);
            AttachedEnv attached;
            auto *env = attached.get();
            if (env == nullptr) {
              throw std::runtime_error("cannot attach to JavaVM");
            }
            LocalFrame frame(env);
            jvalue jvm_arguments[1]{};
            const auto argument_0 = arguments[0].asString(runtime).utf8(runtime);
            jvm_arguments[0].l = write_byte_array(env, reinterpret_cast<const std::byte *>(argument_0.data()), argument_0.size());
            auto result = env->CallStaticObjectMethodA(static_cast<jclass>(resolved->adapter_class.get()), resolved->method, jvm_arguments);
            require_no_implementation_exception(env);
            const auto bytes = read_byte_array(env, static_cast<jbyteArray>(result));
            const std::string text(
                reinterpret_cast<const char *>(bytes.data()), bytes.size());
            return facebook::jsi::Value(
                facebook::jsi::String::createFromUtf8(runtime, text));
          } catch (const facebook::jsi::JSError &) {
            throw;
          } catch (const JvmImplementationFailure &error) {
            supernote_throw_error(
                runtime, "IMPLEMENTATION_ERROR", error.what());
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    auto accepts = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "greetFromJvm.accepts"),
        1,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 1) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.greetFromJvm: expected 1 argument (string name); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.greetFromJvm",
                "1 argument (string name)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.greetFromJvm: argument 1 (name) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.greetFromJvm.argument[0](name)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
            return facebook::jsi::Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return facebook::jsi::Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    auto check_arguments = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "greetFromJvm.checkArguments"),
        1,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 1) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.greetFromJvm: expected 1 argument (string name); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.greetFromJvm",
                "1 argument (string name)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.greetFromJvm: argument 1 (name) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.greetFromJvm.argument[0](name)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
              runtime, facebook::jsi::Value(runtime, error.value()));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    function = supernote_attach_preflight(
        runtime, std::move(function), std::move(accepts),
        std::move(check_arguments));
    exports.setProperty(runtime, "greetFromJvm", std::move(function));
  }
  {
    auto route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.adapters.Adapter_d298b6e2e2d9e7cd7b1d", "(Lcom/ziv/snfiletools/NoteRefresh;[BIJ)Lkotlinx/coroutines/Job;");
    auto cancel_route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.runtime.SupernoteCoroutineBridge",
        "(Lkotlinx/coroutines/Job;)V", "cancel");
    auto owner_route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.adapters.Adapter_7ef685164ed9789997ba", "(Lcom/facebook/react/bridge/ReactApplicationContext;)Lcom/ziv/snfiletools/NoteRefresh;");
    auto function = Function::createFromHostFunction(
        runtime,
        PropNameID::forAscii(runtime, "clearNoteFileCache"),
        2,
        [route, cancel_route, feature_session, owner_route](facebook::jsi::Runtime &runtime,
           const Value &,
           const Value *arguments,
           std::size_t argument_count) -> Value {
          if (argument_count != 2) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.clearNoteFileCache: expected 2 arguments (string path, number page); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.clearNoteFileCache",
                "2 arguments (string path, number page)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.clearNoteFileCache: argument 1 (path) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.clearNoteFileCache.argument[0](path)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
          if (!arguments[1].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.clearNoteFileCache: argument 2 (page) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.clearNoteFileCache.argument[1](page)",
                "number",
                supernote_describe_value(runtime, arguments[1]));
          }
          const double supernote_argument_1 = arguments[1].asNumber();
          if (!std::isfinite(supernote_argument_1) ||
              std::trunc(supernote_argument_1) != supernote_argument_1 ||
              supernote_argument_1 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_1 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.clearNoteFileCache: argument 2 (page) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.clearNoteFileCache.argument[1](page)", "int32",
                supernote_describe_value(runtime, arguments[1]));
          }
          auto supernote_input_0 = arguments[0].asString(runtime).utf8(runtime);
          auto supernote_input_1 = static_cast<std::int32_t>(arguments[1].asNumber());
          struct SuspendState {
            bool success{false};
            std::optional<bool> value;
            std::string code;
            std::string error;
          };
          auto state = std::make_shared<SuspendState>();
          auto executor = Function::createFromHostFunction(
              runtime,
              PropNameID::forAscii(runtime, "SupernoteSuspendExecutor"),
              2,
              [route, cancel_route, feature_session, owner_route, state, supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](
                  facebook::jsi::Runtime &runtime,
                  const Value &,
                  const Value *continuation_arguments,
                  std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = feature_session
                    ? feature_session->accept_factory(
                          [](supernote::runtime::SessionId operation_id) {
                            return [operation_id](void *runtime_pointer) {
                              auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                  runtime_pointer);
                              supernote_reject_operation(
                                  runtime, operation_id, "FEATURE_CLOSED",
                                  "feature closed before async completion");
                            };
                          })
                    : nullptr;
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature =
                    feature_session;
                const auto completion_id =
                    supernote::runtime::process_services()
                        .register_jvm_async_completion(
                            [operation, operation_id, weak_feature, state](
                                void *environment, void *result,
                                std::string error_code,
                                std::string error_message) {
                              if (operation->cancellation_token().is_cancelled()) return;
                              if (!error_code.empty()) {
                                state->code = std::move(error_code);
                                state->error = std::move(error_message);
                              } else {
                                try {
                                  auto *env = static_cast<JNIEnv *>(environment);
                                  auto object = static_cast<jobject>(result);
                                  if (env == nullptr || object == nullptr) {
                                    throw std::runtime_error("Kotlin coroutine returned null");
                                  }
                                  LocalFrame frame(env);
                                  auto value_class = env->GetObjectClass(object);
                                  auto unbox = value_class == nullptr
                                      ? nullptr
                                      : env->GetMethodID(value_class, "booleanValue",
                                                         "()Z");
                                  if (unbox == nullptr) {
                                    clear_exception(env);
                                    throw std::runtime_error("cannot unbox Kotlin coroutine result");
                                  }
                                  auto value = env->CallBooleanMethod(object, unbox);
                                  if (env->ExceptionCheck()) {
                                    clear_exception(env);
                                    throw std::runtime_error("cannot read Kotlin coroutine result");
                                  }
                                  state->value = value == JNI_TRUE;
                                  state->success = true;
                                } catch (const std::exception &error) {
                                  state->code = "INTERNAL";
                                  state->error = error.what();
                                } catch (...) {
                                  state->code = "INTERNAL";
                                  state->error = "cannot decode Kotlin coroutine result";
                                }
                              }
                              if (operation->cancellation_token().is_cancelled()) return;
                              auto feature = weak_feature.lock();
                              if (!feature) return;
                              feature->schedule_completion(
                                  operation,
                                  [state, operation_id](void *runtime_pointer) {
                                    auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                        runtime_pointer);
                                    if (!state->success) {
                                      supernote_reject_operation(
                                          runtime, operation_id,
                                          state->code.empty()
                                              ? "INTERNAL"
                                              : state->code.c_str(),
                                          state->error.empty()
                                              ? "Kotlin coroutine failed"
                                              : state->error);
                                      return;
                                    }
                                    try {
                auto value = Value(*state->value);
                supernote_resolve_operation(
                    runtime, operation_id, std::move(value));
                                    } catch (const std::exception &error) {
                                      supernote_reject_operation(
                                          runtime, operation_id, "INTERNAL", error.what());
                                    }
                                  });
                            });
                operation->set_cancel_hook([completion_id] {
                  supernote::runtime::process_services()
                      .discard_jvm_async_completion(completion_id);
                });
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, weak_feature, route, cancel_route, completion_id, owner_route, supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](
                        supernote::runtime::CancellationToken executor_cancel) mutable {
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature ||
                          implementation_feature->state() !=
                              supernote::runtime::FeatureState::ACTIVE) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(
                          implementation_feature);
                      try {
            auto owner = implementation_feature->service<JvmOwner>(
                "jvm:com.ziv.snfiletools.NoteRefresh", [&] {
                  auto constructor = owner_route->get(implementation_feature);
                  auto runtime_session = implementation_feature->runtime();
                  auto context = runtime_session
                      ? runtime_session->platform_context() : nullptr;
                  if (!context) {
                    throw std::runtime_error("platform Context is unavailable");
                  }
                  AttachedEnv attached;
                  auto *env = attached.get();
                  if (env == nullptr) {
                    throw std::runtime_error("cannot attach to JavaVM");
                  }
                  LocalFrame frame(env);
                  jvalue arguments[1]{};
                  arguments[0].l = static_cast<jobject>(context.get());
                  auto local = env->CallStaticObjectMethodA(
                      static_cast<jclass>(constructor->adapter_class.get()),
                      constructor->method, arguments);
                  require_no_implementation_exception(env);
                  if (local == nullptr) {
                    throw std::runtime_error("JVM owner constructor returned null");
                  }
                  return std::make_shared<JvmOwner>(retain_global(env, local));
                });
              auto resolved = route->get(implementation_feature);
                        auto cancel_resolved = cancel_route->get(
                            implementation_feature);
                        AttachedEnv attached;
                        auto *env = attached.get();
                        if (env == nullptr) {
                          throw std::runtime_error("cannot attach to JavaVM");
                        }
                        LocalFrame frame(env);
                        jvalue jvm_arguments[4]{};
              jvm_arguments[0].l = static_cast<jobject>(owner->value.get());
              jvm_arguments[1].l = write_byte_array(env, reinterpret_cast<const std::byte *>(supernote_input_0.data()), supernote_input_0.size());
              jvm_arguments[2].i = static_cast<jint>(supernote_input_1);
              jvm_arguments[3].j = static_cast<jlong>(completion_id);
                        auto local_job = env->CallStaticObjectMethodA(
                            static_cast<jclass>(resolved->adapter_class.get()),
                            resolved->method, jvm_arguments);
                        if (env->ExceptionCheck()) {
                          require_no_implementation_exception(env);
                        }
                        if (local_job == nullptr) {
                          throw std::runtime_error(
                              "cannot launch generated Kotlin coroutine adapter");
                        }
                        auto job = retain_global(env, local_job);
                        operation->set_cancel_hook(
                            [completion_id, job, cancel_resolved] {
                              supernote::runtime::process_services()
                                  .discard_jvm_async_completion(completion_id);
                              try {
                                AttachedEnv attached;
                                auto *env = attached.get();
                                if (env == nullptr) return;
                                LocalFrame frame(env);
                                jvalue arguments[1]{};
                                arguments[0].l = static_cast<jobject>(job.get());
                                env->CallStaticVoidMethodA(
                                    static_cast<jclass>(
                                        cancel_resolved->adapter_class.get()),
                                    cancel_resolved->method, arguments);
                                clear_exception(env);
                              } catch (...) {}
                            });
                      } catch (const std::exception &error) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL", error.what());
                      } catch (...) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL",
                            "cannot launch Kotlin coroutine adapter");
                      }
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  supernote::runtime::process_services().complete_jvm_async(
                      completion_id, nullptr, nullptr, "RESOURCE_EXHAUSTED",
                      "Supernote worker queue is full");
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        });
    auto accepts = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "clearNoteFileCache.accepts"),
        2,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 2) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.clearNoteFileCache: expected 2 arguments (string path, number page); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.clearNoteFileCache",
                "2 arguments (string path, number page)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.clearNoteFileCache: argument 1 (path) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.clearNoteFileCache.argument[0](path)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
          if (!arguments[1].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.clearNoteFileCache: argument 2 (page) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.clearNoteFileCache.argument[1](page)",
                "number",
                supernote_describe_value(runtime, arguments[1]));
          }
          const double supernote_argument_1 = arguments[1].asNumber();
          if (!std::isfinite(supernote_argument_1) ||
              std::trunc(supernote_argument_1) != supernote_argument_1 ||
              supernote_argument_1 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_1 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.clearNoteFileCache: argument 2 (page) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.clearNoteFileCache.argument[1](page)", "int32",
                supernote_describe_value(runtime, arguments[1]));
          }
            return facebook::jsi::Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return facebook::jsi::Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    auto check_arguments = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "clearNoteFileCache.checkArguments"),
        2,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 2) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.clearNoteFileCache: expected 2 arguments (string path, number page); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.clearNoteFileCache",
                "2 arguments (string path, number page)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.clearNoteFileCache: argument 1 (path) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.clearNoteFileCache.argument[0](path)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
          if (!arguments[1].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.clearNoteFileCache: argument 2 (page) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.clearNoteFileCache.argument[1](page)",
                "number",
                supernote_describe_value(runtime, arguments[1]));
          }
          const double supernote_argument_1 = arguments[1].asNumber();
          if (!std::isfinite(supernote_argument_1) ||
              std::trunc(supernote_argument_1) != supernote_argument_1 ||
              supernote_argument_1 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_1 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.clearNoteFileCache: argument 2 (page) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.clearNoteFileCache.argument[1](page)", "int32",
                supernote_describe_value(runtime, arguments[1]));
          }
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
              runtime, facebook::jsi::Value(runtime, error.value()));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    function = supernote_attach_preflight(
        runtime, std::move(function), std::move(accepts),
        std::move(check_arguments));
    exports.setProperty(runtime, "clearNoteFileCache", std::move(function));
  }
  {
    auto route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.adapters.Adapter_c7f222126e065a1f88be", "(Lcom/ziv/snfiletools/NoteRefresh;J)Lkotlinx/coroutines/Job;");
    auto cancel_route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.runtime.SupernoteCoroutineBridge",
        "(Lkotlinx/coroutines/Job;)V", "cancel");
    auto owner_route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.adapters.Adapter_7ef685164ed9789997ba", "(Lcom/facebook/react/bridge/ReactApplicationContext;)Lcom/ziv/snfiletools/NoteRefresh;");
    auto function = Function::createFromHostFunction(
        runtime,
        PropNameID::forAscii(runtime, "requestNoteRedraw"),
        0,
        [route, cancel_route, feature_session, owner_route](facebook::jsi::Runtime &runtime,
           const Value &,
           const Value *arguments,
           std::size_t argument_count) -> Value {
          if (argument_count != 0) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.requestNoteRedraw: expected 0 arguments (); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.requestNoteRedraw",
                "0 arguments ()",
                std::to_string(argument_count) + " arguments");
          }

          struct SuspendState {
            bool success{false};
            std::optional<std::string> value;
            std::string code;
            std::string error;
          };
          auto state = std::make_shared<SuspendState>();
          auto executor = Function::createFromHostFunction(
              runtime,
              PropNameID::forAscii(runtime, "SupernoteSuspendExecutor"),
              2,
              [route, cancel_route, feature_session, owner_route, state](
                  facebook::jsi::Runtime &runtime,
                  const Value &,
                  const Value *continuation_arguments,
                  std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = feature_session
                    ? feature_session->accept_factory(
                          [](supernote::runtime::SessionId operation_id) {
                            return [operation_id](void *runtime_pointer) {
                              auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                  runtime_pointer);
                              supernote_reject_operation(
                                  runtime, operation_id, "FEATURE_CLOSED",
                                  "feature closed before async completion");
                            };
                          })
                    : nullptr;
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature =
                    feature_session;
                const auto completion_id =
                    supernote::runtime::process_services()
                        .register_jvm_async_completion(
                            [operation, operation_id, weak_feature, state](
                                void *environment, void *result,
                                std::string error_code,
                                std::string error_message) {
                              if (operation->cancellation_token().is_cancelled()) return;
                              if (!error_code.empty()) {
                                state->code = std::move(error_code);
                                state->error = std::move(error_message);
                              } else {
                                try {
                                  auto *env = static_cast<JNIEnv *>(environment);
                                  auto object = static_cast<jobject>(result);
                                  if (env == nullptr || object == nullptr) {
                                    throw std::runtime_error("Kotlin coroutine returned null");
                                  }
                                  LocalFrame frame(env);
                                  const auto bytes = read_byte_array(env, static_cast<jbyteArray>(object));
                                  state->value = std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size());
                                  state->success = true;
                                } catch (const std::exception &error) {
                                  state->code = "INTERNAL";
                                  state->error = error.what();
                                } catch (...) {
                                  state->code = "INTERNAL";
                                  state->error = "cannot decode Kotlin coroutine result";
                                }
                              }
                              if (operation->cancellation_token().is_cancelled()) return;
                              auto feature = weak_feature.lock();
                              if (!feature) return;
                              feature->schedule_completion(
                                  operation,
                                  [state, operation_id](void *runtime_pointer) {
                                    auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                        runtime_pointer);
                                    if (!state->success) {
                                      supernote_reject_operation(
                                          runtime, operation_id,
                                          state->code.empty()
                                              ? "INTERNAL"
                                              : state->code.c_str(),
                                          state->error.empty()
                                              ? "Kotlin coroutine failed"
                                              : state->error);
                                      return;
                                    }
                                    try {
                auto value = Value(String::createFromUtf8(runtime, *state->value));
                supernote_resolve_operation(
                    runtime, operation_id, std::move(value));
                                    } catch (const std::exception &error) {
                                      supernote_reject_operation(
                                          runtime, operation_id, "INTERNAL", error.what());
                                    }
                                  });
                            });
                operation->set_cancel_hook([completion_id] {
                  supernote::runtime::process_services()
                      .discard_jvm_async_completion(completion_id);
                });
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, weak_feature, route, cancel_route, completion_id, owner_route](
                        supernote::runtime::CancellationToken executor_cancel) mutable {
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature ||
                          implementation_feature->state() !=
                              supernote::runtime::FeatureState::ACTIVE) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(
                          implementation_feature);
                      try {
            auto owner = implementation_feature->service<JvmOwner>(
                "jvm:com.ziv.snfiletools.NoteRefresh", [&] {
                  auto constructor = owner_route->get(implementation_feature);
                  auto runtime_session = implementation_feature->runtime();
                  auto context = runtime_session
                      ? runtime_session->platform_context() : nullptr;
                  if (!context) {
                    throw std::runtime_error("platform Context is unavailable");
                  }
                  AttachedEnv attached;
                  auto *env = attached.get();
                  if (env == nullptr) {
                    throw std::runtime_error("cannot attach to JavaVM");
                  }
                  LocalFrame frame(env);
                  jvalue arguments[1]{};
                  arguments[0].l = static_cast<jobject>(context.get());
                  auto local = env->CallStaticObjectMethodA(
                      static_cast<jclass>(constructor->adapter_class.get()),
                      constructor->method, arguments);
                  require_no_implementation_exception(env);
                  if (local == nullptr) {
                    throw std::runtime_error("JVM owner constructor returned null");
                  }
                  return std::make_shared<JvmOwner>(retain_global(env, local));
                });
              auto resolved = route->get(implementation_feature);
                        auto cancel_resolved = cancel_route->get(
                            implementation_feature);
                        AttachedEnv attached;
                        auto *env = attached.get();
                        if (env == nullptr) {
                          throw std::runtime_error("cannot attach to JavaVM");
                        }
                        LocalFrame frame(env);
                        jvalue jvm_arguments[2]{};
              jvm_arguments[0].l = static_cast<jobject>(owner->value.get());
              jvm_arguments[1].j = static_cast<jlong>(completion_id);
                        auto local_job = env->CallStaticObjectMethodA(
                            static_cast<jclass>(resolved->adapter_class.get()),
                            resolved->method, jvm_arguments);
                        if (env->ExceptionCheck()) {
                          require_no_implementation_exception(env);
                        }
                        if (local_job == nullptr) {
                          throw std::runtime_error(
                              "cannot launch generated Kotlin coroutine adapter");
                        }
                        auto job = retain_global(env, local_job);
                        operation->set_cancel_hook(
                            [completion_id, job, cancel_resolved] {
                              supernote::runtime::process_services()
                                  .discard_jvm_async_completion(completion_id);
                              try {
                                AttachedEnv attached;
                                auto *env = attached.get();
                                if (env == nullptr) return;
                                LocalFrame frame(env);
                                jvalue arguments[1]{};
                                arguments[0].l = static_cast<jobject>(job.get());
                                env->CallStaticVoidMethodA(
                                    static_cast<jclass>(
                                        cancel_resolved->adapter_class.get()),
                                    cancel_resolved->method, arguments);
                                clear_exception(env);
                              } catch (...) {}
                            });
                      } catch (const std::exception &error) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL", error.what());
                      } catch (...) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL",
                            "cannot launch Kotlin coroutine adapter");
                      }
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  supernote::runtime::process_services().complete_jvm_async(
                      completion_id, nullptr, nullptr, "RESOURCE_EXHAUSTED",
                      "Supernote worker queue is full");
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        });
    auto accepts = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "requestNoteRedraw.accepts"),
        0,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 0) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.requestNoteRedraw: expected 0 arguments (); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.requestNoteRedraw",
                "0 arguments ()",
                std::to_string(argument_count) + " arguments");
          }
            return facebook::jsi::Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return facebook::jsi::Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    auto check_arguments = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "requestNoteRedraw.checkArguments"),
        0,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 0) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.requestNoteRedraw: expected 0 arguments (); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.requestNoteRedraw",
                "0 arguments ()",
                std::to_string(argument_count) + " arguments");
          }
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
              runtime, facebook::jsi::Value(runtime, error.value()));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    function = supernote_attach_preflight(
        runtime, std::move(function), std::move(accepts),
        std::move(check_arguments));
    exports.setProperty(runtime, "requestNoteRedraw", std::move(function));
  }
  {
    auto route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.adapters.Adapter_52224597b558df0cc584", "(Lcom/ziv/snfiletools/NoteRefresh;IJ)Lkotlinx/coroutines/Job;");
    auto cancel_route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.runtime.SupernoteCoroutineBridge",
        "(Lkotlinx/coroutines/Job;)V", "cancel");
    auto owner_route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.adapters.Adapter_7ef685164ed9789997ba", "(Lcom/facebook/react/bridge/ReactApplicationContext;)Lcom/ziv/snfiletools/NoteRefresh;");
    auto function = Function::createFromHostFunction(
        runtime,
        PropNameID::forAscii(runtime, "waitForNoteRefresh"),
        1,
        [route, cancel_route, feature_session, owner_route](facebook::jsi::Runtime &runtime,
           const Value &,
           const Value *arguments,
           std::size_t argument_count) -> Value {
          if (argument_count != 1) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.waitForNoteRefresh: expected 1 argument (number milliseconds); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.waitForNoteRefresh",
                "1 argument (number milliseconds)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.waitForNoteRefresh: argument 1 (milliseconds) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.waitForNoteRefresh.argument[0](milliseconds)",
                "number",
                supernote_describe_value(runtime, arguments[0]));
          }
          const double supernote_argument_0 = arguments[0].asNumber();
          if (!std::isfinite(supernote_argument_0) ||
              std::trunc(supernote_argument_0) != supernote_argument_0 ||
              supernote_argument_0 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_0 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.waitForNoteRefresh: argument 1 (milliseconds) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.waitForNoteRefresh.argument[0](milliseconds)", "int32",
                supernote_describe_value(runtime, arguments[0]));
          }
          auto supernote_input_0 = static_cast<std::int32_t>(arguments[0].asNumber());
          struct SuspendState {
            bool success{false};
            std::string code;
            std::string error;
          };
          auto state = std::make_shared<SuspendState>();
          auto executor = Function::createFromHostFunction(
              runtime,
              PropNameID::forAscii(runtime, "SupernoteSuspendExecutor"),
              2,
              [route, cancel_route, feature_session, owner_route, state, supernote_input_0 = std::move(supernote_input_0)](
                  facebook::jsi::Runtime &runtime,
                  const Value &,
                  const Value *continuation_arguments,
                  std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = feature_session
                    ? feature_session->accept_factory(
                          [](supernote::runtime::SessionId operation_id) {
                            return [operation_id](void *runtime_pointer) {
                              auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                  runtime_pointer);
                              supernote_reject_operation(
                                  runtime, operation_id, "FEATURE_CLOSED",
                                  "feature closed before async completion");
                            };
                          })
                    : nullptr;
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature =
                    feature_session;
                const auto completion_id =
                    supernote::runtime::process_services()
                        .register_jvm_async_completion(
                            [operation, operation_id, weak_feature, state](
                                void *environment, void *result,
                                std::string error_code,
                                std::string error_message) {
                              if (operation->cancellation_token().is_cancelled()) return;
                              if (!error_code.empty()) {
                                state->code = std::move(error_code);
                                state->error = std::move(error_message);
                              } else {
                                try {
                                  (void)environment;
                                  (void)result;
                                  state->success = true;
                                } catch (const std::exception &error) {
                                  state->code = "INTERNAL";
                                  state->error = error.what();
                                } catch (...) {
                                  state->code = "INTERNAL";
                                  state->error = "cannot decode Kotlin coroutine result";
                                }
                              }
                              if (operation->cancellation_token().is_cancelled()) return;
                              auto feature = weak_feature.lock();
                              if (!feature) return;
                              feature->schedule_completion(
                                  operation,
                                  [state, operation_id](void *runtime_pointer) {
                                    auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                        runtime_pointer);
                                    if (!state->success) {
                                      supernote_reject_operation(
                                          runtime, operation_id,
                                          state->code.empty()
                                              ? "INTERNAL"
                                              : state->code.c_str(),
                                          state->error.empty()
                                              ? "Kotlin coroutine failed"
                                              : state->error);
                                      return;
                                    }
                                    try {
                supernote_resolve_operation(
                    runtime, operation_id, Value::undefined());
                                    } catch (const std::exception &error) {
                                      supernote_reject_operation(
                                          runtime, operation_id, "INTERNAL", error.what());
                                    }
                                  });
                            });
                operation->set_cancel_hook([completion_id] {
                  supernote::runtime::process_services()
                      .discard_jvm_async_completion(completion_id);
                });
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, weak_feature, route, cancel_route, completion_id, owner_route, supernote_input_0 = std::move(supernote_input_0)](
                        supernote::runtime::CancellationToken executor_cancel) mutable {
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature ||
                          implementation_feature->state() !=
                              supernote::runtime::FeatureState::ACTIVE) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(
                          implementation_feature);
                      try {
            auto owner = implementation_feature->service<JvmOwner>(
                "jvm:com.ziv.snfiletools.NoteRefresh", [&] {
                  auto constructor = owner_route->get(implementation_feature);
                  auto runtime_session = implementation_feature->runtime();
                  auto context = runtime_session
                      ? runtime_session->platform_context() : nullptr;
                  if (!context) {
                    throw std::runtime_error("platform Context is unavailable");
                  }
                  AttachedEnv attached;
                  auto *env = attached.get();
                  if (env == nullptr) {
                    throw std::runtime_error("cannot attach to JavaVM");
                  }
                  LocalFrame frame(env);
                  jvalue arguments[1]{};
                  arguments[0].l = static_cast<jobject>(context.get());
                  auto local = env->CallStaticObjectMethodA(
                      static_cast<jclass>(constructor->adapter_class.get()),
                      constructor->method, arguments);
                  require_no_implementation_exception(env);
                  if (local == nullptr) {
                    throw std::runtime_error("JVM owner constructor returned null");
                  }
                  return std::make_shared<JvmOwner>(retain_global(env, local));
                });
              auto resolved = route->get(implementation_feature);
                        auto cancel_resolved = cancel_route->get(
                            implementation_feature);
                        AttachedEnv attached;
                        auto *env = attached.get();
                        if (env == nullptr) {
                          throw std::runtime_error("cannot attach to JavaVM");
                        }
                        LocalFrame frame(env);
                        jvalue jvm_arguments[3]{};
              jvm_arguments[0].l = static_cast<jobject>(owner->value.get());
              jvm_arguments[1].i = static_cast<jint>(supernote_input_0);
              jvm_arguments[2].j = static_cast<jlong>(completion_id);
                        auto local_job = env->CallStaticObjectMethodA(
                            static_cast<jclass>(resolved->adapter_class.get()),
                            resolved->method, jvm_arguments);
                        if (env->ExceptionCheck()) {
                          require_no_implementation_exception(env);
                        }
                        if (local_job == nullptr) {
                          throw std::runtime_error(
                              "cannot launch generated Kotlin coroutine adapter");
                        }
                        auto job = retain_global(env, local_job);
                        operation->set_cancel_hook(
                            [completion_id, job, cancel_resolved] {
                              supernote::runtime::process_services()
                                  .discard_jvm_async_completion(completion_id);
                              try {
                                AttachedEnv attached;
                                auto *env = attached.get();
                                if (env == nullptr) return;
                                LocalFrame frame(env);
                                jvalue arguments[1]{};
                                arguments[0].l = static_cast<jobject>(job.get());
                                env->CallStaticVoidMethodA(
                                    static_cast<jclass>(
                                        cancel_resolved->adapter_class.get()),
                                    cancel_resolved->method, arguments);
                                clear_exception(env);
                              } catch (...) {}
                            });
                      } catch (const std::exception &error) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL", error.what());
                      } catch (...) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL",
                            "cannot launch Kotlin coroutine adapter");
                      }
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  supernote::runtime::process_services().complete_jvm_async(
                      completion_id, nullptr, nullptr, "RESOURCE_EXHAUSTED",
                      "Supernote worker queue is full");
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        });
    auto accepts = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "waitForNoteRefresh.accepts"),
        1,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 1) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.waitForNoteRefresh: expected 1 argument (number milliseconds); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.waitForNoteRefresh",
                "1 argument (number milliseconds)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.waitForNoteRefresh: argument 1 (milliseconds) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.waitForNoteRefresh.argument[0](milliseconds)",
                "number",
                supernote_describe_value(runtime, arguments[0]));
          }
          const double supernote_argument_0 = arguments[0].asNumber();
          if (!std::isfinite(supernote_argument_0) ||
              std::trunc(supernote_argument_0) != supernote_argument_0 ||
              supernote_argument_0 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_0 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.waitForNoteRefresh: argument 1 (milliseconds) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.waitForNoteRefresh.argument[0](milliseconds)", "int32",
                supernote_describe_value(runtime, arguments[0]));
          }
            return facebook::jsi::Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return facebook::jsi::Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    auto check_arguments = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "waitForNoteRefresh.checkArguments"),
        1,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 1) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.waitForNoteRefresh: expected 1 argument (number milliseconds); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.waitForNoteRefresh",
                "1 argument (number milliseconds)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isNumber()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.waitForNoteRefresh: argument 1 (milliseconds) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.waitForNoteRefresh.argument[0](milliseconds)",
                "number",
                supernote_describe_value(runtime, arguments[0]));
          }
          const double supernote_argument_0 = arguments[0].asNumber();
          if (!std::isfinite(supernote_argument_0) ||
              std::trunc(supernote_argument_0) != supernote_argument_0 ||
              supernote_argument_0 < static_cast<double>(
                  std::numeric_limits<std::int32_t>::min()) ||
              supernote_argument_0 > static_cast<double>(
                  std::numeric_limits<std::int32_t>::max())) {
            supernote_throw_range_error(
                runtime, "Snfiletools.waitForNoteRefresh: argument 1 (milliseconds) must be a signed 32-bit integer",
                "OUT_OF_RANGE", "Snfiletools.waitForNoteRefresh.argument[0](milliseconds)", "int32",
                supernote_describe_value(runtime, arguments[0]));
          }
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
              runtime, facebook::jsi::Value(runtime, error.value()));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    function = supernote_attach_preflight(
        runtime, std::move(function), std::move(accepts),
        std::move(check_arguments));
    exports.setProperty(runtime, "waitForNoteRefresh", std::move(function));
  }
  {
    auto route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.adapters.Adapter_a00aa705aa10bfc2e531", "([BJ)Lkotlinx/coroutines/Job;");
    auto cancel_route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.runtime.SupernoteCoroutineBridge",
        "(Lkotlinx/coroutines/Job;)V", "cancel");
    auto function = Function::createFromHostFunction(
        runtime,
        PropNameID::forAscii(runtime, "getTotalPathTestState"),
        1,
        [route, cancel_route, feature_session](facebook::jsi::Runtime &runtime,
           const Value &,
           const Value *arguments,
           std::size_t argument_count) -> Value {
          if (argument_count != 1) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.getTotalPathTestState: expected 1 argument (string pluginDirectory); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.getTotalPathTestState",
                "1 argument (string pluginDirectory)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.getTotalPathTestState: argument 1 (pluginDirectory) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.getTotalPathTestState.argument[0](pluginDirectory)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
          auto supernote_input_0 = arguments[0].asString(runtime).utf8(runtime);
          struct SuspendState {
            bool success{false};
            std::optional<std::string> value;
            std::string code;
            std::string error;
          };
          auto state = std::make_shared<SuspendState>();
          auto executor = Function::createFromHostFunction(
              runtime,
              PropNameID::forAscii(runtime, "SupernoteSuspendExecutor"),
              2,
              [route, cancel_route, feature_session, state, supernote_input_0 = std::move(supernote_input_0)](
                  facebook::jsi::Runtime &runtime,
                  const Value &,
                  const Value *continuation_arguments,
                  std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = feature_session
                    ? feature_session->accept_factory(
                          [](supernote::runtime::SessionId operation_id) {
                            return [operation_id](void *runtime_pointer) {
                              auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                  runtime_pointer);
                              supernote_reject_operation(
                                  runtime, operation_id, "FEATURE_CLOSED",
                                  "feature closed before async completion");
                            };
                          })
                    : nullptr;
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature =
                    feature_session;
                const auto completion_id =
                    supernote::runtime::process_services()
                        .register_jvm_async_completion(
                            [operation, operation_id, weak_feature, state](
                                void *environment, void *result,
                                std::string error_code,
                                std::string error_message) {
                              if (operation->cancellation_token().is_cancelled()) return;
                              if (!error_code.empty()) {
                                state->code = std::move(error_code);
                                state->error = std::move(error_message);
                              } else {
                                try {
                                  auto *env = static_cast<JNIEnv *>(environment);
                                  auto object = static_cast<jobject>(result);
                                  if (env == nullptr || object == nullptr) {
                                    throw std::runtime_error("Kotlin coroutine returned null");
                                  }
                                  LocalFrame frame(env);
                                  const auto bytes = read_byte_array(env, static_cast<jbyteArray>(object));
                                  state->value = std::string(reinterpret_cast<const char *>(bytes.data()), bytes.size());
                                  state->success = true;
                                } catch (const std::exception &error) {
                                  state->code = "INTERNAL";
                                  state->error = error.what();
                                } catch (...) {
                                  state->code = "INTERNAL";
                                  state->error = "cannot decode Kotlin coroutine result";
                                }
                              }
                              if (operation->cancellation_token().is_cancelled()) return;
                              auto feature = weak_feature.lock();
                              if (!feature) return;
                              feature->schedule_completion(
                                  operation,
                                  [state, operation_id](void *runtime_pointer) {
                                    auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                        runtime_pointer);
                                    if (!state->success) {
                                      supernote_reject_operation(
                                          runtime, operation_id,
                                          state->code.empty()
                                              ? "INTERNAL"
                                              : state->code.c_str(),
                                          state->error.empty()
                                              ? "Kotlin coroutine failed"
                                              : state->error);
                                      return;
                                    }
                                    try {
                auto value = Value(String::createFromUtf8(runtime, *state->value));
                supernote_resolve_operation(
                    runtime, operation_id, std::move(value));
                                    } catch (const std::exception &error) {
                                      supernote_reject_operation(
                                          runtime, operation_id, "INTERNAL", error.what());
                                    }
                                  });
                            });
                operation->set_cancel_hook([completion_id] {
                  supernote::runtime::process_services()
                      .discard_jvm_async_completion(completion_id);
                });
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, weak_feature, route, cancel_route, completion_id, supernote_input_0 = std::move(supernote_input_0)](
                        supernote::runtime::CancellationToken executor_cancel) mutable {
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature ||
                          implementation_feature->state() !=
                              supernote::runtime::FeatureState::ACTIVE) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(
                          implementation_feature);
                      try {
              auto resolved = route->get(implementation_feature);
                        auto cancel_resolved = cancel_route->get(
                            implementation_feature);
                        AttachedEnv attached;
                        auto *env = attached.get();
                        if (env == nullptr) {
                          throw std::runtime_error("cannot attach to JavaVM");
                        }
                        LocalFrame frame(env);
                        jvalue jvm_arguments[2]{};
              jvm_arguments[0].l = write_byte_array(env, reinterpret_cast<const std::byte *>(supernote_input_0.data()), supernote_input_0.size());
              jvm_arguments[1].j = static_cast<jlong>(completion_id);
                        auto local_job = env->CallStaticObjectMethodA(
                            static_cast<jclass>(resolved->adapter_class.get()),
                            resolved->method, jvm_arguments);
                        if (env->ExceptionCheck()) {
                          require_no_implementation_exception(env);
                        }
                        if (local_job == nullptr) {
                          throw std::runtime_error(
                              "cannot launch generated Kotlin coroutine adapter");
                        }
                        auto job = retain_global(env, local_job);
                        operation->set_cancel_hook(
                            [completion_id, job, cancel_resolved] {
                              supernote::runtime::process_services()
                                  .discard_jvm_async_completion(completion_id);
                              try {
                                AttachedEnv attached;
                                auto *env = attached.get();
                                if (env == nullptr) return;
                                LocalFrame frame(env);
                                jvalue arguments[1]{};
                                arguments[0].l = static_cast<jobject>(job.get());
                                env->CallStaticVoidMethodA(
                                    static_cast<jclass>(
                                        cancel_resolved->adapter_class.get()),
                                    cancel_resolved->method, arguments);
                                clear_exception(env);
                              } catch (...) {}
                            });
                      } catch (const std::exception &error) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL", error.what());
                      } catch (...) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL",
                            "cannot launch Kotlin coroutine adapter");
                      }
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  supernote::runtime::process_services().complete_jvm_async(
                      completion_id, nullptr, nullptr, "RESOURCE_EXHAUSTED",
                      "Supernote worker queue is full");
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        });
    auto accepts = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "getTotalPathTestState.accepts"),
        1,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 1) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.getTotalPathTestState: expected 1 argument (string pluginDirectory); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.getTotalPathTestState",
                "1 argument (string pluginDirectory)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.getTotalPathTestState: argument 1 (pluginDirectory) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.getTotalPathTestState.argument[0](pluginDirectory)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
            return facebook::jsi::Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return facebook::jsi::Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    auto check_arguments = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "getTotalPathTestState.checkArguments"),
        1,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 1) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.getTotalPathTestState: expected 1 argument (string pluginDirectory); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.getTotalPathTestState",
                "1 argument (string pluginDirectory)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.getTotalPathTestState: argument 1 (pluginDirectory) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.getTotalPathTestState.argument[0](pluginDirectory)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
              runtime, facebook::jsi::Value(runtime, error.value()));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    function = supernote_attach_preflight(
        runtime, std::move(function), std::move(accepts),
        std::move(check_arguments));
    exports.setProperty(runtime, "getTotalPathTestState", std::move(function));
  }
  {
    auto route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.adapters.Adapter_4f6ff35573aec0b49b51", "([B[BJ)Lkotlinx/coroutines/Job;");
    auto cancel_route = std::make_shared<LazyJvmRoute>(
        "supernote.generated.runtime.SupernoteCoroutineBridge",
        "(Lkotlinx/coroutines/Job;)V", "cancel");
    auto function = Function::createFromHostFunction(
        runtime,
        PropNameID::forAscii(runtime, "setTotalPathTestState"),
        2,
        [route, cancel_route, feature_session](facebook::jsi::Runtime &runtime,
           const Value &,
           const Value *arguments,
           std::size_t argument_count) -> Value {
          if (argument_count != 2) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.setTotalPathTestState: expected 2 arguments (string pluginDirectory, string state); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.setTotalPathTestState",
                "2 arguments (string pluginDirectory, string state)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.setTotalPathTestState: argument 1 (pluginDirectory) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.setTotalPathTestState.argument[0](pluginDirectory)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
          if (!arguments[1].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.setTotalPathTestState: argument 2 (state) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.setTotalPathTestState.argument[1](state)",
                "string",
                supernote_describe_value(runtime, arguments[1]));
          }
          auto supernote_input_0 = arguments[0].asString(runtime).utf8(runtime);
          auto supernote_input_1 = arguments[1].asString(runtime).utf8(runtime);
          struct SuspendState {
            bool success{false};
            std::string code;
            std::string error;
          };
          auto state = std::make_shared<SuspendState>();
          auto executor = Function::createFromHostFunction(
              runtime,
              PropNameID::forAscii(runtime, "SupernoteSuspendExecutor"),
              2,
              [route, cancel_route, feature_session, state, supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](
                  facebook::jsi::Runtime &runtime,
                  const Value &,
                  const Value *continuation_arguments,
                  std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = feature_session
                    ? feature_session->accept_factory(
                          [](supernote::runtime::SessionId operation_id) {
                            return [operation_id](void *runtime_pointer) {
                              auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                  runtime_pointer);
                              supernote_reject_operation(
                                  runtime, operation_id, "FEATURE_CLOSED",
                                  "feature closed before async completion");
                            };
                          })
                    : nullptr;
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature =
                    feature_session;
                const auto completion_id =
                    supernote::runtime::process_services()
                        .register_jvm_async_completion(
                            [operation, operation_id, weak_feature, state](
                                void *environment, void *result,
                                std::string error_code,
                                std::string error_message) {
                              if (operation->cancellation_token().is_cancelled()) return;
                              if (!error_code.empty()) {
                                state->code = std::move(error_code);
                                state->error = std::move(error_message);
                              } else {
                                try {
                                  (void)environment;
                                  (void)result;
                                  state->success = true;
                                } catch (const std::exception &error) {
                                  state->code = "INTERNAL";
                                  state->error = error.what();
                                } catch (...) {
                                  state->code = "INTERNAL";
                                  state->error = "cannot decode Kotlin coroutine result";
                                }
                              }
                              if (operation->cancellation_token().is_cancelled()) return;
                              auto feature = weak_feature.lock();
                              if (!feature) return;
                              feature->schedule_completion(
                                  operation,
                                  [state, operation_id](void *runtime_pointer) {
                                    auto &runtime = *static_cast<facebook::jsi::Runtime *>(
                                        runtime_pointer);
                                    if (!state->success) {
                                      supernote_reject_operation(
                                          runtime, operation_id,
                                          state->code.empty()
                                              ? "INTERNAL"
                                              : state->code.c_str(),
                                          state->error.empty()
                                              ? "Kotlin coroutine failed"
                                              : state->error);
                                      return;
                                    }
                                    try {
                supernote_resolve_operation(
                    runtime, operation_id, Value::undefined());
                                    } catch (const std::exception &error) {
                                      supernote_reject_operation(
                                          runtime, operation_id, "INTERNAL", error.what());
                                    }
                                  });
                            });
                operation->set_cancel_hook([completion_id] {
                  supernote::runtime::process_services()
                      .discard_jvm_async_completion(completion_id);
                });
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, weak_feature, route, cancel_route, completion_id, supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](
                        supernote::runtime::CancellationToken executor_cancel) mutable {
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature ||
                          implementation_feature->state() !=
                              supernote::runtime::FeatureState::ACTIVE) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(
                          implementation_feature);
                      try {
              auto resolved = route->get(implementation_feature);
                        auto cancel_resolved = cancel_route->get(
                            implementation_feature);
                        AttachedEnv attached;
                        auto *env = attached.get();
                        if (env == nullptr) {
                          throw std::runtime_error("cannot attach to JavaVM");
                        }
                        LocalFrame frame(env);
                        jvalue jvm_arguments[3]{};
              jvm_arguments[0].l = write_byte_array(env, reinterpret_cast<const std::byte *>(supernote_input_0.data()), supernote_input_0.size());
              jvm_arguments[1].l = write_byte_array(env, reinterpret_cast<const std::byte *>(supernote_input_1.data()), supernote_input_1.size());
              jvm_arguments[2].j = static_cast<jlong>(completion_id);
                        auto local_job = env->CallStaticObjectMethodA(
                            static_cast<jclass>(resolved->adapter_class.get()),
                            resolved->method, jvm_arguments);
                        if (env->ExceptionCheck()) {
                          require_no_implementation_exception(env);
                        }
                        if (local_job == nullptr) {
                          throw std::runtime_error(
                              "cannot launch generated Kotlin coroutine adapter");
                        }
                        auto job = retain_global(env, local_job);
                        operation->set_cancel_hook(
                            [completion_id, job, cancel_resolved] {
                              supernote::runtime::process_services()
                                  .discard_jvm_async_completion(completion_id);
                              try {
                                AttachedEnv attached;
                                auto *env = attached.get();
                                if (env == nullptr) return;
                                LocalFrame frame(env);
                                jvalue arguments[1]{};
                                arguments[0].l = static_cast<jobject>(job.get());
                                env->CallStaticVoidMethodA(
                                    static_cast<jclass>(
                                        cancel_resolved->adapter_class.get()),
                                    cancel_resolved->method, arguments);
                                clear_exception(env);
                              } catch (...) {}
                            });
                      } catch (const std::exception &error) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL", error.what());
                      } catch (...) {
                        supernote::runtime::process_services().complete_jvm_async(
                            completion_id, nullptr, nullptr, "INTERNAL",
                            "cannot launch Kotlin coroutine adapter");
                      }
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  supernote::runtime::process_services().complete_jvm_async(
                      completion_id, nullptr, nullptr, "RESOURCE_EXHAUSTED",
                      "Supernote worker queue is full");
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        });
    auto accepts = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "setTotalPathTestState.accepts"),
        2,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 2) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.setTotalPathTestState: expected 2 arguments (string pluginDirectory, string state); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.setTotalPathTestState",
                "2 arguments (string pluginDirectory, string state)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.setTotalPathTestState: argument 1 (pluginDirectory) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.setTotalPathTestState.argument[0](pluginDirectory)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
          if (!arguments[1].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.setTotalPathTestState: argument 2 (state) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.setTotalPathTestState.argument[1](state)",
                "string",
                supernote_describe_value(runtime, arguments[1]));
          }
            return facebook::jsi::Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return facebook::jsi::Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    auto check_arguments = facebook::jsi::Function::createFromHostFunction(
        runtime,
        facebook::jsi::PropNameID::forAscii(runtime, "setTotalPathTestState.checkArguments"),
        2,
        [](facebook::jsi::Runtime &runtime,
           const facebook::jsi::Value &,
           const facebook::jsi::Value *arguments,
           std::size_t argument_count) -> facebook::jsi::Value {
          try {
          if (argument_count != 2) {
            supernote_throw_type_error(
                runtime, std::string("Snfiletools.setTotalPathTestState: expected 2 arguments (string pluginDirectory, string state); received ") +
                std::to_string(argument_count),
                "ARITY_MISMATCH", "Snfiletools.setTotalPathTestState",
                "2 arguments (string pluginDirectory, string state)",
                std::to_string(argument_count) + " arguments");
          }
          if (!arguments[0].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.setTotalPathTestState: argument 1 (pluginDirectory) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.setTotalPathTestState.argument[0](pluginDirectory)",
                "string",
                supernote_describe_value(runtime, arguments[0]));
          }
          if (!arguments[1].isString()) {
            supernote_throw_type_error(
                runtime, "Snfiletools.setTotalPathTestState: argument 2 (state) has the wrong JavaScript type",
                "TYPE_MISMATCH", "Snfiletools.setTotalPathTestState.argument[1](state)",
                "string",
                supernote_describe_value(runtime, arguments[1]));
          }
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
              runtime, facebook::jsi::Value(runtime, error.value()));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        });
    function = supernote_attach_preflight(
        runtime, std::move(function), std::move(accepts),
        std::move(check_arguments));
    exports.setProperty(runtime, "setTotalPathTestState", std::move(function));
  }

}

}  // namespace supernote::generated::jvm_feature_7e680f69f87879eb

namespace supernote::internal::Snfiletools {



}  // namespace supernote::internal::Snfiletools
