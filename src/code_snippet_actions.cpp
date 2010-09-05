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
    void code_snippet_actions::append_code()
    {
        if(snippet_stack.empty()) return;
        snippet_data& snippet = snippet_stack.top();

        if (!code.empty())
        {
            detail::unindent(code); // remove all indents

            if(snippet.content.empty())
            {
                snippet.start_code = true;
            }
            else if(!snippet.end_code)
            {
                snippet.content += "\n\n";
                snippet.content += source_type;
                snippet.content += "```\n";
            }
            
            snippet.content += code;
            snippet.end_code = true;

            code.clear();
        }
    }

    void code_snippet_actions::close_code()
    {
        if(snippet_stack.empty()) return;
        snippet_data& snippet = snippet_stack.top();

        if(snippet.end_code)
        {
            snippet.content += "```\n\n";
            snippet.end_code = false;
        }
    }

    void code_snippet_actions::process_action::operator()(char x, unused_type, unused_type) const
    {
        if(actions.snippet_stack.empty()) return;
        actions.code += x;
    }

    void code_snippet_actions::process_action::operator()(callout const& x, unused_type, unused_type) const
    {
        if(actions.snippet_stack.empty()) return;
        actions.code += "``[[callout" + boost::lexical_cast<std::string>(actions.callout_id) + "]]``";
     
        callout_source item;
        item.body = template_body(x.content, x.position, true);
        item.role = x.role;
        actions.snippet_stack.top().callouts.push_back(item);
        ++actions.callout_id;
    }

    void code_snippet_actions::process_action::operator()(escaped_comment const& x, unused_type, unused_type) const
    {
        if(actions.snippet_stack.empty()) return;
        snippet_data& snippet = actions.snippet_stack.top();
        actions.append_code();
        actions.close_code();
        
        std::string temp(x.content);
        detail::unindent(temp); // remove all indents
        if (temp.size() != 0)
        {
            snippet.content += "\n" + temp; // add a linebreak to allow block marskups
        }
    }

    void code_snippet_actions::process_action::operator()(start_snippet const& x, unused_type, unused_type) const
    {    
        actions.append_code();
        actions.snippet_stack.push(snippet_data(x.identifier, actions.callout_id, x.position));
    }

    void code_snippet_actions::process_action::operator()(end_snippet const& x, unused_type, unused_type) const
    {
        // TODO: Error?
        if(actions.snippet_stack.empty()) return;

        actions.append_code();

        snippet_data snippet = actions.snippet_stack.top();
        actions.snippet_stack.pop();

        std::string body;
        if(snippet.start_code) {
            body += "\n\n";
            body += actions.source_type;
            body += "```\n";
        }
        body += snippet.content;
        if(snippet.end_code) {
            body += "```\n\n";
        }

        std::vector<std::string> params;
        for (size_t i = 0; i < snippet.callouts.size(); ++i)
        {   
            params.push_back("[callout" + boost::lexical_cast<std::string>(snippet.callout_base_id + i) + "]");
        }

        define_template d(snippet.id, params, template_body(body, snippet.position, true));
        d.callouts = snippet.callouts;
        actions.storage.push_back(d);

        // Merge the snippet into its parent

        if(!actions.snippet_stack.empty())
        {
            snippet_data& next = actions.snippet_stack.top();
            if(!snippet.content.empty()) {
                if(!snippet.start_code) {
                    actions.close_code();
                }
                else if(!next.end_code) {
                    next.content += "\n\n";
                    next.content += actions.source_type;
                    next.content += "```\n";
                }
                
                next.content += snippet.content;
                next.end_code = snippet.end_code;
            }
            
            next.callouts.insert(next.callouts.end(), snippet.callouts.begin(), snippet.callouts.end());
        }
    }
}
