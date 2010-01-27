/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2005 Thomas Guest
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include <numeric>
#include <functional>
#include <algorithm>
#include <iterator>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/support_unused.hpp>
#include "./quickbook.hpp"
#include "./actions.hpp"
#include "./utils.hpp"
#include "./markups.hpp"
#include "./actions_class.hpp"
#include "../grammars.hpp"

namespace quickbook
{
    char const* quickbook_get_date = "__quickbook_get_date__";
    char const* quickbook_get_time = "__quickbook_get_time__";
    unsigned qbk_major_version = 0;
    unsigned qbk_minor_version = 0;
    unsigned qbk_version_n = 0; // qbk_major_version * 100 + qbk_minor_version

    namespace {
        std::string fully_qualified_id(std::string const& library_id,
            std::string const& qualified_section_id,
            std::string const& section_id)
        {
            std::string id = library_id;
            if(!id.empty() && !qualified_section_id.empty()) id += '.';
            id += qualified_section_id;
            if(!id.empty() && !section_id.empty()) id += '.';
            id += section_id;
            return id;
        }
    }

    void error_action::operator()(iterator_range x, unused_type, unused_type) const
    {
        boost::spirit::classic::file_position const pos = x.begin().get_position();
        detail::outerr(pos.file,pos.line)
            << "Syntax Error near column " << pos.column << ".\n";
        ++error_count;
    }

    void plain_char_action::operator()(char ch, unused_type, unused_type) const
    {
        detail::print_char(ch, phrase.get());
    }

    void plain_char_action::operator()(iterator_range x, unused_type, unused_type) const
    {
        detail::print_char(*x.begin(), phrase.get());
    }

    void element_id_warning_action::operator()(iterator_range x, unused_type, unused_type) const
    {
        boost::spirit::classic::file_position const pos = x.begin().get_position();
        detail::outwarn(pos.file,pos.line) << "Empty id.\n";        
    }

    void code_snippet_actions::pass_thru(char x)
    {
        code += x;
    }

    namespace detail
    {
        int callout_id = 0;
    }

    void code_snippet_actions::callout(std::string const& x, char const* role)
    {
        using detail::callout_id;
        code += "``'''";
        code += std::string("<phrase role=\"") + role + "\">";
        code += "<co id=\"";
        code += doc_id + boost::lexical_cast<std::string>(callout_id + callouts.size()) + "co\" ";
        code += "linkends=\"";
        code += doc_id + boost::lexical_cast<std::string>(callout_id + callouts.size()) + "\" />";
        code += "</phrase>";
        code += "'''``";

        callouts.push_back(x);
    }

    void code_snippet_actions::inline_callout(std::string const& x)
    {
        callout(x, "callout_bug");
    }

    void code_snippet_actions::line_callout(std::string const& x)
    {
        callout(x, "line_callout_bug");
    }

    void code_snippet_actions::escaped_comment(std::string const& x)
    {
        if (!code.empty())
        {
            detail::unindent(code); // remove all indents
            if (code.size() != 0)
            {
                snippet += "\n\n";
                snippet += source_type;
                snippet += "``\n" + code + "``\n\n";
                code.clear();
            }
        }
        std::string temp(x);
        detail::unindent(temp); // remove all indents
        if (temp.size() != 0)
        {
            snippet += "\n" + temp; // add a linebreak to allow block marskups
        }
    }

    void code_snippet_actions::compile(boost::iterator_range<iterator> x)
    {
        using detail::callout_id;
        if (!code.empty())
        {
            detail::unindent(code); // remove all indents
            if (code.size() != 0)
            {
                snippet += "\n\n";
                snippet += source_type;
                snippet += "```\n" + code + "```\n\n";
            }

            if(callouts.size() > 0)
            {
              snippet += "'''<calloutlist>'''";
              for (size_t i = 0; i < callouts.size(); ++i)
              {
                  snippet += "'''<callout arearefs=\"";
                  snippet += doc_id + boost::lexical_cast<std::string>(callout_id + i) + "co\" ";
                  snippet += "id=\"";
                  snippet += doc_id + boost::lexical_cast<std::string>(callout_id + i) + "\">";
                  snippet += "'''";

                  snippet += "'''<para>'''";
                  snippet += callouts[i];
                  snippet += "'''</para>'''";
                  snippet += "'''</callout>'''";
              }
              snippet += "'''</calloutlist>'''";
            }
        }

        std::vector<std::string> tinfo;
        tinfo.push_back(id);
        tinfo.push_back(snippet);
        storage.push_back(template_symbol(tinfo, x.begin().get_position()));

        callout_id += callouts.size();
        callouts.clear();
        code.clear();
        snippet.clear();
        id.clear();
    }

    void xml_author::operator()(std::pair<std::string, std::string> const& author) const
    {
        out << "      <author>\n"
            << "        <firstname>" << author.first << "</firstname>\n"
            << "        <surname>" << author.second << "</surname>\n"
            << "      </author>\n";
    }

    void xml_copyright::operator()(std::pair<std::vector<std::string>, std::string> const& copyright) const
    {
        out << "\n" << "    <copyright>\n";

        for_each(
            copyright.first.begin()
          , copyright.first.end()
          , xml_year(out));

        out << "      <holder>" << copyright.second << "</holder>\n"
            << "    </copyright>\n"
            << "\n"
        ;
    }

