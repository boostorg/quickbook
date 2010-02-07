/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <string>
#include <vector>
#include <utility>
#include "fwd.hpp"
#include "collector.hpp"
#include "quickbook.hpp"
#include "doc_info_actions.hpp"
#include "actions.hpp"
#include "state.hpp"
#include "utils.hpp"

namespace quickbook
{
    doc_info process(quickbook::actions& actions, doc_info const& x)
    {
        doc_info info = x;
    
        // The doc_info in the file has been parsed. Here's what we'll do
        // *before* anything else.

        if(!info.doc_title.empty())
            actions.state_.doc_title = info.doc_title;

        if(info.doc_id.empty())
            info.doc_id = detail::make_identifier(
                actions.state_.doc_title.begin(),
                actions.state_.doc_title.end());

        if(actions.state_.doc_id.empty())
            actions.state_.doc_id = info.doc_id;

        if (info.doc_dirname.empty() && info.doc_type == "library")
            info.doc_dirname = actions.state_.doc_id;

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

        return info;
    }
}
