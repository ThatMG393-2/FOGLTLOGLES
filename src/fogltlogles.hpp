#pragma once

#include "common/types.hpp"

#include <cstddef>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#ifndef TO_FUNCPTR
#define TO_FUNCPTR(func) reinterpret_cast<FunctionPtr>(func)
#endif

namespace FOGLTLOGLES {
    struct Entry {
        FunctionPtr base = nullptr;
        FunctionPtr exposed = nullptr;
        bool hooked = false;
    };

    Entry& entryFor(const std::string& name);
    void provide(const std::string& name, FunctionPtr function);
    FunctionPtr getFunctionAddress(std::string);

    void init();

    template <auto Key>
    struct Chain;

    template <typename R, typename... A, R (*Key)(A...)>
    struct Chain<Key> {
        struct Next {
            std::size_t i;
            R operator()(A... a) const { return run(i, a...); }
        };

        using Hook = std::function<R(Next, A...)>;

        template <typename F>
        static constexpr bool middleware = std::is_invocable_v<F, Next, A...>;

        static inline Entry* entry = nullptr;
        static inline std::vector<Hook> hooks;

        static R run(std::size_t i, A... a) {
            if (i < hooks.size()) return hooks[i](Next{ i + 1 }, a...);
            if (!entry || !entry->base) return R();
            return reinterpret_cast<R (*)(A...)>(entry->base)(a...);
        }

        static R dispatch(A... a) { return run(0, a...); }
    };

    template <auto Key, typename F>
    void addHook(const char* name, F f) {
        using C = Chain<Key>;
        Entry& e = entryFor(name);

        C::entry = &e;
        if constexpr (C::template middleware<F>) {
            C::hooks.emplace_back(std::move(f));
        } else {
            C::hooks.emplace_back([f = std::move(f)](typename C::Next next, auto... a) {
                f(a...);
                return next(a...);
            });
        }

        e.hooked = true;
        e.exposed = TO_FUNCPTR(&C::dispatch);
    }
}

#define PROVIDE_PICK_(_1, _2, N, ...) N
#define PROVIDE_1_(f) FOGLTLOGLES::provide(#f, TO_FUNCPTR(f))
#define PROVIDE_2_(f, t) FOGLTLOGLES::provide(#f, TO_FUNCPTR(t))
#define PROVIDE(...) PROVIDE_PICK_(__VA_ARGS__, PROVIDE_2_, PROVIDE_1_, _)(__VA_ARGS__)

#define HOOK(f, fn) FOGLTLOGLES::addHook<&f>(#f, fn)

#ifndef GET_OVFUNC
#define GET_OVFUNC(type, name) reinterpret_cast<type>(FOGLTLOGLES::getFunctionAddress(#name))
#endif
