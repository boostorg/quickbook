/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <stack>
#include <boost/assert.hpp>
#include "actions_class.hpp"
#include "gen_types.hpp"
#include "utils.hpp"

namespace quickbook
{
    namespace {
        int indent_length(std::string const& indent)
        {
            int length = 0;
            for(std::string::const_iterator
                first = indent.begin(), end = indent.end(); first != end; ++first)
            {
                switch(*first) {
                    case ' ': ++length; break;
                    // hardcoded tab to 4 for now
                    case '\t': length = ((length + 4) / 4) * 4; break;
                    default: BOOST_ASSERT(false);
                }
            }
            
            return length;
        }
    }

    struct stack_entry
    {
        explicit stack_entry(list2& list, int indent) : list(list), indent(indent) {}
        list2& list;
        int indent;
    };

    list2 process(quickbook::actions& actions, quickbook::list const& list)
    {
        list::const_iterator it = list.begin(), end = list.end();
        BOOST_ASSERT(it != end);
        
        list2 r;
        r.mark = list.begin()->mark;
        std::stack<stack_entry> stack;
        stack.push(stack_entry(r, 0));

        for(list::const_iterator it = list.begin(), end = list.end(); it != end; ++it)
        {
            int new_indent = indent_length(it->indent);
            BOOST_ASSERT(it->mark == '#' || it->mark == '*');
            
            if (new_indent > stack.top().indent)
            {
                stack.push(stack_entry(stack.top().list.items.back().sublist, new_indent));
                stack.top().list.mark = it->mark;
            }
            else if (new_indent < stack.top().indent)
            {
                // TODO: This assumes that new_indent is equal to one of the
                // existing indents.
                while (!stack.empty() && (new_indent < stack.top().indent))
                    stack.pop();
                BOOST_ASSERT(!stack.empty());
            }
            
            list_item2 item;
            item.content = it->content;
            stack.top().list.items.push_back(item);
    
            if (it->mark != stack.top().list.mark)
            {
                file_position const pos = it->position;
                detail::outerr(pos.file,pos.line)
                    << "Illegal change of list style near column " << pos.column << ".\n";
                detail::outwarn(pos.file,pos.line)
                    << "Ignoring change of list style" << std::endl;
                ++actions.error_count;
            }
        }

        return r;
    }
}