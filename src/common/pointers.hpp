#pragma once

#include <memory>

template<typename T, typename... Args>
inline std::shared_ptr<T> MakeAggregateShared(Args&&... args) {
    if constexpr (std::is_aggregate_v<T>) {
        // aggregate initialization
        return std::make_shared<T>(T{std::forward<Args>(args)...});
    } else {
        // plain-ol constructor
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}
