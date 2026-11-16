// BOOST_STATIC_ASSERT_MSG, which Cannonball's stdint.hpp uses: C++11's
// static_assert. Our own; not Boost.
// Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt).
#ifndef CB_COMPAT_STATIC_ASSERT_HPP
#define CB_COMPAT_STATIC_ASSERT_HPP
#define BOOST_STATIC_ASSERT_MSG(c, msg) static_assert(c, msg)
#define BOOST_STATIC_ASSERT(c) static_assert(c, #c)
#endif
