#include <jsi/jsi.h>

#include "runtime_services.hpp"

#include <cstddef>
#include <string>
#include <utility>

namespace supernote::generated::feature_7e680f69f87879eb {
void register_feature(facebook::jsi::Runtime &runtime,
                      facebook::jsi::Object &feature_registry,
                      const std::shared_ptr<
                          supernote::runtime::FeatureSession> &feature_session);
}
namespace supernote::generated::jvm_feature_7e680f69f87879eb {
void register_jvm_feature(facebook::jsi::Runtime &runtime,
                          facebook::jsi::Object &feature_registry,
                          const std::shared_ptr<
                              supernote::runtime::FeatureSession> &feature_session);
}

namespace supernote::generated {
namespace {

constexpr char kFeatureRegistryGlobal[] =
    "__supernoteModuleFeatureRegistry_63f6999c8c67";

[[noreturn]] void throw_type_error(
    facebook::jsi::Runtime &runtime, const std::string &message) {
  auto constructor =
      runtime.global().getPropertyAsFunction(runtime, "TypeError");
  throw facebook::jsi::JSError(
      runtime, constructor.callAsConstructor(runtime, message));
}

}  // namespace

void install_plugin_bindings(
    facebook::jsi::Runtime &runtime,
    const std::shared_ptr<supernote::runtime::RuntimeSession> &runtime_session) {
  using facebook::jsi::Function;
  using facebook::jsi::Object;
  using facebook::jsi::PropNameID;
  using facebook::jsi::Value;

  Object features(runtime);
  {
    auto feature_session = supernote::runtime::FeatureSession::create(
        runtime_session, supernote::runtime::process_services().cleanup());
    feature_7e680f69f87879eb::register_feature(
        runtime, features, feature_session);
    jvm_feature_7e680f69f87879eb::register_jvm_feature(
        runtime, features, feature_session);
  }
  runtime.global().setProperty(
      runtime, kFeatureRegistryGlobal, std::move(features));

  Object public_runtime(runtime);
  auto feature = Function::createFromHostFunction(
      runtime,
      PropNameID::forAscii(runtime, "feature"),
      1,
      [](facebook::jsi::Runtime &runtime,
         const Value &,
         const Value *arguments,
         std::size_t argument_count) -> Value {
        if (argument_count != 1 || !arguments[0].isString()) {
          throw_type_error(
              runtime,
              "Supernote generated runtime feature(id) expects exactly one string");
        }
        const auto feature_id = arguments[0].asString(runtime).utf8(runtime);
        auto registry = runtime.global().getPropertyAsObject(
            runtime, kFeatureRegistryGlobal);
        auto binding = registry.getProperty(runtime, feature_id.c_str());
        if (binding.isUndefined()) {
          throw_type_error(
              runtime, "unknown Supernote generated feature: " + feature_id);
        }
        return binding;
      });
  public_runtime.setProperty(runtime, "feature", std::move(feature));
  runtime.global().setProperty(
      runtime, "__supernoteModule", std::move(public_runtime));
}

}  // namespace supernote::generated
