#include "fwd.hpp"
#include "boostbook.hpp"
#include "phrase.hpp"
#include "actions_class.hpp"
#include "utils.hpp"

namespace quickbook
{
    template <typename Iter>
    std::string encode(Iter first, Iter last)
    {
        std::string r;
        
        for(;first != last; ++first) {
            switch (*first)
            {
                case '<': r += "&lt;";    break;
                case '>': r += "&gt;";    break;
                case '&': r += "&amp;";   break;
                case '"': r += "&quot;";  break;
                default:  r += *first;    break;
            }
        }
        
        return r;
    }

    std::string encode(std::string const& x) {
        return encode(x.begin(), x.end());
    }

    std::string encode(char const* x) {
        char const* end = x;
        while(*end) ++end;
        return encode(x, end);
    }

    std::string encode(char c) {
        return encode(&c, &c + 1);
    }

    namespace {
        struct boostbook_markup {
            char const* quickbook;
            char const* pre;
            char const* post;
        };
    
        boostbook_markup markups[] = {
            { "", "", "" },
            { "comment", "<!--", "-->" },
            { "paragraph", "<para>\n", "</para>\n" },
            { "h1", "<bridgehead renderas=\"sect1\">", "</bridgehead>" },
            { "h2", "<bridgehead renderas=\"sect2\">", "</bridgehead>" },
            { "h3", "<bridgehead renderas=\"sect3\">", "</bridgehead>" },
            { "h4", "<bridgehead renderas=\"sect4\">", "</bridgehead>" },
            { "h5", "<bridgehead renderas=\"sect5\">", "</bridgehead>" },
            { "h6", "<bridgehead renderas=\"sect6\">", "</bridgehead>" },
            { "blurb", "<sidebar role=\"blurb\">\n", "</sidebar>\n" },
            { "blockquote", "<blockquote><para>", "</para></blockquote>" },
            { "preformatted", "<programlisting>", "</programlisting>" },
            { "warning", "<warning>", "</warning>" },
            { "caution", "<caution>", "</caution>" },
            { "important", "<important>", "</important>" },
            { "note", "<note>", "</note>" },
            { "tip", "<tip>", "</tip>" },
            { "list_item", "<listitem>\n", "\n</listitem>" },
            { "bold", "<emphasis role=\"bold\">", "</emphasis>" },
            { "italic", "<emphasis>", "</emphasis>" },
            { "underline", "<emphasis role=\"underline\">", "</emphasis>" },
            { "teletype", "<literal>", "</literal>" },
            { "strikethrough", "<emphasis role=\"strikethrough\">", "</emphasis>" },
            { "quote", "<quote>", "</quote>" },
            { "url", "<ulink url=\"", "</ulink>" },
            { "link", "<link linkend=\"", "</link>" },
            { "funcref", "<functionname alt=\"", "</functionname>" },
            { "classref", "<classname alt=\"", "</classname>" },
            { "memberref", "<methodname alt=\"", "</methodname>" },
            { "enumref", "<enumname alt=\"", "</enumname>" },
            { "macroref", "<macroname alt=\"", "</macroname>" },
            { "headerref", "<headername alt=\"", "</headername>" },
            { "conceptref", "<conceptname alt=\"", "</conceptname>" },
            { "globalref", "<globalname alt=\"", "</globalname>" },
            { "footnote", "<footnote><para>", "</para></footnote>" },
            { "escape", "<!--quickbook-escape-prefix-->", "<!--quickbook-escape-postfix-->" },
            { "replaceable", "<replaceable>", "</replaceable>" },
            { "varlistentry", "<varlistentry>", "</varlistentry>\n" },
            { "varlistterm", "<term>", "</term>" },
            { "varlistitem", "<listitem>", "</listitem>" },
            { "header", "<thead>", "</thead>\n" },
            { "row", "<row>", "</row>\n" },
            { "cell", "<entry>", "</entry>" },
            { "programlisting", "<programlisting>", "</programlisting>\n" },
            { "code", "<code>", "</code>" },
            { "hr", "<para/>", "" },
            { "break", "<sbr/>", "" },
        };
        
        std::map<std::string, boostbook_markup> markup_map;
        
        struct initialize {
            initialize() {
                BOOST_FOREACH(boostbook_markup m, markups) {
                    markup_map[m.quickbook] = m;
                }
            }
        } initialize_instance;
    }

    void output(quickbook::actions& actions, std::string const& x)
    {
        actions.phrase << x;
    }

    void output(quickbook::actions& actions, anchor const& x) {
        actions.phrase << "<anchor id=\"";
        detail::print_string(x.id, actions.phrase.get());
        actions.phrase << "\" />\n";
    }

    void output(quickbook::actions& actions, link const& x) {
        boostbook_markup m = markup_map.at(x.type);
        actions.phrase << m.pre;
        detail::print_string(x.destination, actions.phrase.get());
        actions.phrase << "\">";
        actions.phrase << x.content;
        actions.phrase << m.post;
    }

    void output(quickbook::actions& actions, formatted const& x) {
        boostbook_markup m = markup_map.at(x.type);
        actions.phrase << m.pre << x.content << m.post;
    }

    void output(quickbook::actions& actions, break_ const& x) {
        boostbook_markup m = markup_map.at("break");
        actions.phrase << m.pre;
    }
}
