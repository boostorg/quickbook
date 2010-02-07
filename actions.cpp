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