    void xml_year::operator()(std::string const &year) const
    {
        out << "      <year>" << year << "</year>\n";
    }

    static void write_document_title(collector& out, quickbook::actions& actions);
    static void write_document_info(collector& out, quickbook::actions& actions);

    void pre(collector& out, quickbook::actions& actions, bool ignore_docinfo)
    {
        // The doc_info in the file has been parsed. Here's what we'll do
        // *before* anything else.

        if (actions.doc_id.empty())
            actions.doc_id = detail::make_identifier(
                actions.doc_title.begin(),actions.doc_title.end());

        if (actions.doc_dirname.empty() && actions.doc_type == "library")
            actions.doc_dirname = actions.doc_id;

        if (actions.doc_last_revision.empty())
        {
            // default value for last-revision is now

            char strdate[64];
            strftime(
                strdate, sizeof(strdate),
                (debug_mode ?
                    "DEBUG MODE Date: %Y/%m/%d %H:%M:%S $" :
                    "$" /* prevent CVS substitution */ "Date: %Y/%m/%d %H:%M:%S $"),
                current_gm_time
            );
            actions.doc_last_revision = strdate;
        }

        // if we're ignoring the document info, we're done.
        if (ignore_docinfo)
        {
            return;
        }

        if (qbk_major_version == 0)
        {
            // hard code quickbook version to v1.1
            qbk_major_version = 1;
            qbk_minor_version = 1;
            qbk_version_n = 101;
            detail::outwarn(actions.filename.native_file_string(),1)
                << "Warning: Quickbook version undefined. "
                "Version 1.1 is assumed" << std::endl;
        }
        else
        {
            qbk_version_n = (qbk_major_version * 100) + qbk_minor_version;
        }

        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<!DOCTYPE library PUBLIC \"-//Boost//DTD BoostBook XML V1.0//EN\"\n"
            << "     \"http://www.boost.org/tools/boostbook/dtd/boostbook.dtd\">\n"
            << '<' << actions.doc_type << "\n"
            << "    id=\"" << actions.doc_id << "\"\n";
        
        if(actions.doc_type == "library")
        {
            out << "    name=\"" << actions.doc_title << "\"\n";
        }

        if(!actions.doc_dirname.empty())
        {
            out << "    dirname=\"" << actions.doc_dirname << "\"\n";
        }

        out << "    last-revision=\"" << actions.doc_last_revision << "\" \n"
            << "    xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n";
            
        if(actions.doc_type == "library") {
            write_document_info(out, actions);
            write_document_title(out, actions);
        }
        else {
            write_document_title(out, actions);
            write_document_info(out, actions);
        }
    }
    
    void post(collector& out, quickbook::actions& actions, bool ignore_docinfo)
    {
        // if we're ignoring the document info, do nothing.
        if (ignore_docinfo)
        {
            return;
        }

        // We've finished generating our output. Here's what we'll do
        // *after* everything else.
        out << "\n</" << actions.doc_type << ">\n\n";
    }

    void write_document_title(collector& out, quickbook::actions& actions)
    {
        if (!actions.doc_title.empty())
        {
            out<< "  <title>" << actions.doc_title;
            if (!actions.doc_version.empty())
                out << ' ' << actions.doc_version;
            out<< "</title>\n\n\n";
        }
    }

    void write_document_info(collector& out, quickbook::actions& actions)
    {
        out << "  <" << actions.doc_type << "info>\n";

        if(!actions.doc_authors.empty())
        {
            out << "    <authorgroup>\n";
            for_each(
                actions.doc_authors.begin()
              , actions.doc_authors.end()
              , xml_author(out));
            out << "    </authorgroup>\n";
        }

        if (!actions.doc_copyrights.empty())
        {
            for_each(
                actions.doc_copyrights.begin()
              , actions.doc_copyrights.end()
              , xml_copyright(out));
        }

        if (qbk_version_n < 103)
        {
            // version < 1.3 compatibility
            actions.doc_license = actions.doc_license_1_1;
            actions.doc_purpose = actions.doc_purpose_1_1;
        }

        if (!actions.doc_license.empty())
        {
            out << "    <legalnotice>\n"
                << "      <para>\n"
                << "        " << actions.doc_license << "\n"
                << "      </para>\n"
                << "    </legalnotice>\n"
                << "\n"
            ;
        }

        if (!actions.doc_purpose.empty())
        {
            out << "    <" << actions.doc_type << "purpose>\n"
                << "      " << actions.doc_purpose
                << "    </" << actions.doc_type << "purpose>\n"
                << "\n"
            ;
        }

        if (!actions.doc_category.empty())
        {
            out << "    <" << actions.doc_type << "category name=\"category:"
                << actions.doc_category
                << "\"></" << actions.doc_type << "category>\n"
                << "\n"
            ;
        }

        out << "  </" << actions.doc_type << "info>\n"
            << "\n"
        ;
    }

    void phrase_push_action::operator()(unused_type, unused_type, unused_type) const
    {
        phrase.push();
    }

    std::string phrase_pop_action::operator()() const
    {
        std::string out;
        phrase.swap(out);
        phrase.pop();
        return out;
    }

    void phrase_to_string_action::operator()(unused_type, unused_type, unused_type) const
    {
        phrase.swap(out);
    }
}

