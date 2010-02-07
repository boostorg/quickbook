/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2005 Thomas Guest
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "actions.hpp"
#include "actions_class.hpp"
#include "doc_info.hpp"
#include "quickbook.hpp" // TODO: Quickbook version number
#include "utils.hpp"

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
        file_position const pos = x.begin().get_position();
        detail::outerr(pos.file,pos.line)
            << "Syntax Error near column " << pos.column << ".\n";
        ++error_count;
    }

    void element_id_warning_action::operator()(iterator_range x, unused_type, unused_type) const
    {
        file_position const pos = x.begin().get_position();
        detail::outwarn(pos.file,pos.line) << "Empty id.\n";        
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

    static void write_document_title(collector& out, doc_info& actions);
    static void write_document_info(collector& out, doc_info& actions);

    void pre(collector& out, quickbook::actions& actions, doc_info& info, bool ignore_docinfo)
    {
        // The doc_info in the file has been parsed. Here's what we'll do
        // *before* anything else.

        if (!info.doc_title.empty())
            actions.doc_title = info.doc_title;

        if (info.doc_id.empty())
            info.doc_id = detail::make_identifier(
                actions.doc_title.begin(),actions.doc_title.end());

        if(actions.doc_id.empty())
            actions.doc_id = info.doc_id;

        if (info.doc_dirname.empty() && info.doc_type == "library")
            info.doc_dirname = actions.doc_id;

        if (info.doc_last_revision.empty())
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
            info.doc_last_revision = strdate;
        }

        // if we're ignoring the document info, we're done.
        if (ignore_docinfo)
        {
            return;
        }

        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            << "<!DOCTYPE library PUBLIC \"-//Boost//DTD BoostBook XML V1.0//EN\"\n"
            << "     \"http://www.boost.org/tools/boostbook/dtd/boostbook.dtd\">\n"
            << '<' << info.doc_type << "\n"
            << "    id=\"" << info.doc_id << "\"\n";
        
        if(info.doc_type == "library")
        {
            out << "    name=\"" << info.doc_title << "\"\n";
        }

        if(!info.doc_dirname.empty())
        {
            out << "    dirname=\"" << info.doc_dirname << "\"\n";
        }

        out << "    last-revision=\"" << info.doc_last_revision << "\" \n"
            << "    xmlns:xi=\"http://www.w3.org/2001/XInclude\">\n";
            
        if(info.doc_type == "library") {
            write_document_info(out, info);
            write_document_title(out, info);
        }
        else {
            write_document_title(out, info);
            write_document_info(out, info);
        }
    }
    
    void post(collector& out, quickbook::actions& actions, doc_info& info, bool ignore_docinfo)
    {
        // if we're ignoring the document info, do nothing.
        if (ignore_docinfo)
        {
            return;
        }

        // We've finished generating our output. Here's what we'll do
        // *after* everything else.
        out << "\n</" << info.doc_type << ">\n\n";
    }

    void write_document_title(collector& out, doc_info& info)
    {
        if (!info.doc_title.empty())
        {
            out<< "  <title>" << info.doc_title;
            if (!info.doc_version.empty())
                out << ' ' << info.doc_version;
            out<< "</title>\n\n\n";
        }
    }

    void write_document_info(collector& out, doc_info& info)
    {
        out << "  <" << info.doc_type << "info>\n";

        if(!info.doc_authors.empty())
        {
            out << "    <authorgroup>\n";
            for_each(
                info.doc_authors.begin()
              , info.doc_authors.end()
              , xml_author(out));
            out << "    </authorgroup>\n";
        }

        if (!info.doc_copyrights.empty())
        {
            for_each(
                info.doc_copyrights.begin()
              , info.doc_copyrights.end()
              , xml_copyright(out));
        }

        if (!info.doc_license.empty())
        {
            out << "    <legalnotice>\n"
                << "      <para>\n"
                << "        " << info.doc_license << "\n"
                << "      </para>\n"
                << "    </legalnotice>\n"
                << "\n"
            ;
        }

        if (!info.doc_purpose.empty())
        {
            out << "    <" << info.doc_type << "purpose>\n"
                << "      " << info.doc_purpose
                << "    </" << info.doc_type << "purpose>\n"
                << "\n"
            ;
        }

        if (!info.doc_category.empty())
        {
            out << "    <" << info.doc_type << "category name=\"category:"
                << info.doc_category
                << "\"></" << info.doc_type << "category>\n"
                << "\n"
            ;
        }

        out << "  </" << info.doc_type << "info>\n"
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
}

