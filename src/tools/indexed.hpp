#pragma once
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

/**
* @brief A class to manage multiple objects (unique_ptr) of type T identified by an enum value.
* @tparam T The type of objects to manage.
* @tparam Enum The enum type used to identify the objects.
* @note Enum must have continuous values starting from 0 and a Count value (last value of the enum and not used).
* @note Something like: `enum class MyEnum { Value1, Value2, Count };` (Value1 = 0, Value2 = 1, Count = 2, Count should never be used directly).
*/
template <class T, class Enum>
class Indexed {
public:
    static_assert(std::is_enum_v<Enum>, "Enum must be an enum type");
    static_assert(static_cast<std::size_t>(Enum::Count) > 0, "Enum must have at least one value");

    static constexpr std::size_t N = static_cast<std::size_t>(Enum::Count);

    Indexed() = default;

    using index_type = std::size_t;

    // Set a specific object
    void set(Enum e, std::unique_ptr<T> p) {
        const auto id = idx(e);
        assert(id < N);
        items_[id] = std::move(p);
    }

    // Set the active object
    void set(std::unique_ptr<T> p) {
        const auto id = idx(active_);
        assert(id < N);
        items_[id] = std::move(p);
    }

    //! ///////////////////////////////////////////////////////////////////////

    // Define the active object
    void setActive(Enum e) {
        assert(idx(e) < N);
        active_ = e;
    }

    // Get the active object enum value
    Enum getActive() const {
        return active_;
    }

    //! ///////////////////////////////////////////////////////////////////////

    // Access to a specific object
    T *get(Enum e) {
        const auto id = idx(e);
        assert(id < N);
        return items_[id].get();
    }

    // Access to a specific object (const)
    T *get(Enum e) const {
        const auto id = idx(e);
        assert(id < N);
        return items_[id].get();
    }

    // Access to the active object
    T *get() {
        const auto id = idx(active_);
        assert(id < N);
        return items_[id].get();
    }

    // Access to the active object (const)
    T *get() const {
        const auto id = idx(active_);
        assert(id < N);
        return items_[id].get();
    }

    //! ///////////////////////////////////////////////////////////////////////

    // Check if the active object is valid
    explicit operator bool() const { return get() != nullptr; }

    //! ///////////////////////////////////////////////////////////////////////

    // unique_ptr-like access to the ACTIVE object
    T *operator->() {
        auto *p = get();
        assert(p && "Indexed: active object is null");
        return p;
    }

    // unique_ptr-like access to the ACTIVE object
    T *operator->() const {
        auto *p = get();
        assert(p && "Indexed: active object is null");
        return p;
    }

    // unique_ptr-like access to the ACTIVE object
    T &operator*() {
        auto *p = get();
        assert(p && "Indexed: active object is null");
        return *p;
    }

    // unique_ptr-like access to the ACTIVE object
    T &operator*() const {
        auto *p = get();
        assert(p && "Indexed: active object is null");
        return *p;
    }

    //! ///////////////////////////////////////////////////////////////////////

    // Apply a lambda function to all objects
    template <class F>
    void applyToAll(F &&f) {
        for (auto &up : items_) {
            if (up) {
                std::forward<F>(f)(*up);
            }
        }
    }

    // Apply a lambda function to all objects (const)
    template <class F>
    void applyToAll(F &&f) const {
        for (auto &up : items_) {
            if (up) {
                std::forward<F>(f)(*up);
            }
        }
    }

    // Apply a member function to all objects
    template <class Method, class... Args>
    requires std::is_member_function_pointer_v<std::remove_reference_t<Method>>
    void applyToAll(Method method, Args&&... args) {
        for (auto &up : items_) {
            if (up) {
                (up.get()->*method)(std::forward<Args>(args)...);
            }
        }
    }

    // Apply a member function to all objects (const)
    template <class Method, class... Args>
    requires std::is_member_function_pointer_v<std::remove_reference_t<Method>>
    void applyToAll(Method method, Args&&... args) const {
        for (auto &up : items_) {
            if (up) {
                (up.get()->*method)(std::forward<Args>(args)...);
            }
        }
    }

    //! ///////////////////////////////////////////////////////////////////////

    // Apply a lambda function to the active object
    template <class F>
    void applyToActive(F &&f) {
        if (auto *p = get()) {
            std::forward<F>(f)(*p);
        }
    }

    // Apply a lambda function to the active object (const)
    template <class F>
    void applyToActive(F &&f) const {
        if (auto *p = get()) {
            std::forward<F>(f)(*p);
        }
    }

    // Apply a member function to the active object
    template <class Method, class... Args>
    requires std::is_member_function_pointer_v<std::remove_reference_t<Method>>
    void applyToActive(Method method, Args&&... args) {
        if (auto* p = get()) {
            (p->*method)(std::forward<Args>(args)...);
        }
    }

    // Apply a member function to the active object (const)
    template <class Method, class... Args>
    requires std::is_member_function_pointer_v<std::remove_reference_t<Method>>
    void applyToActive(Method method, Args&&... args) const {
        if (auto* p = get()) {
            (p->*method)(std::forward<Args>(args)...);
        }
    }

    //! ///////////////////////////////////////////////////////////////////////

    // Apply a lambda function to a specific object
    template <class F>
    void applyTo(Enum e, F &&f) {
        if (auto *p = get(e)) {
            std::forward<F>(f)(*p);
        }
    }

    // Apply a lambda function to a specific object (const)
    template <class F>
    void applyTo(Enum e, F &&f) const {
        if (auto *p = get(e)) {
            std::forward<F>(f)(*p);
        }
    }

    // Apply a member function to a specific object
    template <class Method, class... Args>
    requires std::is_member_function_pointer_v<std::remove_reference_t<Method>>
    void applyTo(Enum e, Method method, Args&&... args) {
        if (auto* p = get(e)) {
            (p->*method)(std::forward<Args>(args)...);
        }
    }

    // Apply a member function to a specific object (const)
    template <class Method, class... Args>
    requires std::is_member_function_pointer_v<std::remove_reference_t<Method>>
    void applyTo(Enum e, Method method, Args&&... args) const {
        if (auto* p = get(e)) {
            (p->*method)(std::forward<Args>(args)...);
        }
    }

private:
    static constexpr std::size_t idx(Enum e) {
        return static_cast<std::size_t>(e);
    }

    std::array<std::unique_ptr<T>, N> items_{};
    Enum active_ = static_cast<Enum>(0);
};
