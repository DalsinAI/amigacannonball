// The part of Boost.PropertyTree Cannonball uses, our own: a tree of named
// nodes, each with a string value, read and written by dotted paths
// ("video.window.scale"); "<xmlattr>" children are an XML element's
// attributes (xml_parser.hpp). Values convert with the C++ streams, as
// Boost's do. Not Boost: only what config.cpp, menu.cpp and romloader.cpp
// call, so Cannonball builds without the Boost headers.
// Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt).
#ifndef CB_COMPAT_PTREE_HPP
#define CB_COMPAT_PTREE_HPP

#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <boost/optional.hpp>

namespace boost {
namespace property_tree {

struct ptree_bad_path : std::runtime_error {
    explicit ptree_bad_path(const std::string &p) : std::runtime_error("No such node (" + p + ")") {}
};
struct ptree_bad_data : std::runtime_error {
    explicit ptree_bad_data(const std::string &p) : std::runtime_error("conversion of data failed (" + p + ")") {}
};

namespace detail {
template <class T> inline bool from_string(const std::string &s, T &out)
{
    std::istringstream in(s);
    in >> out;
    if (in.fail())
        return false;
    in >> std::ws;
    return in.eof();
}
template <> inline bool from_string<std::string>(const std::string &s, std::string &out)
{
    out = s;
    return true;
}
template <> inline bool from_string<char>(const std::string &s, char &out)
{
    if (s.size() != 1)
        return false;
    out = s[0];
    return true;
}
template <> inline bool from_string<bool>(const std::string &s, bool &out)
{
    if (s == "true" || s == "1") { out = true; return true; }
    if (s == "false" || s == "0") { out = false; return true; }
    return false;
}
template <class T> inline std::string to_string(const T &v)
{
    std::ostringstream o;
    o.precision(9);
    o << v;
    return o.str();
}
inline std::string to_string(const std::string &v) { return v; }
inline std::string to_string(const char *v) { return v; }
inline std::string to_string(char v) { return std::string(1, v); }
inline std::string to_string(bool v) { return v ? "true" : "false"; }
} // namespace detail

class ptree {
public:
    typedef std::pair<std::string, ptree> value_type;
    typedef std::vector<value_type>::iterator iterator;
    typedef std::vector<value_type>::const_iterator const_iterator;

    const std::string &data() const { return data_; }
    std::string &data() { return data_; }
    bool empty() const { return children_.empty(); }
    std::size_t size() const { return children_.size(); }
    iterator begin() { return children_.begin(); }
    iterator end() { return children_.end(); }
    const_iterator begin() const { return children_.begin(); }
    const_iterator end() const { return children_.end(); }
    void clear() { data_.clear(); children_.clear(); }

    // A child by its name, added at the end
    ptree &push_back_child(const std::string &key)
    {
        children_.push_back(value_type(key, ptree()));
        return children_.back().second;
    }

    // The node at a dotted path, or 0
    const ptree *find(const std::string &path) const
    {
        const ptree *n = this;
        std::size_t at = 0;
        for (;;) {
            std::size_t dot = path.find('.', at);
            std::string key = path.substr(at, dot == std::string::npos ? std::string::npos : dot - at);
            const ptree *next = 0;
            for (const_iterator i = n->begin(); i != n->end(); ++i)
                if (i->first == key) { next = &i->second; break; }
            if (!next)
                return 0;
            n = next;
            if (dot == std::string::npos)
                return n;
            at = dot + 1;
        }
    }

    // The node at a dotted path, made where it is missing (the first of a
    // name is the one taken, as Boost's put does)
    ptree &walk(const std::string &path)
    {
        ptree *n = this;
        std::size_t at = 0;
        for (;;) {
            std::size_t dot = path.find('.', at);
            std::string key = path.substr(at, dot == std::string::npos ? std::string::npos : dot - at);
            ptree *next = 0;
            for (iterator i = n->begin(); i != n->end(); ++i)
                if (i->first == key) { next = &i->second; break; }
            if (!next)
                next = &n->push_back_child(key);
            n = next;
            if (dot == std::string::npos)
                return *n;
            at = dot + 1;
        }
    }

    const ptree &get_child(const std::string &path) const
    {
        const ptree *n = find(path);
        if (!n)
            throw ptree_bad_path(path);
        return *n;
    }

    template <class T> boost::optional<T> get_optional(const std::string &path) const
    {
        const ptree *n = find(path);
        T v;
        if (n && detail::from_string(n->data_, v))
            return boost::optional<T>(v);
        return boost::optional<T>();
    }

    template <class T> T get(const std::string &path) const
    {
        const ptree *n = find(path);
        if (!n)
            throw ptree_bad_path(path);
        T v;
        if (!detail::from_string(n->data_, v))
            throw ptree_bad_data(path);
        return v;
    }

    template <class T> T get(const std::string &path, const T &def) const
    {
        boost::optional<T> v = get_optional<T>(path);
        return v ? *v : def;
    }

    std::string get(const std::string &path, const char *def) const
    {
        return get<std::string>(path, std::string(def));
    }

    template <class T> ptree &put(const std::string &path, const T &value)
    {
        ptree &n = walk(path);
        n.data_ = detail::to_string(value);
        return n;
    }

private:
    std::string data_;
    std::vector<value_type> children_;
};

} // namespace property_tree
} // namespace boost

#endif
