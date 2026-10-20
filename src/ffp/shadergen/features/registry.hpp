#pragma once

#include "ffp/shadergen/features/base.hpp"
#include "common/log.hpp"

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace FFPE::Rendering::ShaderGen::Feature::Registry {

namespace Data {
    inline std::vector<std::unique_ptr<BaseFeature>> registeredFeatures;
    inline std::unordered_map<std::type_index, std::reference_wrapper<BaseFeature>> registeredFeaturesMap;
}

template <typename O, typename D>
concept DerivedFrom = std::is_base_of<D, O>::value;

template<DerivedFrom<BaseFeature> F>
inline void registerFeature() {
    LOGI("Registering feature : %s", typeid(F).name());
    
    Data::registeredFeaturesMap.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(typeid(F)),
        std::forward_as_tuple(std::ref(
            *Data::registeredFeatures.emplace_back(new F())
        ))
    );
}

template<DerivedFrom<BaseFeature> F>
inline F* getFeatureInstance() {
    auto it = Data::registeredFeaturesMap.find(typeid(F));
    if (it != Data::registeredFeaturesMap.end()) {
        return static_cast<F*>(&it->second.get());
    }
    return nullptr;
}

}
