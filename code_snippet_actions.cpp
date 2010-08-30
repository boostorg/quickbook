/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2005 Thomas Guest
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <numeric>
#include <functional>
#include <algorithm>
#include <iterator>
#include <boost/lexical_cast.hpp>
#include "utils.hpp"
#include "grammar.hpp"
#include "code_snippet_types.hpp"
#include "template.hpp"

namespace quickbook
{
    void code_snippet_actions::process_action::operator()(char x, unused_type, unused_type) const
    {
        actions.code += x;
    }

    void code_snippet_actions::process_action::operator()(callout const& x, unused_type, unused_type) const
    {
        actions.code += "``[[callout" + boost::lexical_cast<std::string>(actions.callouts.size()) + "]]``";

        callout_source item;
        item.body = template_body(x.content, x.position, true);
        item.role = x.role;
        actions.callouts.push_back(item);
    }

    void code_snippet_actions::process_action::operator()(escaped_comment const& x, unused_type, unused_type) const
    {
        if (!actions.code.empty())
        {
            detail::unindent(actions.code); // remove all indents
            if (actions.code.size() != 0)
            {
                actions.snippet += "\n\n";
                actions.snippet += actions.source_type;
                actions.snippet += "``\n" + actions.code + "``\n\n";
                actions.code.clear();
            }
        }
        std::string temp(x.content);
        detail::unindent(temp); // remove all indents
        if (temp.size() != 0)
        {
            actions.snippet += "\n" + temp; // add a linebreak to allow block marskups
        }
    }

    void code_snippet_actions::output_action::operator()(code_snippet const& x, unused_type, unused_type) const
    {
        std::vector<std::string> params;

        if (!actions.code.empty())
        {
            detail::unindent(actions.code); // remove all indents
            if (actions.code.size() != 0)
            {
                actions.snippet += "\n\n";
                actions.snippet += actions.source_type;
                actions.snippet += "```\n" + actions.code + "```\n\n";
            }

            if(actions.callouts.size() > 0)
            {
              for (size_t i = 0; i < actions.callouts.size(); ++i)
              {
                  params.push_back("[callout" + boost::lexical_cast<std::string>(i) + "]");
              }
            }
        }

        define_template d(x.identifier, params,
            template_body(actions.snippet, x.position, true));
        d.callouts = actions.callouts;
        actions.storage.push_back(d);

        actions.callouts.clear();
        actions.code.clear();
        actions.snippet.clear();
    }
}
