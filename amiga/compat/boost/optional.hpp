// boost::optional, as far as Cannonball's config.cpp uses it (a value or
// none, tested and read with * ). Our own; not Boost.
// Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt).
#ifndef CB_COMPAT_OPTIONAL_HPP
#define CB_COMPAT_OPTIONAL_HPP

namespace boost {

template <class T> class optional {
public:
    optional() : set_(false), v_() {}
    optional(const T &v) : set_(true), v_(v) {}
    bool is_initialized() const { return set_; }
    explicit operator bool() const { return set_; }
    bool operator!() const { return !set_; }
    const T &operator*() const { return v_; }
    T &operator*() { return v_; }
    const T *operator->() const { return &v_; }
    const T &get() const { return v_; }
    const T &value() const { return v_; }
    T value_or(const T &d) const { return set_ ? v_ : d; }
private:
    bool set_;
    T v_;
};

} // namespace boost

#endif
