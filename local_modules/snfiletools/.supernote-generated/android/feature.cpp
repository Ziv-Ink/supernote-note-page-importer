// Supernote conversion plan SHA-256: d4ccb5337c1a6101964c1d1f9b3d93cb10685659838f40157c269e1ae64beccc
#include <supernote/conversion.hpp>
#include <supernote/cpp_objects.hpp>
#include <jni.h>
#include <jsi/jsi.h>
#include "runtime_services.hpp"


#include <android/log.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace supernote_feature_Snfiletools {
std::string greet(std::string name);
}
namespace supernote_feature_Snfiletools {
std::int64_t getSize(std::string path);
}
namespace supernote_feature_Snfiletools {
void setJournalDirectory(std::string path);
}
namespace supernote_feature_Snfiletools {
std::string testMetadataEditAndRestore(std::string path, std::string backupPath);
}
namespace supernote_feature_Snfiletools {
std::string testTotalPathEdit(std::string path, std::string backupPath);
}
namespace supernote_feature_Snfiletools {
void restoreTestNote(std::string path, std::string backupPath);
}
namespace supernote_feature_Snfiletools {
std::string testTotalPathPaste(std::string path, std::string sourcePath, std::string backupPath, std::string pageKey);
}
namespace supernote_feature_Snfiletools {
std::string testBitmapReplace(std::string path, std::string patternPath, std::string backupPath, std::string pageKey);
}
namespace supernote_feature_Snfiletools {
bool bitmapMatches(std::string path, std::string patternPath, std::string pageKey);
}
namespace supernote_feature_Snfiletools {
void testReopenSnapshot(std::string path, std::string snapshotPath);
}
namespace supernote_feature_Snfiletools {
std::string testTotalPathCopy(std::string path, std::string sourcePath, std::string backupPath, std::string pageKey);
}
namespace supernote_feature_Snfiletools {
std::string copyPageTotalPath(std::string path, std::string sourcePath, std::string backupPath, std::string pageKey, std::string sourcePageKey);
}
namespace supernote_feature_Snfiletools {
bool testBitmapUnchanged(std::string path, std::string referencePath, std::string pageKey);
}
namespace supernote_feature_Snfiletools {
bool testTotalPathUnchanged(std::string path, std::string referencePath, std::string pageKey);
}

