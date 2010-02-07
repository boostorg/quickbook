#include <algorithm>
#include <boost/foreach.hpp>
#include "fwd.hpp"
#include "boostbook.hpp"
#include "phrase.hpp"
#include "state.hpp"

namespace quickbook
{
    struct output_action
    {
        output_action(quickbook::state& state) : state(state) {}    
        quickbook::state& state;

        template <typename T>
        void operator()(T const& x) const {
            output(state, x);
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

    void output(quickbook::state& state, std::string const& x)
    {
        state.phrase << x;
    }

    void output(quickbook::state& state, char x)
    {
        state.phrase << encode(x);
    }

    void output(quickbook::state& state, anchor const& x) {
        state.phrase << "<anchor id=\"";
        state.phrase << encode(x.id);
        state.phrase << "\"/>\n";
    }

    void output(quickbook::state& state, link const& x) {
        boostbook_markup m = markup_map.at(x.type);
        state.phrase << m.pre;
        state.phrase << encode(x.destination);
        state.phrase << "\">";
        state.phrase << x.content;
        state.phrase << m.post;
    }

    void output(quickbook::state& state, formatted const& x) {
        boostbook_markup m = markup_map.at(x.type);
        state.phrase << m.pre << x.content << m.post;
    }

    void output(quickbook::state& state, break_ const& x) {
        boostbook_markup m = markup_map.at("break");
        state.phrase << m.pre;
    }

    void output(quickbook::state& state, image2 const& x) {
        state.phrase << "<inlinemediaobject>";

        state.phrase << "<imageobject><imagedata";
        
        for(image2::attribute_map::const_iterator
            attr_first = x.attributes.begin(), attr_last  = x.attributes.end();
            attr_first != attr_last; ++attr_first)
        {
            if(attr_first->first == "alt") continue;
        
            state.phrase
                << " "
                << attr_first->first
                << "=\""
                << encode(attr_first->second)
                << "\"";
        }

        state.phrase << "></imagedata></imageobject>";

        image2::attribute_map::const_iterator it = x.attributes.find("alt");
        if(it != x.attributes.end()) {
            // Also add a textobject -- use the basename of the image file.
            // This will mean we get "alt" attributes of the HTML img.
            state.phrase << "<textobject><phrase>";
            state.phrase << encode(it->second);
            state.phrase << "</phrase></textobject>";
        }

        state.phrase << "</inlinemediaobject>";
    }

    void output(quickbook::state& state, hr) {
        state.phrase << markup_map.at("hr").pre;
    }

    void output(quickbook::state& state, begin_section2 const& x) {
        state.phrase << "\n<section id=\"" << x.id << "\">\n";
        if(x.linkend.empty()) {
            state.phrase
                << "<title>"
                << x.content
                << "</title>\n"
                ;
        }
        else {
            state.phrase
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

    void output(quickbook::state& state, end_section2 const& x) {
        state.phrase << "</section>";
    }

    void output(quickbook::state& state, heading2 const& x) {
        state.phrase
            << "<anchor id=\"" << x.id << "\"/>"
            << "<bridgehead renderas=\"sect" << x.level << "\">";

        if(x.linkend.empty()) {
            state.phrase << x.content;
        }
        else {
            state.phrase
                << "<link linkend=\"" << x.linkend << "\">"
                << x.content << "</link>";
        }

        state.phrase << "</bridgehead>";
    }

    void output(quickbook::state& state, variablelist const& x)
    {
        state.phrase << "<variablelist>\n";

        state.phrase << "<title>";
        state.phrase << encode(x.title);
        state.phrase << "</title>\n";

        boostbook_markup m = markup_map.at("varlistentry");

        for(std::vector<varlistentry>::const_iterator
            it = x.entries.begin(); it != x.entries.end(); ++it)
        {
            state.phrase << m.pre;
            std::for_each(it->begin(), it->end(), output_action(state));
            state.phrase << m.post;
        }

        state.phrase << "</variablelist>\n";
    }

    void output(quickbook::state& state, table2 const& x)
    {
        if (x.title)
        {
            state.phrase << "<table frame=\"all\"";
            if(x.id)
                state.phrase << " id=\"" << *x.id << "\"";
            state.phrase << ">\n";
            state.phrase << "<title>";
            state.phrase << encode(*x.title);
            state.phrase << "</title>";
        }
        else
        {
            state.phrase << "<informaltable frame=\"all\"";
            if(x.id)
                state.phrase << " id=\"" << *x.id << "\"";
            state.phrase << ">\n";
        }

        // This is a bit odd for backwards compatability: the old version just
        // used the last count that was calculated.
        state.phrase << "<tgroup cols=\"" << x.cols << "\">\n";

        boostbook_markup m = markup_map.at("row");

        if (x.head)
        {
            state.phrase << "<thead>";
            state.phrase << m.pre;
            std::for_each(x.head->begin(), x.head->end(), output_action(state));
            state.phrase << m.post;
            state.phrase << "</thead>\n";
        }

        state.phrase << "<tbody>\n";

        for(std::vector<table_row>::const_iterator
            it = x.rows.begin(); it != x.rows.end(); ++it)
        {
            state.phrase << m.pre;
            std::for_each(it->begin(), it->end(), output_action(state));
            state.phrase << m.post;
        }

        state.phrase << "</tbody>\n" << "</tgroup>\n";

        if (x.title)
        {
            state.phrase << "</table>\n";
        }
        else
        {
            state.phrase << "</informaltable>\n";
        }
    }

    void output(quickbook::state& state, xinclude2 const& x)
    {
        state.phrase << "\n<xi:include href=\"" << x.path << "\" />\n";
    }

    void output(quickbook::state& state, list2 const& x)
    {
        state.phrase << std::string(x.mark == '#' ? "<orderedlist>\n" : "<itemizedlist>\n");

        for(std::vector<list_item2>::const_iterator
            it = x.items.begin(), end = x.items.end(); it != end; ++it)
        {
            state.phrase << "<listitem>\n" << it->content;
            if(!it->sublist.items.empty()) output(state, it->sublist);
            state.phrase << std::string("\n</listitem>");
        }

        state.phrase << std::string(x.mark == '#' ? "\n</orderedlist>" : "\n</itemizedlist>");
    }

    void output(quickbook::state& state, code_token const& x)
    {
        std::string type = x.type;
        if(type == "space") {
            state.phrase << x.text;
        }
        else {
            state.phrase
                << "<phrase role=\"" << x.type << "\">"
                << encode(x.text)
                << "</phrase>";
        }
    }

    void output(quickbook::state& state, doc_info const& info)
    {
        // if we're ignoring the document info, we're done.
        if (info.ignore) return;

        state.phrase
            << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<!DOCTYPE library PUBLIC \"-//Boost//DTD BoostBook XML V1.0//EN\""
            << " \"http://www.boost.org/tools/boostbook/dtd/boostbook.dtd\">";

        // Document tag

        state.phrase
            << '<' << info.doc_type << " id=\"" << info.doc_id << "\"\n";
        
        if(info.doc_type == "library")
        {
            state.phrase << " name=\"" << info.doc_title << "\"\n";
        }

        if(!info.doc_dirname.empty())
        {
            state.phrase << " dirname=\"" << info.doc_dirname << "\"\n";
        }

        state.phrase
            << "last-revision=\"" << info.doc_last_revision << "\""
            << " xmlns:xi=\"http://www.w3.org/2001/XInclude\"";

        state.phrase << ">"; // end document tag.

        // Title tag

        std::string title;
        if(!info.doc_title.empty())
        {
            title =  "<title>" + info.doc_title;
            if (!info.doc_version.empty())
                title += ' ' + info.doc_version;
            title += "</title>\n";
        }

        // For 'library', the title comes after the info block.
        if(info.doc_type != "library") state.phrase << title;

        // Info tag

        state.phrase << "<" << info.doc_type << "info>\n";

        if(!info.doc_authors.empty())
        {
            state.phrase << "<authorgroup>\n";
            BOOST_FOREACH(doc_info::author const& author, info.doc_authors) {
                state.phrase
                    << "<author>\n"
                    << "<firstname>" << author.first << "</firstname>\n"
                    << "<surname>" << author.second << "</surname>\n"
                    << "</author>\n";
            }
            state.phrase << "</authorgroup>\n";
        }

        BOOST_FOREACH(doc_info::copyright_entry const& copyright,
            info.doc_copyrights)
        {
            state.phrase << "<copyright>\n";

            BOOST_FOREACH(std::string const& year, copyright.first) {
                state.phrase << "<year>" << year << "</year>\n";
            }

            state.phrase
                << "<holder>" << copyright.second << "</holder>\n"
                << "</copyright>\n"
            ;
        }

        if (!info.doc_license.empty())
        {
            state.phrase
                << "<legalnotice>\n"
                << "<para>\n"
                << info.doc_license
                << "\n"
                << "</para>\n"
                << "</legalnotice>\n"
                << "\n"
            ;
        }

        if (!info.doc_purpose.empty())
        {
            state.phrase
                << "<" << info.doc_type << "purpose>\n"
                << info.doc_purpose
                << "</" << info.doc_type << "purpose>\n"
                << "\n"
            ;
        }

        if (!info.doc_category.empty())
        {
            state.phrase
                << "<" << info.doc_type << "category name=\"category:"
                << info.doc_category
                << "\"></" << info.doc_type << "category>\n"
                << "\n"
            ;
        }

        state.phrase
            << "</" << info.doc_type << "info>\n"
        ;

        if(info.doc_type == "library") state.phrase << title;
    }

    void output(quickbook::state& state, doc_info_post const& x)
    {
        // if we're ignoring the document info, do nothing.
        if (x.info.ignore) return;

        // We've finished generating our output. Here's what we'll do
        // *after* everything else.
        state.phrase << "</" << x.info.doc_type << ">";
    }
}
