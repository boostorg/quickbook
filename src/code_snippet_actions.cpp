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
#include <stack>
#include <boost/lexical_cast.hpp>
#include "utils.hpp"
#include "grammar.hpp"
#include "code_snippet_types.hpp"
#include "template.hpp"
#include "quickbook.hpp"
#include "utils.hpp"

namespace quickbook
{
    struct code_snippet_state
    {
        code_snippet_state(std::vector<define_template>& storage,
                                 std::string const& doc_id,
                                 char const* source_type)
            : storage(storage)
            , doc_id(doc_id)
            , source_type(source_type)
        {}

        void append_code();
        void close_code();

        struct snippet_data
        {
            snippet_data(std::string const& id, int callout_base_id, file_position position)
                : id(id)
                , callout_base_id(callout_base_id)
                , position(position)
                , content()
                , start_code(false)
                , end_code(false)
            {}

            std::string id;
            int callout_base_id;
            file_position position;
            std::string content;
            bool start_code;
            bool end_code;
            quickbook::callouts callouts;
        };

        int callout_id;
        std::stack<snippet_data> snippet_stack;
        std::string code;
        std::vector<define_template>& storage;
        std::string const doc_id;
        char const* const source_type;
    };

    int load_snippets(
        std::string const& file
      , std::vector<define_template>& storage   // for storing snippets are stored in a
                                                // vector of define_templates
      , std::string const& extension
      , std::string const& doc_id)
    {
        std::string code;
        int err = detail::load(file, code);
        if (err != 0)
            return err; // return early on error

        iterator first(code.begin(), code.end(), file.c_str());
        iterator last(code.end(), code.end());

        bool is_python = extension == ".py";
        code_snippet_state state(storage, doc_id, is_python ? "[python]" : "[c++]");
        snippet_actions actions(state);
        load_code_snippets(actions, storage, is_python, first, last);

        return 0;
    }

    void code_snippet_state::append_code()
    {
        if(snippet_stack.empty()) return;
        code_snippet_state::snippet_data& snippet = snippet_stack.top();

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

    void code_snippet_state::close_code()
    {
        if(snippet_stack.empty()) return;
        code_snippet_state::snippet_data& snippet = snippet_stack.top();

        if(snippet.end_code)
        {
            snippet.content += "```\n\n";
            snippet.end_code = false;
        }
    }

    snippet_actions::snippet_actions(code_snippet_state& a)
        : state(a) {}

    void snippet_actions::operator()(char x, unused_type, unused_type) const
    {
        if(state.snippet_stack.empty()) return;
        state.code += x;
    }

    void snippet_actions::operator()(callout const& x, unused_type, unused_type) const
    {
        if(state.snippet_stack.empty()) return;
        state.code += "``[[callout" + boost::lexical_cast<std::string>(state.callout_id) + "]]``";
     
        callout_source item;
        item.body = template_body(x.content, x.position, true);
        item.role = x.role;
        state.snippet_stack.top().callouts.push_back(item);
        ++state.callout_id;
    }

    void snippet_actions::operator()(escaped_comment const& x, unused_type, unused_type) const
    {
        if(state.snippet_stack.empty()) return;
        code_snippet_state::snippet_data& snippet = state.snippet_stack.top();
        state.append_code();
        state.close_code();
        
        std::string temp(x.content);
        detail::unindent(temp); // remove all indents
        if (temp.size() != 0)
        {
            snippet.content += "\n" + temp; // add a linebreak to allow block marskups
        }
    }

    void snippet_actions::operator()(start_snippet const& x, unused_type, unused_type) const
    {    
        state.append_code();
        state.snippet_stack.push(code_snippet_state::snippet_data(x.identifier, state.callout_id, x.position));
    }

    void snippet_actions::operator()(end_snippet const& x, unused_type, unused_type) const
    {
        // TODO: Error?
        if(state.snippet_stack.empty()) return;

        state.append_code();

        code_snippet_state::snippet_data snippet = state.snippet_stack.top();
        state.snippet_stack.pop();

        std::string body;
        if(snippet.start_code) {
            body += "\n\n";
            body += state.source_type;
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
        state.storage.push_back(d);

        // Merge the snippet into its parent

        if(!state.snippet_stack.empty())
        {
            code_snippet_state::snippet_data& next = state.snippet_stack.top();
            if(!snippet.content.empty()) {
                if(!snippet.start_code) {
                    state.close_code();
                }
                else if(!next.end_code) {
                    next.content += "\n\n";
                    next.content += state.source_type;
                    next.content += "```\n";
                }
                
                next.content += snippet.content;
                next.end_code = snippet.end_code;
            }
            
            next.callouts.insert(next.callouts.end(), snippet.callouts.begin(), snippet.callouts.end());
        }
    }
}