namespace supernote::generated::feature_7e680f69f87879eb {

constexpr char kLogTag[] = "SupernoteJsiModuleFeature";
constexpr char kFeatureRegistryGlobal[] =
    "__supernoteModuleFeatureRegistry_63f6999c8c67";
constexpr char kFeatureId[] = "supernote:feature:7e680f69f87879eb";


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



std::int64_t supernote_module_from_js_12b61f4c9dec(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::string &path,
    std::uint64_t depth);
facebook::jsi::Value supernote_module_to_js_12b61f4c9dec(
    facebook::jsi::Runtime &runtime,
    const std::int64_t &value,
    const std::shared_ptr<supernote::runtime::CppObjectRegistry> &registry,
    const std::shared_ptr<supernote::runtime::FeatureSession> &feature,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth);
void supernote_module_retain_native_12b61f4c9dec(
    const std::int64_t &value,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::shared_ptr<supernote::runtime::DeferredDestruction> &cleanup);
void supernote_validate_js_12b61f4c9dec(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth);

bool supernote_module_from_js_1414fc31e8c5(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::string &path,
    std::uint64_t depth);
facebook::jsi::Value supernote_module_to_js_1414fc31e8c5(
    facebook::jsi::Runtime &runtime,
    const bool &value,
    const std::shared_ptr<supernote::runtime::CppObjectRegistry> &registry,
    const std::shared_ptr<supernote::runtime::FeatureSession> &feature,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth);
void supernote_module_retain_native_1414fc31e8c5(
    const bool &value,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::shared_ptr<supernote::runtime::DeferredDestruction> &cleanup);
void supernote_validate_js_1414fc31e8c5(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth);

std::string supernote_module_from_js_5184f4bd6853(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::string &path,
    std::uint64_t depth);
facebook::jsi::Value supernote_module_to_js_5184f4bd6853(
    facebook::jsi::Runtime &runtime,
    const std::string &value,
    const std::shared_ptr<supernote::runtime::CppObjectRegistry> &registry,
    const std::shared_ptr<supernote::runtime::FeatureSession> &feature,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth);
void supernote_module_retain_native_5184f4bd6853(
    const std::string &value,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::shared_ptr<supernote::runtime::DeferredDestruction> &cleanup);
void supernote_validate_js_5184f4bd6853(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth);

constexpr char kCppObjectRegistryProperty[] =
    "__supernoteModuleCppObjectRegistry_5f271b119c3a";

std::shared_ptr<supernote::runtime::CppObjectRegistry>
supernote_module_object_registry(facebook::jsi::Runtime &runtime) {
  auto registry = runtime.global().getPropertyAsObject(
      runtime, kFeatureRegistryGlobal);
  auto exports = registry.getPropertyAsObject(runtime, kFeatureId);
  auto owner = exports.getPropertyAsObject(
      runtime, kCppObjectRegistryProperty);
  return owner.getHostObject<supernote::runtime::CppObjectRegistryOwner>(
      runtime)->registry();
}

[[noreturn]] void supernote_module_throw_conversion_failure(
    facebook::jsi::Runtime &runtime,
    const supernote::conversion::Failure &failure) {
  if (failure.kind() == supernote::conversion::FailureKind::TYPE) {
    supernote_throw_type_error(
        runtime, failure.what(), "TYPE_MISMATCH", failure.path(),
        "valid generated value", "rejected");
  }
  if (failure.kind() == supernote::conversion::FailureKind::RANGE) {
    supernote_throw_range_error(
        runtime, failure.what(), "LIMIT_EXCEEDED", failure.path(),
        "within generated conversion limits", "rejected");
  }
  supernote_throw_error(runtime, "INTERNAL", failure.what());
}

std::int64_t supernote_module_from_js_12b61f4c9dec(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::string &path,
    std::uint64_t depth) {
  (void)retained;
  budget.visit(path, depth);
  if (value.isUndefined()) {
    supernote_throw_type_error(runtime, path + ": expected a defined value", "TYPE_MISMATCH", path, "a defined value", supernote_describe_value(runtime, value));
  }
  if (value.isNull()) {
    supernote_throw_type_error(runtime, path + ": expected a non-null value", "TYPE_MISMATCH", path, "a non-null value", supernote_describe_value(runtime, value));
  }
  if (!value.isBigInt()) supernote_throw_type_error(runtime, path + ": expected an int64 bigint", "TYPE_MISMATCH", path, "an int64 bigint", supernote_describe_value(runtime, value));
  const auto bigint = value.getBigInt(runtime);
  if (!bigint.isInt64(runtime)) {
    supernote_throw_range_error(runtime, path + ": int64 value is out of range",
        "OUT_OF_RANGE", path, "int64 bigint", "bigint");
  }
  return static_cast<std::int64_t>(bigint.asInt64(runtime));
}

void supernote_validate_js_12b61f4c9dec(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth) {
  budget.visit(path, depth);
  if (value.isUndefined()) {
    supernote_throw_type_error(runtime, path + ": expected a defined value", "TYPE_MISMATCH", path, "a defined value", supernote_describe_value(runtime, value));
  }
  if (value.isNull()) {
    supernote_throw_type_error(runtime, path + ": expected a non-null value", "TYPE_MISMATCH", path, "a non-null value", supernote_describe_value(runtime, value));
  }
  if (!value.isBigInt()) supernote_throw_type_error(runtime, path + ": expected an int64 bigint", "TYPE_MISMATCH", path, "an int64 bigint", supernote_describe_value(runtime, value));
  if (!value.getBigInt(runtime).isInt64(runtime)) {
    supernote_throw_range_error(runtime, path + ": int64 value is out of range",
        "OUT_OF_RANGE", path, "int64 bigint", "bigint");
  }
}

facebook::jsi::Value supernote_module_to_js_12b61f4c9dec(
    facebook::jsi::Runtime &runtime,
    const std::int64_t &value,
    const std::shared_ptr<supernote::runtime::CppObjectRegistry> &registry,
    const std::shared_ptr<supernote::runtime::FeatureSession> &feature,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth) {
  budget.visit(path, depth);
  return facebook::jsi::Value(facebook::jsi::BigInt::fromInt64(
      runtime, static_cast<std::int64_t>(value)));
}

void supernote_module_retain_native_12b61f4c9dec(
    const std::int64_t &value,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::shared_ptr<supernote::runtime::DeferredDestruction> &cleanup) {
  (void)value;
  (void)retained;
  (void)cleanup;
}

bool supernote_module_from_js_1414fc31e8c5(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::string &path,
    std::uint64_t depth) {
  (void)retained;
  budget.visit(path, depth);
  if (value.isUndefined()) {
    supernote_throw_type_error(runtime, path + ": expected a defined value", "TYPE_MISMATCH", path, "a defined value", supernote_describe_value(runtime, value));
  }
  if (value.isNull()) {
    supernote_throw_type_error(runtime, path + ": expected a non-null value", "TYPE_MISMATCH", path, "a non-null value", supernote_describe_value(runtime, value));
  }
  if (!value.isBool()) supernote_throw_type_error(runtime, path + ": expected boolean", "TYPE_MISMATCH", path, "boolean", supernote_describe_value(runtime, value));
  return value.getBool();
}

void supernote_validate_js_1414fc31e8c5(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth) {
  budget.visit(path, depth);
  if (value.isUndefined()) {
    supernote_throw_type_error(runtime, path + ": expected a defined value", "TYPE_MISMATCH", path, "a defined value", supernote_describe_value(runtime, value));
  }
  if (value.isNull()) {
    supernote_throw_type_error(runtime, path + ": expected a non-null value", "TYPE_MISMATCH", path, "a non-null value", supernote_describe_value(runtime, value));
  }
  if (!value.isBool()) supernote_throw_type_error(runtime, path + ": expected boolean", "TYPE_MISMATCH", path, "boolean", supernote_describe_value(runtime, value));
}

facebook::jsi::Value supernote_module_to_js_1414fc31e8c5(
    facebook::jsi::Runtime &runtime,
    const bool &value,
    const std::shared_ptr<supernote::runtime::CppObjectRegistry> &registry,
    const std::shared_ptr<supernote::runtime::FeatureSession> &feature,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth) {
  budget.visit(path, depth);
  return facebook::jsi::Value(value);
}

void supernote_module_retain_native_1414fc31e8c5(
    const bool &value,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::shared_ptr<supernote::runtime::DeferredDestruction> &cleanup) {
  (void)value;
  (void)retained;
  (void)cleanup;
}

std::string supernote_module_from_js_5184f4bd6853(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::string &path,
    std::uint64_t depth) {
  (void)retained;
  budget.visit(path, depth);
  if (value.isUndefined()) {
    supernote_throw_type_error(runtime, path + ": expected a defined value", "TYPE_MISMATCH", path, "a defined value", supernote_describe_value(runtime, value));
  }
  if (value.isNull()) {
    supernote_throw_type_error(runtime, path + ": expected a non-null value", "TYPE_MISMATCH", path, "a non-null value", supernote_describe_value(runtime, value));
  }
  if (!value.isString()) supernote_throw_type_error(runtime, path + ": expected a string", "TYPE_MISMATCH", path, "a string", supernote_describe_value(runtime, value));
  auto result = value.asString(runtime).utf8(runtime);
  budget.check_string_bytes(path, result.size());
  budget.reserve(path, result.size());
  return result;
}

void supernote_validate_js_5184f4bd6853(
    facebook::jsi::Runtime &runtime,
    const facebook::jsi::Value &value,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth) {
  budget.visit(path, depth);
  if (value.isUndefined()) {
    supernote_throw_type_error(runtime, path + ": expected a defined value", "TYPE_MISMATCH", path, "a defined value", supernote_describe_value(runtime, value));
  }
  if (value.isNull()) {
    supernote_throw_type_error(runtime, path + ": expected a non-null value", "TYPE_MISMATCH", path, "a non-null value", supernote_describe_value(runtime, value));
  }
  if (!value.isString()) supernote_throw_type_error(runtime, path + ": expected a string", "TYPE_MISMATCH", path, "a string", supernote_describe_value(runtime, value));
  auto text = value.asString(runtime).utf8(runtime);
  budget.check_string_bytes(path, text.size());
}

facebook::jsi::Value supernote_module_to_js_5184f4bd6853(
    facebook::jsi::Runtime &runtime,
    const std::string &value,
    const std::shared_ptr<supernote::runtime::CppObjectRegistry> &registry,
    const std::shared_ptr<supernote::runtime::FeatureSession> &feature,
    supernote::conversion::Budget &budget,
    const std::string &path,
    std::uint64_t depth) {
  budget.visit(path, depth);
  budget.check_string_bytes(path, value.size());
  budget.reserve(path, value.size());
  return facebook::jsi::Value(facebook::jsi::String::createFromUtf8(runtime, value));
}

void supernote_module_retain_native_5184f4bd6853(
    const std::string &value,
    std::vector<supernote::runtime::ManagedAnyRef> &retained,
    const std::shared_ptr<supernote::runtime::DeferredDestruction> &cleanup) {
  (void)value;
  (void)retained;
  (void)cleanup;
}


void register_feature(
    facebook::jsi::Runtime &runtime,
    facebook::jsi::Object &feature_registry,
    const std::shared_ptr<supernote::runtime::FeatureSession> &feature_session) {
  using facebook::jsi::Function;
  using facebook::jsi::Object;
  using facebook::jsi::PropNameID;
  using facebook::jsi::String;
  using facebook::jsi::Value;

  Object exports(runtime);
  auto object_registry = std::make_shared<supernote::runtime::CppObjectRegistry>(
      supernote::runtime::process_services().cleanup());
  exports.setProperty(
      runtime, kCppObjectRegistryProperty,
      Object::createFromHostObject(
          runtime, std::make_shared<supernote::runtime::CppObjectRegistryOwner>(
              object_registry)));
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "greet"),
        1,
        [feature_session, object_registry](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
        if (argument_count != 1) {
          supernote_throw_type_error(
              runtime, "Snfiletools.greet: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.greet",
              "1 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          [[maybe_unused]] supernote::conversion::Budget conversion_budget;
          [[maybe_unused]] std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.greet.argument[0](name)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          supernote::runtime::FeatureCallScope feature_call_scope(active_feature);
          auto native_result = ::supernote_feature_Snfiletools::greet(supernote_input_0);
          supernote::conversion::Budget result_budget;
          return supernote_module_to_js_5184f4bd6853(
              runtime, native_result, object_registry, active_feature, result_budget,
              "Snfiletools.greet.result", 1);
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "IMPLEMENTATION_ERROR",
              std::string("Snfiletools.greet: ") + error.what());
        } catch (...) {
          supernote_throw_error(
              runtime, "IMPLEMENTATION_ERROR",
              "Snfiletools.greet: unknown C++ exception");
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "greet.accepts"),
        1,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 1) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.greet.argument[0](name)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "greet.checkArguments"),
        1,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 1) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.greet: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.greet",
                "1 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.greet.argument[0](name)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "greet", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "getSize"),
        1,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 1) {
          supernote_throw_type_error(
              runtime, "Snfiletools.getSize: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.getSize",
              "1 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.getSize.argument[0](path)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::optional<std::int64_t> value;
            std::vector<supernote::runtime::ManagedAnyRef> retained_result;
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string>>(
              retained_objects, supernote_input_0);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        state->value.emplace(::supernote_feature_Snfiletools::getSize(supernote_input_0));
                      supernote_module_retain_native_12b61f4c9dec(
                          *state->value, state->retained_result,
                          supernote::runtime::process_services().cleanup());
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
                              return;
                            }
                            try {
                        auto object_registry = supernote_module_object_registry(runtime);
                        supernote::conversion::Budget result_budget;
                        auto value = supernote_module_to_js_12b61f4c9dec(
                            runtime, *state->value, object_registry,
                            completion_feature, result_budget,
                            "Snfiletools.getSize.result", 1);
                        supernote_resolve_operation(
                            runtime, operation_id, std::move(value));
                            } catch (const std::exception &error) {
                              supernote_reject_operation(
                                  runtime, operation_id, "INTERNAL", error.what());
                            }
                          });
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.getSize: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "getSize.accepts"),
        1,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 1) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.getSize.argument[0](path)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "getSize.checkArguments"),
        1,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 1) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.getSize: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.getSize",
                "1 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.getSize.argument[0](path)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "getSize", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "setJournalDirectory"),
        1,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 1) {
          supernote_throw_type_error(
              runtime, "Snfiletools.setJournalDirectory: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.setJournalDirectory",
              "1 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.setJournalDirectory.argument[0](path)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string>>(
              retained_objects, supernote_input_0);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        ::supernote_feature_Snfiletools::setJournalDirectory(supernote_input_0);
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
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
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.setJournalDirectory: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "setJournalDirectory.accepts"),
        1,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 1) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.setJournalDirectory.argument[0](path)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "setJournalDirectory.checkArguments"),
        1,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 1) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.setJournalDirectory: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.setJournalDirectory",
                "1 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.setJournalDirectory.argument[0](path)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "setJournalDirectory", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testMetadataEditAndRestore"),
        2,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 2) {
          supernote_throw_type_error(
              runtime, "Snfiletools.testMetadataEditAndRestore: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.testMetadataEditAndRestore",
              "2 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.testMetadataEditAndRestore.argument[0](path)", 1);
          auto supernote_input_1 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[1], conversion_budget, retained_objects,
              "Snfiletools.testMetadataEditAndRestore.argument[1](backupPath)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::optional<std::string> value;
            std::vector<supernote::runtime::ManagedAnyRef> retained_result;
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string, std::string>>(
              retained_objects, supernote_input_0, supernote_input_1);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        state->value.emplace(::supernote_feature_Snfiletools::testMetadataEditAndRestore(supernote_input_0, supernote_input_1));
                      supernote_module_retain_native_5184f4bd6853(
                          *state->value, state->retained_result,
                          supernote::runtime::process_services().cleanup());
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
                              return;
                            }
                            try {
                        auto object_registry = supernote_module_object_registry(runtime);
                        supernote::conversion::Budget result_budget;
                        auto value = supernote_module_to_js_5184f4bd6853(
                            runtime, *state->value, object_registry,
                            completion_feature, result_budget,
                            "Snfiletools.testMetadataEditAndRestore.result", 1);
                        supernote_resolve_operation(
                            runtime, operation_id, std::move(value));
                            } catch (const std::exception &error) {
                              supernote_reject_operation(
                                  runtime, operation_id, "INTERNAL", error.what());
                            }
                          });
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.testMetadataEditAndRestore: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testMetadataEditAndRestore.accepts"),
        2,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 2) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testMetadataEditAndRestore.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testMetadataEditAndRestore.argument[1](backupPath)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testMetadataEditAndRestore.checkArguments"),
        2,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 2) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.testMetadataEditAndRestore: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.testMetadataEditAndRestore",
                "2 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testMetadataEditAndRestore.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testMetadataEditAndRestore.argument[1](backupPath)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "testMetadataEditAndRestore", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathEdit"),
        2,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 2) {
          supernote_throw_type_error(
              runtime, "Snfiletools.testTotalPathEdit: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.testTotalPathEdit",
              "2 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathEdit.argument[0](path)", 1);
          auto supernote_input_1 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[1], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathEdit.argument[1](backupPath)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::optional<std::string> value;
            std::vector<supernote::runtime::ManagedAnyRef> retained_result;
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string, std::string>>(
              retained_objects, supernote_input_0, supernote_input_1);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        state->value.emplace(::supernote_feature_Snfiletools::testTotalPathEdit(supernote_input_0, supernote_input_1));
                      supernote_module_retain_native_5184f4bd6853(
                          *state->value, state->retained_result,
                          supernote::runtime::process_services().cleanup());
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
                              return;
                            }
                            try {
                        auto object_registry = supernote_module_object_registry(runtime);
                        supernote::conversion::Budget result_budget;
                        auto value = supernote_module_to_js_5184f4bd6853(
                            runtime, *state->value, object_registry,
                            completion_feature, result_budget,
                            "Snfiletools.testTotalPathEdit.result", 1);
                        supernote_resolve_operation(
                            runtime, operation_id, std::move(value));
                            } catch (const std::exception &error) {
                              supernote_reject_operation(
                                  runtime, operation_id, "INTERNAL", error.what());
                            }
                          });
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.testTotalPathEdit: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathEdit.accepts"),
        2,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 2) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testTotalPathEdit.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testTotalPathEdit.argument[1](backupPath)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathEdit.checkArguments"),
        2,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 2) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.testTotalPathEdit: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.testTotalPathEdit",
                "2 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testTotalPathEdit.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testTotalPathEdit.argument[1](backupPath)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "testTotalPathEdit", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "restoreTestNote"),
        2,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 2) {
          supernote_throw_type_error(
              runtime, "Snfiletools.restoreTestNote: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.restoreTestNote",
              "2 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.restoreTestNote.argument[0](path)", 1);
          auto supernote_input_1 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[1], conversion_budget, retained_objects,
              "Snfiletools.restoreTestNote.argument[1](backupPath)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string, std::string>>(
              retained_objects, supernote_input_0, supernote_input_1);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        ::supernote_feature_Snfiletools::restoreTestNote(supernote_input_0, supernote_input_1);
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
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
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.restoreTestNote: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "restoreTestNote.accepts"),
        2,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 2) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.restoreTestNote.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.restoreTestNote.argument[1](backupPath)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "restoreTestNote.checkArguments"),
        2,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 2) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.restoreTestNote: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.restoreTestNote",
                "2 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.restoreTestNote.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.restoreTestNote.argument[1](backupPath)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "restoreTestNote", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathPaste"),
        4,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 4) {
          supernote_throw_type_error(
              runtime, "Snfiletools.testTotalPathPaste: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.testTotalPathPaste",
              "4 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathPaste.argument[0](path)", 1);
          auto supernote_input_1 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[1], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathPaste.argument[1](sourcePath)", 1);
          auto supernote_input_2 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[2], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathPaste.argument[2](backupPath)", 1);
          auto supernote_input_3 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[3], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathPaste.argument[3](pageKey)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::optional<std::string> value;
            std::vector<supernote::runtime::ManagedAnyRef> retained_result;
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string, std::string, std::string, std::string>>(
              retained_objects, supernote_input_0, supernote_input_1, supernote_input_2, supernote_input_3);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2), supernote_input_3 = std::move(supernote_input_3)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2), supernote_input_3 = std::move(supernote_input_3)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        state->value.emplace(::supernote_feature_Snfiletools::testTotalPathPaste(supernote_input_0, supernote_input_1, supernote_input_2, supernote_input_3));
                      supernote_module_retain_native_5184f4bd6853(
                          *state->value, state->retained_result,
                          supernote::runtime::process_services().cleanup());
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
                              return;
                            }
                            try {
                        auto object_registry = supernote_module_object_registry(runtime);
                        supernote::conversion::Budget result_budget;
                        auto value = supernote_module_to_js_5184f4bd6853(
                            runtime, *state->value, object_registry,
                            completion_feature, result_budget,
                            "Snfiletools.testTotalPathPaste.result", 1);
                        supernote_resolve_operation(
                            runtime, operation_id, std::move(value));
                            } catch (const std::exception &error) {
                              supernote_reject_operation(
                                  runtime, operation_id, "INTERNAL", error.what());
                            }
                          });
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.testTotalPathPaste: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathPaste.accepts"),
        4,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 4) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testTotalPathPaste.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testTotalPathPaste.argument[1](sourcePath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.testTotalPathPaste.argument[2](backupPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[3], conversion_budget,
                "Snfiletools.testTotalPathPaste.argument[3](pageKey)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathPaste.checkArguments"),
        4,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 4) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.testTotalPathPaste: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.testTotalPathPaste",
                "4 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testTotalPathPaste.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testTotalPathPaste.argument[1](sourcePath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.testTotalPathPaste.argument[2](backupPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[3], conversion_budget,
                "Snfiletools.testTotalPathPaste.argument[3](pageKey)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "testTotalPathPaste", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testBitmapReplace"),
        4,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 4) {
          supernote_throw_type_error(
              runtime, "Snfiletools.testBitmapReplace: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.testBitmapReplace",
              "4 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.testBitmapReplace.argument[0](path)", 1);
          auto supernote_input_1 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[1], conversion_budget, retained_objects,
              "Snfiletools.testBitmapReplace.argument[1](patternPath)", 1);
          auto supernote_input_2 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[2], conversion_budget, retained_objects,
              "Snfiletools.testBitmapReplace.argument[2](backupPath)", 1);
          auto supernote_input_3 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[3], conversion_budget, retained_objects,
              "Snfiletools.testBitmapReplace.argument[3](pageKey)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::optional<std::string> value;
            std::vector<supernote::runtime::ManagedAnyRef> retained_result;
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string, std::string, std::string, std::string>>(
              retained_objects, supernote_input_0, supernote_input_1, supernote_input_2, supernote_input_3);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2), supernote_input_3 = std::move(supernote_input_3)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2), supernote_input_3 = std::move(supernote_input_3)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        state->value.emplace(::supernote_feature_Snfiletools::testBitmapReplace(supernote_input_0, supernote_input_1, supernote_input_2, supernote_input_3));
                      supernote_module_retain_native_5184f4bd6853(
                          *state->value, state->retained_result,
                          supernote::runtime::process_services().cleanup());
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
                              return;
                            }
                            try {
                        auto object_registry = supernote_module_object_registry(runtime);
                        supernote::conversion::Budget result_budget;
                        auto value = supernote_module_to_js_5184f4bd6853(
                            runtime, *state->value, object_registry,
                            completion_feature, result_budget,
                            "Snfiletools.testBitmapReplace.result", 1);
                        supernote_resolve_operation(
                            runtime, operation_id, std::move(value));
                            } catch (const std::exception &error) {
                              supernote_reject_operation(
                                  runtime, operation_id, "INTERNAL", error.what());
                            }
                          });
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.testBitmapReplace: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testBitmapReplace.accepts"),
        4,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 4) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testBitmapReplace.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testBitmapReplace.argument[1](patternPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.testBitmapReplace.argument[2](backupPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[3], conversion_budget,
                "Snfiletools.testBitmapReplace.argument[3](pageKey)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testBitmapReplace.checkArguments"),
        4,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 4) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.testBitmapReplace: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.testBitmapReplace",
                "4 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testBitmapReplace.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testBitmapReplace.argument[1](patternPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.testBitmapReplace.argument[2](backupPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[3], conversion_budget,
                "Snfiletools.testBitmapReplace.argument[3](pageKey)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "testBitmapReplace", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "bitmapMatches"),
        3,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 3) {
          supernote_throw_type_error(
              runtime, "Snfiletools.bitmapMatches: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.bitmapMatches",
              "3 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.bitmapMatches.argument[0](path)", 1);
          auto supernote_input_1 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[1], conversion_budget, retained_objects,
              "Snfiletools.bitmapMatches.argument[1](patternPath)", 1);
          auto supernote_input_2 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[2], conversion_budget, retained_objects,
              "Snfiletools.bitmapMatches.argument[2](pageKey)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::optional<bool> value;
            std::vector<supernote::runtime::ManagedAnyRef> retained_result;
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string, std::string, std::string>>(
              retained_objects, supernote_input_0, supernote_input_1, supernote_input_2);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        state->value.emplace(::supernote_feature_Snfiletools::bitmapMatches(supernote_input_0, supernote_input_1, supernote_input_2));
                      supernote_module_retain_native_1414fc31e8c5(
                          *state->value, state->retained_result,
                          supernote::runtime::process_services().cleanup());
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
                              return;
                            }
                            try {
                        auto object_registry = supernote_module_object_registry(runtime);
                        supernote::conversion::Budget result_budget;
                        auto value = supernote_module_to_js_1414fc31e8c5(
                            runtime, *state->value, object_registry,
                            completion_feature, result_budget,
                            "Snfiletools.bitmapMatches.result", 1);
                        supernote_resolve_operation(
                            runtime, operation_id, std::move(value));
                            } catch (const std::exception &error) {
                              supernote_reject_operation(
                                  runtime, operation_id, "INTERNAL", error.what());
                            }
                          });
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.bitmapMatches: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "bitmapMatches.accepts"),
        3,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 3) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.bitmapMatches.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.bitmapMatches.argument[1](patternPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.bitmapMatches.argument[2](pageKey)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "bitmapMatches.checkArguments"),
        3,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 3) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.bitmapMatches: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.bitmapMatches",
                "3 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.bitmapMatches.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.bitmapMatches.argument[1](patternPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.bitmapMatches.argument[2](pageKey)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "bitmapMatches", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testReopenSnapshot"),
        2,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 2) {
          supernote_throw_type_error(
              runtime, "Snfiletools.testReopenSnapshot: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.testReopenSnapshot",
              "2 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.testReopenSnapshot.argument[0](path)", 1);
          auto supernote_input_1 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[1], conversion_budget, retained_objects,
              "Snfiletools.testReopenSnapshot.argument[1](snapshotPath)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string, std::string>>(
              retained_objects, supernote_input_0, supernote_input_1);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        ::supernote_feature_Snfiletools::testReopenSnapshot(supernote_input_0, supernote_input_1);
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
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
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.testReopenSnapshot: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testReopenSnapshot.accepts"),
        2,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 2) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testReopenSnapshot.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testReopenSnapshot.argument[1](snapshotPath)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testReopenSnapshot.checkArguments"),
        2,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 2) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.testReopenSnapshot: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.testReopenSnapshot",
                "2 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testReopenSnapshot.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testReopenSnapshot.argument[1](snapshotPath)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "testReopenSnapshot", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathCopy"),
        4,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 4) {
          supernote_throw_type_error(
              runtime, "Snfiletools.testTotalPathCopy: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.testTotalPathCopy",
              "4 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathCopy.argument[0](path)", 1);
          auto supernote_input_1 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[1], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathCopy.argument[1](sourcePath)", 1);
          auto supernote_input_2 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[2], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathCopy.argument[2](backupPath)", 1);
          auto supernote_input_3 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[3], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathCopy.argument[3](pageKey)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::optional<std::string> value;
            std::vector<supernote::runtime::ManagedAnyRef> retained_result;
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string, std::string, std::string, std::string>>(
              retained_objects, supernote_input_0, supernote_input_1, supernote_input_2, supernote_input_3);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2), supernote_input_3 = std::move(supernote_input_3)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2), supernote_input_3 = std::move(supernote_input_3)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        state->value.emplace(::supernote_feature_Snfiletools::testTotalPathCopy(supernote_input_0, supernote_input_1, supernote_input_2, supernote_input_3));
                      supernote_module_retain_native_5184f4bd6853(
                          *state->value, state->retained_result,
                          supernote::runtime::process_services().cleanup());
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
                              return;
                            }
                            try {
                        auto object_registry = supernote_module_object_registry(runtime);
                        supernote::conversion::Budget result_budget;
                        auto value = supernote_module_to_js_5184f4bd6853(
                            runtime, *state->value, object_registry,
                            completion_feature, result_budget,
                            "Snfiletools.testTotalPathCopy.result", 1);
                        supernote_resolve_operation(
                            runtime, operation_id, std::move(value));
                            } catch (const std::exception &error) {
                              supernote_reject_operation(
                                  runtime, operation_id, "INTERNAL", error.what());
                            }
                          });
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.testTotalPathCopy: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathCopy.accepts"),
        4,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 4) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testTotalPathCopy.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testTotalPathCopy.argument[1](sourcePath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.testTotalPathCopy.argument[2](backupPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[3], conversion_budget,
                "Snfiletools.testTotalPathCopy.argument[3](pageKey)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathCopy.checkArguments"),
        4,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 4) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.testTotalPathCopy: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.testTotalPathCopy",
                "4 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testTotalPathCopy.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testTotalPathCopy.argument[1](sourcePath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.testTotalPathCopy.argument[2](backupPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[3], conversion_budget,
                "Snfiletools.testTotalPathCopy.argument[3](pageKey)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "testTotalPathCopy", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "copyPageTotalPath"),
        5,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 5) {
          supernote_throw_type_error(
              runtime, "Snfiletools.copyPageTotalPath: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.copyPageTotalPath",
              "5 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.copyPageTotalPath.argument[0](path)", 1);
          auto supernote_input_1 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[1], conversion_budget, retained_objects,
              "Snfiletools.copyPageTotalPath.argument[1](sourcePath)", 1);
          auto supernote_input_2 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[2], conversion_budget, retained_objects,
              "Snfiletools.copyPageTotalPath.argument[2](backupPath)", 1);
          auto supernote_input_3 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[3], conversion_budget, retained_objects,
              "Snfiletools.copyPageTotalPath.argument[3](pageKey)", 1);
          auto supernote_input_4 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[4], conversion_budget, retained_objects,
              "Snfiletools.copyPageTotalPath.argument[4](sourcePageKey)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::optional<std::string> value;
            std::vector<supernote::runtime::ManagedAnyRef> retained_result;
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string, std::string, std::string, std::string, std::string>>(
              retained_objects, supernote_input_0, supernote_input_1, supernote_input_2, supernote_input_3, supernote_input_4);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2), supernote_input_3 = std::move(supernote_input_3), supernote_input_4 = std::move(supernote_input_4)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2), supernote_input_3 = std::move(supernote_input_3), supernote_input_4 = std::move(supernote_input_4)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        state->value.emplace(::supernote_feature_Snfiletools::copyPageTotalPath(supernote_input_0, supernote_input_1, supernote_input_2, supernote_input_3, supernote_input_4));
                      supernote_module_retain_native_5184f4bd6853(
                          *state->value, state->retained_result,
                          supernote::runtime::process_services().cleanup());
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
                              return;
                            }
                            try {
                        auto object_registry = supernote_module_object_registry(runtime);
                        supernote::conversion::Budget result_budget;
                        auto value = supernote_module_to_js_5184f4bd6853(
                            runtime, *state->value, object_registry,
                            completion_feature, result_budget,
                            "Snfiletools.copyPageTotalPath.result", 1);
                        supernote_resolve_operation(
                            runtime, operation_id, std::move(value));
                            } catch (const std::exception &error) {
                              supernote_reject_operation(
                                  runtime, operation_id, "INTERNAL", error.what());
                            }
                          });
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.copyPageTotalPath: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "copyPageTotalPath.accepts"),
        5,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 5) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.copyPageTotalPath.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.copyPageTotalPath.argument[1](sourcePath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.copyPageTotalPath.argument[2](backupPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[3], conversion_budget,
                "Snfiletools.copyPageTotalPath.argument[3](pageKey)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[4], conversion_budget,
                "Snfiletools.copyPageTotalPath.argument[4](sourcePageKey)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "copyPageTotalPath.checkArguments"),
        5,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 5) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.copyPageTotalPath: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.copyPageTotalPath",
                "5 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.copyPageTotalPath.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.copyPageTotalPath.argument[1](sourcePath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.copyPageTotalPath.argument[2](backupPath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[3], conversion_budget,
                "Snfiletools.copyPageTotalPath.argument[3](pageKey)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[4], conversion_budget,
                "Snfiletools.copyPageTotalPath.argument[4](sourcePageKey)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "copyPageTotalPath", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testBitmapUnchanged"),
        3,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 3) {
          supernote_throw_type_error(
              runtime, "Snfiletools.testBitmapUnchanged: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.testBitmapUnchanged",
              "3 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.testBitmapUnchanged.argument[0](path)", 1);
          auto supernote_input_1 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[1], conversion_budget, retained_objects,
              "Snfiletools.testBitmapUnchanged.argument[1](referencePath)", 1);
          auto supernote_input_2 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[2], conversion_budget, retained_objects,
              "Snfiletools.testBitmapUnchanged.argument[2](pageKey)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::optional<bool> value;
            std::vector<supernote::runtime::ManagedAnyRef> retained_result;
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string, std::string, std::string>>(
              retained_objects, supernote_input_0, supernote_input_1, supernote_input_2);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        state->value.emplace(::supernote_feature_Snfiletools::testBitmapUnchanged(supernote_input_0, supernote_input_1, supernote_input_2));
                      supernote_module_retain_native_1414fc31e8c5(
                          *state->value, state->retained_result,
                          supernote::runtime::process_services().cleanup());
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
                              return;
                            }
                            try {
                        auto object_registry = supernote_module_object_registry(runtime);
                        supernote::conversion::Budget result_budget;
                        auto value = supernote_module_to_js_1414fc31e8c5(
                            runtime, *state->value, object_registry,
                            completion_feature, result_budget,
                            "Snfiletools.testBitmapUnchanged.result", 1);
                        supernote_resolve_operation(
                            runtime, operation_id, std::move(value));
                            } catch (const std::exception &error) {
                              supernote_reject_operation(
                                  runtime, operation_id, "INTERNAL", error.what());
                            }
                          });
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.testBitmapUnchanged: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testBitmapUnchanged.accepts"),
        3,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 3) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testBitmapUnchanged.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testBitmapUnchanged.argument[1](referencePath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.testBitmapUnchanged.argument[2](pageKey)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testBitmapUnchanged.checkArguments"),
        3,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 3) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.testBitmapUnchanged: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.testBitmapUnchanged",
                "3 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testBitmapUnchanged.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testBitmapUnchanged.argument[1](referencePath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.testBitmapUnchanged.argument[2](pageKey)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "testBitmapUnchanged", std::move(function));
  }
  {
    auto function = supernote_attach_preflight(
        runtime,
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathUnchanged"),
        3,
        [feature_session](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) mutable -> Value {
        if (argument_count != 3) {
          supernote_throw_type_error(
              runtime, "Snfiletools.testTotalPathUnchanged: wrong argument count",
              "ARITY_MISMATCH", "Snfiletools.testTotalPathUnchanged",
              "3 arguments",
              std::to_string(argument_count) + " arguments");
        }
        try {
          supernote::conversion::Budget conversion_budget;
          std::vector<supernote::runtime::ManagedAnyRef> retained_objects;
          auto supernote_input_0 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[0], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathUnchanged.argument[0](path)", 1);
          auto supernote_input_1 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[1], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathUnchanged.argument[1](referencePath)", 1);
          auto supernote_input_2 = supernote_module_from_js_5184f4bd6853(
              runtime, arguments[2], conversion_budget, retained_objects,
              "Snfiletools.testTotalPathUnchanged.argument[2](pageKey)", 1);
          auto active_feature = feature_session;
          if (!active_feature ||
              active_feature->state() != supernote::runtime::FeatureState::ACTIVE) {
            supernote_throw_error(runtime, "FEATURE_CLOSED", "feature is closed");
          }
          struct AsyncState {
            bool success{false};
            std::optional<bool> value;
            std::vector<supernote::runtime::ManagedAnyRef> retained_result;
            std::string error;
          };
          auto state = std::make_shared<AsyncState>();
          auto retained_input_state = std::make_shared<std::tuple<
              std::vector<supernote::runtime::ManagedAnyRef>, std::string, std::string, std::string>>(
              retained_objects, supernote_input_0, supernote_input_1, supernote_input_2);
          auto executor = Function::createFromHostFunction(
              runtime, PropNameID::forAscii(runtime, "SupernoteAsyncExecutor"), 2,
              [active_feature, state, retained_input_state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2)](facebook::jsi::Runtime &runtime,
                 const Value &, const Value *continuation_arguments,
                 std::size_t continuation_count) mutable -> Value {
                if (continuation_count != 2 ||
                    !continuation_arguments[0].isObject() ||
                    !continuation_arguments[1].isObject()) {
                  throw facebook::jsi::JSError(
                      runtime, "Promise supplied invalid continuation functions");
                }
                auto operation = active_feature->accept_factory(
                    [](supernote::runtime::SessionId operation_id) {
                      return [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "FEATURE_CLOSED",
                            "feature closed before async completion");
                      };
                    });
                if (!operation) {
                  supernote_reject_new_promise(
                      runtime, continuation_arguments[1], "FEATURE_CLOSED",
                      "feature is closed");
                  return Value::undefined();
                }
                operation->set_retained_state(retained_input_state);
                const auto operation_id = operation->id();
                supernote_register_continuation(
                    runtime, operation_id, continuation_arguments[0],
                    continuation_arguments[1]);
                std::weak_ptr<supernote::runtime::FeatureSession> weak_feature = active_feature;
                auto work = supernote::runtime::process_services().workers().submit(
                    [operation, operation_id, weak_feature, state, retained_objects = std::move(retained_objects), supernote_input_0 = std::move(supernote_input_0), supernote_input_1 = std::move(supernote_input_1), supernote_input_2 = std::move(supernote_input_2)](supernote::runtime::CancellationToken executor_cancel) mutable {
                      (void)retained_objects;
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto implementation_feature = weak_feature.lock();
                      if (!implementation_feature) return;
                      supernote::runtime::FeatureCallScope feature_call_scope(implementation_feature);
                      implementation_feature.reset();
                      try {
                        state->value.emplace(::supernote_feature_Snfiletools::testTotalPathUnchanged(supernote_input_0, supernote_input_1, supernote_input_2));
                      supernote_module_retain_native_1414fc31e8c5(
                          *state->value, state->retained_result,
                          supernote::runtime::process_services().cleanup());
                      state->success = true;
                      } catch (const std::exception &error) {
                        state->error = error.what();
                      } catch (...) {
                        state->error = "unknown C++ implementation failure";
                      }
                      if (executor_cancel.is_cancelled() ||
                          operation->cancellation_token().is_cancelled()) return;
                      auto completion_feature = weak_feature.lock();
                      if (!completion_feature) return;
                      completion_feature->schedule_completion(
                          operation, [state, operation_id, completion_feature](void *runtime_pointer) {
                            auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                            if (!state->success) {
                              supernote_reject_operation(
                                  runtime, operation_id, "IMPLEMENTATION_ERROR",
                                  state->error.empty() ? "C++ implementation failed" : state->error);
                              return;
                            }
                            try {
                        auto object_registry = supernote_module_object_registry(runtime);
                        supernote::conversion::Budget result_budget;
                        auto value = supernote_module_to_js_1414fc31e8c5(
                            runtime, *state->value, object_registry,
                            completion_feature, result_budget,
                            "Snfiletools.testTotalPathUnchanged.result", 1);
                        supernote_resolve_operation(
                            runtime, operation_id, std::move(value));
                            } catch (const std::exception &error) {
                              supernote_reject_operation(
                                  runtime, operation_id, "INTERNAL", error.what());
                            }
                          });
                    });
                operation->set_work(work);
                if (!work.accepted()) {
                  active_feature->schedule_completion(
                      operation, [operation_id](void *runtime_pointer) {
                        auto &runtime = *static_cast<facebook::jsi::Runtime *>(runtime_pointer);
                        supernote_reject_operation(
                            runtime, operation_id, "RESOURCE_EXHAUSTED",
                            "Supernote worker queue is full");
                      });
                }
                return Value::undefined();
              });
          auto promise = runtime.global().getPropertyAsFunction(runtime, "Promise");
          const Value executor_argument(std::move(executor));
          return promise.callAsConstructor(
              runtime, &executor_argument, static_cast<std::size_t>(1));
        } catch (const facebook::jsi::JSError &) {
          throw;
        } catch (const supernote::conversion::Failure &failure) {
          supernote_module_throw_conversion_failure(runtime, failure);
        } catch (const std::exception &error) {
          supernote_throw_error(
              runtime, "INTERNAL", std::string("Snfiletools.testTotalPathUnchanged: ") + error.what());
        }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathUnchanged.accepts"),
        3,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 3) {
            return Value(false);
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testTotalPathUnchanged.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testTotalPathUnchanged.argument[1](referencePath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.testTotalPathUnchanged.argument[2](pageKey)", 1);
            return Value(true);
          } catch (const facebook::jsi::JSError &error) {
            return Value(false);
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            return Value(false);
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }),
        Function::createFromHostFunction(
        runtime, PropNameID::forAscii(runtime, "testTotalPathUnchanged.checkArguments"),
        3,
        [](facebook::jsi::Runtime &runtime, const Value &,
           const Value *arguments, std::size_t argument_count) -> Value {
          if (argument_count != 3) {
            auto error = supernote_make_builtin_error(
                runtime, "TypeError",
                "Snfiletools.testTotalPathUnchanged: wrong argument count",
                "ARITY_MISMATCH", "Snfiletools.testTotalPathUnchanged",
                "3 arguments",
                std::to_string(argument_count) + " arguments");
            return supernote_validation_failure(runtime, std::move(error));
          }
          try {
            [[maybe_unused]] supernote::conversion::Budget conversion_budget;
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[0], conversion_budget,
                "Snfiletools.testTotalPathUnchanged.argument[0](path)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[1], conversion_budget,
                "Snfiletools.testTotalPathUnchanged.argument[1](referencePath)", 1);
            supernote_validate_js_5184f4bd6853(
                runtime, arguments[2], conversion_budget,
                "Snfiletools.testTotalPathUnchanged.argument[2](pageKey)", 1);
            return supernote_validation_success(runtime);
          } catch (const facebook::jsi::JSError &error) {
            return supernote_validation_failure(
                runtime, Value(runtime, error.value()));
          } catch (const supernote::conversion::Failure &failure) {
            if (failure.kind() == supernote::conversion::FailureKind::ALLOCATION) {
              supernote_throw_error(runtime, "RESOURCE_EXHAUSTED", failure.what());
            }
            const bool range =
                failure.kind() == supernote::conversion::FailureKind::RANGE;
            auto error = supernote_make_builtin_error(
                runtime, range ? "RangeError" : "TypeError", failure.what(),
                range ? "LIMIT_EXCEEDED" : "TYPE_MISMATCH",
                failure.path(), "within generated conversion limits", "rejected");
            return supernote_validation_failure(runtime, std::move(error));
          } catch (const std::exception &error) {
            supernote_throw_error(runtime, "INTERNAL", error.what());
          }
        }));
    exports.setProperty(runtime, "testTotalPathUnchanged", std::move(function));
  }
  feature_registry.setProperty(runtime, "supernote:feature:7e680f69f87879eb", std::move(exports));
}

}  // namespace supernote::generated::feature_7e680f69f87879eb

