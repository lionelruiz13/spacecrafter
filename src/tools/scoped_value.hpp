#ifndef SCOPED_VALUE_HPP
#define SCOPED_VALUE_HPP

#include <utility> // std::move
#include <type_traits>

/**
 * @brief RAII utility to save and restore (when going out of scope) a value using getter and setter functions.
 *
 * @tparam Getter A callable type that returns the current value.
 * @tparam Setter A callable type that sets a new value.
 */
template <class Getter, class Setter>
class ScopedValue {
public:
    using Value = std::decay_t<decltype(std::declval<Getter&>()())>; // Determine the type to store based on the return type of the getter

    ScopedValue(Getter getter, Setter setter)
        : _getter(std::move(getter)), _setter(std::move(setter)), _saved(_getter()) {}

    ScopedValue(const ScopedValue&) = delete;
    ScopedValue& operator=(const ScopedValue&) = delete;

    ScopedValue(ScopedValue&& other) noexcept
        : _getter(std::move(other._getter)),
          _setter(std::move(other._setter)),
          _saved(std::move(other._saved)),
          _active(std::exchange(other._active, false)) {}

    ~ScopedValue() {
        if (_active) _setter(_saved);
    }

    const Value& saved() const { return _saved; }

    Value get() const { return _getter(); }

    void set(Value v) { _setter(std::move(v)); }

    void dismiss() { _active = false; } // If we want to keep the new value and not restore the old one, we can call dismiss()

private:
    Getter _getter;
    Setter _setter;
    Value  _saved;
    bool   _active = true;
};


#endif // SCOPED_VALUE_HPP