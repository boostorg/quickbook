#include <algorithm>
#include <boost/foreach.hpp>
#include "encoder_impl.hpp"

namespace quickbook
{    
    template <typename Iter>
    std::string encode_impl(Iter first, Iter last)
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

    std::string html_encoder::encode(std::string const& x) const {
        return encode_impl(x.begin(), x.end());
    }

    std::string html_encoder::encode(char const* x) const {
        char const* end = x;
        while(*end) ++end;
        return encode_impl(x, end);
    }

    std::string html_encoder::encode(char c) const {
        return encode_impl(&c, &c + 1);
    }

    namespace {
        struct html_markup {
            char const* quickbook;
            char const* pre;
            char const* post;
        };
    
        html_markup markups[] = {
            { "", "", "" },
            { "comment", "<!--", "-->" },
            { "paragraph", "<p>\n", "</p>\n" },
            { "blurb", "<div class=\"blurb\">\n", "</div>\n" },
            { "blockquote", "<blockquote>", "</blockquote>" },
            { "preformatted", "<pre>", "</pre>" },
            { "warning", "<div class=\"warning\">", "</div>" },
            { "caution", "<div class=\"caution\">", "</div>" },
            { "important", "<div class=\"important\">", "</div>" },
            { "note", "<div class=\"note\">", "</div>" },
            { "tip", "<div class=\"tip\">", "</div>" },
            { "list_item", "<li>\n", "\n</li>" },
            { "bold", "<strong>", "</strong>" }, // TODO: Or <b>? Should this be semantically meaningfull?
            { "italic", "<em>", "</em>" }, // TODO: Or <i>?
            { "underline", "<u>", "</u>" },
            { "teletype", "<code>", "</code>" },
            { "strikethrough", "<strike>", "</strike>" },
            { "quote", "<q>", "</q>" },
            { "url", "<a href=\"", "</a>" },
            { "link", "<a href=\"#", "</a>" },
            { "funcref", "", "" },
            { "classref", "", "" },
            { "memberref", "", "" },
            { "enumref", "", "" },
            { "macroref", "", "" },
            { "headerref", "", "" },
            { "conceptref", "", "" },
            { "globalref", "", "" },
            // Will need to deal with this explicitly
            { "footnote", "<div class=\"footnote\"><p>", "</p></div>" },
            { "escape", "<!--quickbook-escape-prefix-->", "<!--quickbook-escape-postfix-->" },
            { "replaceable", "<i>", "</i>" },
            // TODO: Is it possible to have an entry with a term, but no items.
            { "varlistentry", "", "" },
            { "varlistterm", "<dt>", "</dt>" },
            { "varlistitem", "<dd>", "</dd>" },
            { "header", "<thead>", "</thead>\n" },
            { "row", "<tr>", "</tr>\n" },
            { "cell", "<td>", "</td>" },
            { "programlisting", "<pre>", "</pre>\n" },
            { "code", "<code>", "</code>" },
            { "hr", "<hr/>", "" },
            { "break", "<br/>", "" },
        };
        
        std::map<std::string, html_markup> markup_map;
        
        struct initialize {
            initialize() {
                BOOST_FOREACH(html_markup m, markups) {
                    markup_map[m.quickbook] = m;
                }
            }
        } initialize_instance;
    }

    void html_encoder::operator()(quickbook::state& state, std::string const& x) const
    {
        state.phrase << x;
    }

    void html_encoder::operator()(quickbook::state& state, char x) const
    {
        state.phrase << encode(x);
    }

    void html_encoder::operator()(quickbook::state& state, anchor const& x) const {
        state.phrase << "<a id=\"" << encode(x.id) << "\"/>\n";
    }

    void html_encoder::operator()(quickbook::state& state, link const& x) const {
        html_markup m = markup_map.at(x.type);
        if(*m.pre) {
            state.phrase << m.pre;
            state.phrase << encode(x.destination);
            state.phrase << "\">";
            state.phrase << x.content;
            state.phrase << m.post;
        }
        else {
            state.phrase << x.content;
        }
    }

