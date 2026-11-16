// read_xml and write_xml for ptree.hpp, as Cannonball's config.cpp uses
// them for config.xml and the score files. Our own small XML reader and
// writer, not Boost's: elements become children, their text the value,
// their attributes children of "<xmlattr>"; comments, the <?xml ?> line and
// <!DOCTYPE> are skipped. The five standard entities and &#NN; / &#xNN; are
// read; &, <, >, " are written as entities. Several top-level elements are
// allowed (config.xml has several), as Boost's reader allows them.
// Copyright (c) 2026 AmiCradle Team. Cannonball's licence (docs/license.txt).
#ifndef CB_COMPAT_XML_PARSER_HPP
#define CB_COMPAT_XML_PARSER_HPP

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <boost/property_tree/ptree.hpp>

namespace boost {
namespace property_tree {

struct xml_parser_error : std::runtime_error {
    xml_parser_error(const std::string &what, const std::string &file, int line)
        : std::runtime_error(file + "(" + detail::to_string(line) + "): " + what) {}
};

template <class S> struct xml_writer_settings {
    xml_writer_settings(char indent_char = ' ', std::size_t indent_count = 0)
        : indent_char(indent_char), indent_count(indent_count) {}
    char indent_char;
    std::size_t indent_count;
};

namespace xml_parser {
static const int no_concat_text = 1, no_comments = 2, trim_whitespace = 4;

class reader {
public:
    reader(const std::string &text, const std::string &file, int flags)
        : s_(text), p_(0), file_(file), flags_(flags) {}

    void parse(ptree &root)
    {
        for (;;) {
            skip_misc();
            if (p_ >= s_.size())
                return;
            if (s_[p_] != '<')
                fail("text outside an element");
            element(root);
        }
    }

private:
    const std::string &s_;
    std::size_t p_;
    std::string file_;
    int flags_;

