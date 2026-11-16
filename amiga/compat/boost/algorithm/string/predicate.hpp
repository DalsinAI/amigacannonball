// boost::starts_with and boost::ends_with on std::string, as Cannonball's
// menu.cpp and config.cpp use them. Our own; not Boost.
// Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt).
#ifndef CB_COMPAT_PREDICATE_HPP
#define CB_COMPAT_PREDICATE_HPP
#include <algorithm>   // config.cpp uses std::transform, which Boost brings in
#include <string>

namespace boost {
namespace algorithm {
inline bool starts_with(const std::string &s, const std::string &p)
{
    return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}
inline bool ends_with(const std::string &s, const std::string &p)
{
    return s.size() >= p.size() && s.compare(s.size() - p.size(), p.size(), p) == 0;
}
} // namespace algorithm
using algorithm::starts_with;
using algorithm::ends_with;
} // namespace boost
#endif
