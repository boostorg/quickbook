#include "fwd.hpp"
#include "boostbook.hpp"
#include "phrase.hpp"
#include "actions_class.hpp"
#include <algorithm>

namespace quickbook
{
    struct output_action
    {
        output_action(quickbook::actions& actions) : actions(actions) {}    
        quickbook::actions& actions;

        template <typename T>
        void operator()(T const& x) const {
            output(actions, x);
        }
    };

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
        actions.phrase << encode(x.id);
        actions.phrase << "\"/>\n";
    }

    void output(quickbook::actions& actions, link const& x) {
        boostbook_markup m = markup_map.at(x.type);
        actions.phrase << m.pre;
        actions.phrase << encode(x.destination);
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

    void output(quickbook::actions& actions, image2 const& x) {
        actions.phrase << "<inlinemediaobject>";

        actions.phrase << "<imageobject><imagedata";
        
        for(image2::attribute_map::const_iterator
            attr_first = x.attributes.begin(), attr_last  = x.attributes.end();
            attr_first != attr_last; ++attr_first)
        {
            if(attr_first->first == "alt") continue;
        
            actions.phrase
                << " "
                << attr_first->first
                << "=\""
                << encode(attr_first->second)
                << "\"";
        }

        actions.phrase << "></imagedata></imageobject>";

        attribute_map::const_iterator it = x.attributes.find("alt");
        if(it != x.attributes.end()) {
            // Also add a textobject -- use the basename of the image file.
            // This will mean we get "alt" attributes of the HTML img.
            actions.phrase << "<textobject><phrase>";
            actions.phrase << encode(it->second);
            actions.phrase << "</phrase></textobject>";
        }

        actions.phrase << "</inlinemediaobject>";
    }

    void output(quickbook::actions& actions, hr) {
        actions.phrase << markup_map.at("hr").pre;
    }

    void output(quickbook::actions& actions, begin_section2 const& x) {
        actions.phrase << "\n<section id=\"" << x.id << "\">\n";
        if(x.linkend.empty()) {
            actions.phrase
                << "<title>"
                << x.content
                << "</title>\n"
                ;
        }
        else {
            actions.phrase
                << "<title>"
                << "<link linkend=\""
                << x.linkend
                << "\">"
                << x.content
                << "</link>"
                << "</title>\n"
                ;
        }
    }

    void output(quickbook::actions& actions, end_section2 const& x) {
        actions.phrase << "</section>";
    }

    void output(quickbook::actions& actions, heading2 const& x) {
        actions.phrase
            << "<anchor id=\"" << x.id << "\"/>"
            << "<bridgehead renderas=\"sect" << x.level << "\">";

        if(x.linkend.empty()) {
            actions.phrase << x.content;
        }
        else {
            actions.phrase
                << "<link linkend=\"" << x.linkend << "\">"
                << x.content << "</link>";
        }

        actions.phrase << "</bridgehead>";
    }

    void output(quickbook::actions& actions, variablelist const& x)
    {
        actions.phrase << "<variablelist>\n";

        actions.phrase << "<title>";
        actions.phrase << encode(x.title);
        actions.phrase << "</title>\n";

        boostbook_markup m = markup_map.at("varlistentry");

        for(std::vector<varlistentry>::const_iterator
            it = x.entries.begin(); it != x.entries.end(); ++it)
        {
            actions.phrase << m.pre;
            std::for_each(it->begin(), it->end(), output_action(actions));
            actions.phrase << m.post;
        }

        actions.phrase << "</variablelist>\n";
    }

    void output(quickbook::actions& actions, table2 const& x)
    {
        if (x.title)
        {
            actions.phrase << "<table frame=\"all\"";
            if(x.id)
                actions.phrase << " id=\"" << *x.id << "\"";
            actions.phrase << ">\n";
            actions.phrase << "<title>";
            actions.phrase << encode(*x.title);
            actions.phrase << "</title>";
        }
        else
        {
            actions.phrase << "<informaltable frame=\"all\"";
            if(x.id)
                actions.phrase << " id=\"" << *x.id << "\"";
            actions.phrase << ">\n";
        }

        // This is a bit odd for backwards compatability: the old version just
        // used the last count that was calculated.
        actions.phrase << "<tgroup cols=\"" << x.cols << "\">\n";

        boostbook_markup m = markup_map.at("row");

        if (x.head)
        {
            actions.phrase << "<thead>";
            actions.phrase << m.pre;
            std::for_each(x.head->begin(), x.head->end(), actions.process);
            actions.phrase << m.post;
            actions.phrase << "</thead>\n";
        }

        actions.phrase << "<tbody>\n";

        for(std::vector<table_row>::const_iterator
            it = x.rows.begin(); it != x.rows.end(); ++it)
        {
            actions.phrase << m.pre;
            std::for_each(it->begin(), it->end(), actions.process);
            actions.phrase << m.post;
        }

        actions.phrase << "</tbody>\n" << "</tgroup>\n";

        if (x.title)
        {
            actions.phrase << "</table>\n";
        }
        else
        {
            actions.phrase << "</informaltable>\n";
        }
    }

    void output(quickbook::actions& actions, xinclude2 const& x)
    {
        actions.phrase << "\n<xi:include href=\"" <<x.path << "\" />\n";
    }

    void output(quickbook::actions& actions, list2 const& x)
    {
        actions.phrase << std::string(x.mark == '#' ? "<orderedlist>\n" : "<itemizedlist>\n");

        for(std::vector<list_item2>::const_iterator
            it = x.items.begin(), end = x.items.end(); it != end; ++it)
        {
            actions.phrase << "<listitem>\n" << it->content;
            if(!it->sublist.items.empty()) output(actions, it->sublist);
            actions.phrase << std::string("\n</listitem>");
        }

        actions.phrase << std::string(x.mark == '#' ? "\n</orderedlist>" : "\n</itemizedlist>");
    }

    void output(quickbook::actions& actions, code_token const& x)
    {
        std::string type = x.type;
        if(type == "space") {
            actions.phrase << x.text;
        }
        else {
            actions.phrase
                << "<phrase role=\"" << x.type << "\">"
                << encode(x.text)
                << "</phrase>";
        }
    }
}