    int line() const
    {
        int n = 1;
        for (std::size_t i = 0; i < p_ && i < s_.size(); i++)
            n += s_[i] == '\n';
        return n;
    }
    void fail(const std::string &what) const { throw xml_parser_error(what, file_, line()); }
    bool at(const char *w) const { return s_.compare(p_, std::string(w).size(), w) == 0; }
    void skip_ws() { while (p_ < s_.size() && std::isspace((unsigned char)s_[p_])) p_++; }
    void skip_past(const char *end)
    {
        std::size_t e = s_.find(end, p_);
        if (e == std::string::npos)
            fail(std::string("no ") + end);
        p_ = e + std::string(end).size();
    }
    // whitespace, comments, <?...?> and <!DOCTYPE ...>
    void skip_misc()
    {
        for (;;) {
            skip_ws();
            if (at("<!--")) skip_past("-->");
            else if (at("<?")) skip_past("?>");
            else if (at("<!DOCTYPE")) skip_past(">");
            else return;
        }
    }
    std::string name()
    {
        std::size_t b = p_;
        while (p_ < s_.size() && (std::isalnum((unsigned char)s_[p_]) || s_[p_] == '_' ||
                                   s_[p_] == '-' || s_[p_] == '.' || s_[p_] == ':'))
            p_++;
        if (b == p_)
            fail("a name was expected");
        return s_.substr(b, p_ - b);
    }
    std::string decode(const std::string &t) const
    {
        std::string o;
        for (std::size_t i = 0; i < t.size(); i++) {
            if (t[i] != '&') { o += t[i]; continue; }
            std::size_t e = t.find(';', i);
            if (e == std::string::npos)
                fail("an & without ;");
            std::string ent = t.substr(i + 1, e - i - 1);
            if (ent == "lt") o += '<';
            else if (ent == "gt") o += '>';
            else if (ent == "amp") o += '&';
            else if (ent == "quot") o += '"';
            else if (ent == "apos") o += '\'';
            else if (ent.size() > 1 && ent[0] == '#') {
                unsigned long c = ent[1] == 'x' ? std::strtoul(ent.c_str() + 2, 0, 16)
                                                : std::strtoul(ent.c_str() + 1, 0, 10);
                if (c < 0x80) o += (char)c;
                else if (c < 0x800) { o += (char)(0xC0 | (c >> 6)); o += (char)(0x80 | (c & 0x3F)); }
                else { o += (char)(0xE0 | (c >> 12)); o += (char)(0x80 | ((c >> 6) & 0x3F)); o += (char)(0x80 | (c & 0x3F)); }
            } else
                fail("unknown entity &" + ent + ";");
            i = e;
        }
        return o;
    }
    static std::string trim(const std::string &t)
    {
        std::size_t b = 0, e = t.size();
        while (b < e && std::isspace((unsigned char)t[b])) b++;
        while (e > b && std::isspace((unsigned char)t[e - 1])) e--;
        return t.substr(b, e - b);
    }
    void element(ptree &parent)
    {
        p_++;                                    // <
        std::string tag = name();
        ptree &node = parent.push_back_child(tag);
        for (;;) {                               // attributes
            skip_ws();
            if (p_ >= s_.size())
                fail("<" + tag + " is not closed");
            if (s_[p_] == '/' || s_[p_] == '>')
                break;
            std::string an = name();
            skip_ws();
            if (p_ >= s_.size() || s_[p_] != '=')
                fail("= expected after " + an);
            p_++;
            skip_ws();
            char q = p_ < s_.size() ? s_[p_] : 0;
            if (q != '"' && q != '\'')
                fail("a quoted value expected for " + an);
            std::size_t e = s_.find(q, p_ + 1);
            if (e == std::string::npos)
                fail("an attribute value is not closed");
            node.walk("<xmlattr>").push_back_child(an).data() = decode(s_.substr(p_ + 1, e - p_ - 1));
            p_ = e + 1;
        }
        if (s_[p_] == '/') {                     // <tag/>
            if (p_ + 1 >= s_.size() || s_[p_ + 1] != '>')
                fail("/> expected");
            p_ += 2;
            return;
        }
        p_++;                                    // >
        std::string text;
        for (;;) {
            if (p_ >= s_.size())
                fail("</" + tag + "> is missing");
            if (at("<!--")) { skip_past("-->"); continue; }
            if (at("<![CDATA[")) {
                std::size_t e = s_.find("]]>", p_);
                if (e == std::string::npos)
                    fail("CDATA is not closed");
                text += s_.substr(p_ + 9, e - p_ - 9);
                p_ = e + 3;
                continue;
            }
            if (at("</")) {
                p_ += 2;
                std::string end = name();
                if (end != tag)
                    fail("</" + end + "> closes <" + tag + ">");
                skip_ws();
                if (p_ >= s_.size() || s_[p_] != '>')
                    fail("> expected");
                p_++;
                break;
            }
            if (s_[p_] == '<') { element(node); continue; }
            std::size_t e = s_.find('<', p_);
            if (e == std::string::npos)
                e = s_.size();
            text += decode(s_.substr(p_, e - p_));
            p_ = e;
        }
        node.data() = (flags_ & trim_whitespace) ? trim(text) : text;
    }
};

inline std::string encode(const std::string &t, bool attr)
{
    std::string o;
    for (std::size_t i = 0; i < t.size(); i++) {
        switch (t[i]) {
        case '&': o += "&amp;"; break;
        case '<': o += "&lt;"; break;
        case '>': o += "&gt;"; break;
        case '"': if (attr) { o += "&quot;"; break; } /* fall through */
        default: o += t[i];
        }
    }
    return o;
}

template <class S>
inline void write_node(std::ostream &o, const std::string &tag, const ptree &n,
                       int depth, const xml_writer_settings<S> &ws)
{
    std::string pad(depth * ws.indent_count, ws.indent_char);
    o << pad << '<' << tag;
    const ptree *attrs = n.find("<xmlattr>");
    if (attrs)
        for (ptree::const_iterator a = attrs->begin(); a != attrs->end(); ++a)
            o << ' ' << a->first << "=\"" << encode(a->second.data(), true) << '"';
    bool kids = false;
    for (ptree::const_iterator c = n.begin(); c != n.end(); ++c)
        if (c->first != "<xmlattr>") { kids = true; break; }
    if (!kids && n.data().empty()) {
        o << "/>\n";
        return;
    }
    o << '>' << encode(n.data(), false);
    if (kids) {
        o << '\n';
        for (ptree::const_iterator c = n.begin(); c != n.end(); ++c)
            if (c->first != "<xmlattr>")
                write_node(o, c->first, c->second, depth + 1, ws);
        o << pad;
    }
    o << "</" << tag << ">\n";
}
} // namespace xml_parser

inline void read_xml(std::istream &in, ptree &pt, int flags = 0, const std::string &file = "<stream>")
{
    std::ostringstream all;
    all << in.rdbuf();
    std::string text = all.str();
    ptree fresh;
    xml_parser::reader(text, file, flags).parse(fresh);
    pt = fresh;
}

inline void read_xml(const std::string &file, ptree &pt, int flags = 0)
{
    std::ifstream in(file.c_str(), std::ios::in | std::ios::binary);
    if (!in)
        throw xml_parser_error("cannot open file", file, 0);
    read_xml(in, pt, flags, file);
}

template <class S>
inline void write_xml(std::ostream &o, const ptree &pt,
                      const xml_writer_settings<S> &ws = xml_writer_settings<S>())
{
    o << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    for (ptree::const_iterator c = pt.begin(); c != pt.end(); ++c)
        xml_parser::write_node(o, c->first, c->second, 0, ws);
}

template <class S>
inline void write_xml(const std::string &file, const ptree &pt, const std::locale &,
                      const xml_writer_settings<S> &ws)
{
    std::ofstream o(file.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
    if (!o)
        throw xml_parser_error("cannot write file", file, 0);
    write_xml(o, pt, ws);
    o.flush();
    if (!o)
        throw xml_parser_error("write error", file, 0);
}

inline void write_xml(const std::string &file, const ptree &pt)
{
    write_xml(file, pt, std::locale(), xml_writer_settings<std::string>());
}

} // namespace property_tree
} // namespace boost

#endif