    void html_encoder::operator()(quickbook::state& state, formatted const& x) const {
        html_markup m = markup_map.at(x.type);
        state.phrase << m.pre << x.content << m.post;
    }

    void html_encoder::operator()(quickbook::state& state, break_ const& x) const {
        html_markup m = markup_map.at("break");
        state.phrase << m.pre;
    }

    void html_encoder::operator()(quickbook::state& state, image2 const& x) const {
        std::map<std::string, char const*> translate;
        translate["alt"] = "alt";
        translate["fileref"] = "src";
        translate["width"] = "width";
        translate["height"] = "height";
    
        state.phrase << "<img";

        for(image2::attribute_map::const_iterator
            attr_first = x.attributes.begin(), attr_last  = x.attributes.end();
            attr_first != attr_last; ++attr_first)
        {
            char const* html_attribute = translate[attr_first->first];
            if(!html_attribute) continue;

            state.phrase
                << " "
                << html_attribute
                << "=\""
                << encode(attr_first->second)
                << "\"";
        }

        state.phrase << "/>";
    }

    void html_encoder::operator()(quickbook::state& state, hr) const {
        state.phrase << markup_map.at("hr").pre;
    }

    void html_encoder::operator()(quickbook::state& state, begin_section2 const& x) const {
        // TODO: Should this be stored in the 'token', or at least have a nicer interface.
        int level = state.section_level + 1;
        if (level > 6) level = 6; 
    
        state.phrase << "\n<section id=\"" << x.id << "\">\n";
        if(x.linkend.empty()) {
            state.phrase
                << "<h" << level << ">"
                << x.content
                << "</h" << level << ">\n"
                ;
        }
        else {
            state.phrase
                << "<h" << level << " id=\""
                << x.linkend
                << "\">"
                << x.content
                << "</h" << level << ">\n"
                ;
        }
    }

    void html_encoder::operator()(quickbook::state& state, end_section2 const& x) const {
        state.phrase << "</section>";
    }

    void html_encoder::operator()(quickbook::state& state, heading2 const& x) const {
        state.phrase
            << "<h" << x.level << " id=\"" << x.id << "\">"
            ;

        if(!x.linkend.empty()) {
            state.phrase
                << "<a id=\"" << x.linkend << "\"></a>"
                ;
        }
        state.phrase << x.content;

        state.phrase << "</h" << x.level << ">";
    }

    void html_encoder::operator()(quickbook::state& state, variablelist const& x) const
    {
        // TODO: What should I do for the title?
        state.phrase << "<p>";
        state.phrase << encode(x.title);
        state.phrase << "</p>\n";

        state.phrase << "<dl>\n";

        html_markup m = markup_map.at("varlistentry");

        for(std::vector<varlistentry>::const_iterator
            it = x.entries.begin(); it != x.entries.end(); ++it)
        {
            state.phrase << m.pre;
            std::for_each(it->begin(), it->end(), encode_action(state, *this));
            state.phrase << m.post;
        }

        state.phrase << "</dl>\n";
    }

    void html_encoder::operator()(quickbook::state& state, table2 const& x) const
    {
        if (x.title)
        {
            state.phrase << "<table";
            if(x.id)
                state.phrase << " id=\"" << *x.id << "\"";
            state.phrase << ">\n";
            state.phrase << "<caption>";
            state.phrase << encode(*x.title);
            state.phrase << "</caption>";
        }
        else
        {
            state.phrase << "<table";
            if(x.id)
                state.phrase << " id=\"" << *x.id << "\"";
            state.phrase << ">\n";
        }

        html_markup m = markup_map.at("row");

        if (x.head)
        {
            state.phrase << "<thead>";
            state.phrase << m.pre;
            std::for_each(x.head->begin(), x.head->end(), encode_action(state, *this));
            state.phrase << m.post;
            state.phrase << "</thead>\n";
        }

        state.phrase << "<tbody>\n";

        for(std::vector<table_row>::const_iterator
            it = x.rows.begin(); it != x.rows.end(); ++it)
        {
            state.phrase << m.pre;
            std::for_each(it->begin(), it->end(), encode_action(state, *this));
            state.phrase << m.post;
        }

        state.phrase << "</tbody>\n";

        state.phrase << "</table>\n";
    }

    void html_encoder::operator()(quickbook::state& state, xinclude2 const& x) const
    {
        // TODO: ?????
        //state.phrase << "\n<xi:include href=\"" << x.path << "\" />\n";
    }

    void html_encoder::operator()(quickbook::state& state, list2 const& x) const
    {
        state.phrase << std::string(x.mark == '#' ? "<ol>\n" : "<ul>\n");

        for(std::vector<list_item2>::const_iterator
            it = x.items.begin(), end = x.items.end(); it != end; ++it)
        {
            state.phrase << "<li>\n" << it->content;
            if(!it->sublist.items.empty()) (*this)(state, it->sublist);
            state.phrase << std::string("\n</li>");
        }

        state.phrase << std::string(x.mark == '#' ? "\n</ol>" : "\n</ul>");
    }

    void html_encoder::operator()(quickbook::state& state, code_token const& x) const
    {
        std::string type = x.type;
        if(type == "space") {
            state.phrase << x.text;
        }
        else {
            state.phrase
                << "<span class=\"" << x.type << "\">"
                << encode(x.text)
                << "</span>";
        }
    }

    void html_encoder::operator()(quickbook::state& state, doc_info const& info) const
    {
        // if we're ignoring the document info, we're done.
        if (info.ignore) return;

        state.phrase
            << "<!DOCTYPE html>"
            << "<html><head>"
            << "<title>" << info.doc_title << "</title>"
            << "</head>"
            << "<body>"
            << "<header>"
            << "<h1>" << info.doc_title << "</h1>"
            ;

        if(!info.doc_authors.empty())
        {
            state.phrase << "<h2 class=\"authors\">\n";
            BOOST_FOREACH(doc_info::author const& author, info.doc_authors) {
                state.phrase
                    << "<div>\n"
                    << author.first
                    << " "
                    << author.second
                    << "</div>\n";
            }
            state.phrase << "</h2>\n";
        }

        if(!info.doc_copyrights.empty())
        {
            state.phrase
                << "<p class=\"copyrights\">\n";

            BOOST_FOREACH(doc_info::copyright_entry const& copyright,
                info.doc_copyrights)
            {
                state.phrase << "<div>\nCopyright &copy; ";
    
                unsigned int range_state = 0;
                unsigned int previous = 0;
                BOOST_FOREACH(unsigned int year, copyright.first) {
                    switch(range_state) {
                    case 0: // Start
                        state.phrase << year;
                        range_state = 1;
                        break;
                    case 1: // Printed a year in last iteration
                        if(year == previous + 1) {
                            range_state = 2;
                        }
                        else {
                            state.phrase << ", " << year;
                            range_state = 1;
                        }
                        break;
                    case 2: // In the middle of a range
                        if(year != previous + 1) {
                            state.phrase << " - " << previous << ", " << year;
                            range_state = 1;
                        }
                        break;
                    }
                    previous = year;
                }
                if(range_state == 2) state.phrase << " - " << previous;
    
                state.phrase
                    << " "
                    << copyright.second
                    << "</div>\n"
                ;
            }

            state.phrase
                << "</p>\n";
        }

        if (!info.doc_license.empty())
        {
            state.phrase
                << "<p class=\"license\">\n"
                << info.doc_license
                << "\n"
                << "</p>\n"
                << "\n"
            ;
        }

        state.phrase
            << "</header>"
            ;
    }

    void html_encoder::operator()(quickbook::state& state, doc_info_post const& x) const
    {
        // if we're ignoring the document info, do nothing.
        if (x.info.ignore) return;

        // We've finished generating our output. Here's what we'll do
        // *after* everything else.
        state.phrase << "</html>";
    }
}
