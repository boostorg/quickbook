/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/assert.hpp>
#include "block.hpp"
#include "actions_class.hpp"
#include "utils.hpp"

namespace quickbook
{
    typedef std::pair<char, int> mark_type;

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

    void process(quickbook::actions& actions, quickbook::list const& list)
    {
        int list_indent = -1;
        std::stack<mark_type> list_marks;

        for(list::const_iterator it = list.begin(), end = list.end(); it != end; ++it)
        {
            int new_indent = indent_length(it->indent);
            BOOST_ASSERT(it->mark == '#' || it->mark == '*');
            
            // The first item shouldn't be indented.
            BOOST_ASSERT(list_indent != -1 || new_indent == 0);
    
            if (new_indent > list_indent)
            {
                list_indent = new_indent;
                list_marks.push(mark_type(it->mark, list_indent));
                actions.phrase << std::string(it->mark == '#' ? "<orderedlist>\n" : "<itemizedlist>\n");
            }
            else if(new_indent == list_indent)
            {
                actions.phrase << std::string("\n</listitem>");
            }
            else if (new_indent < list_indent)
            {
                list_indent = new_indent;
    
                // TODO: This assumes that list_indent is equal to one of the
                // existing indents.
                while (!list_marks.empty() && (list_indent < list_marks.top().second))
                {
                    actions.phrase << std::string("\n</listitem>");
                    actions.phrase << std::string(list_marks.top().first == '#' ? "\n</orderedlist>" : "\n</itemizedlist>");
                    list_marks.pop();
                }
                BOOST_ASSERT(!list_marks.empty());
                actions.phrase << std::string("\n</listitem>");
            }
    
            if (it->mark != list_marks.top().first)
            {
                file_position const pos = it->position;
                detail::outerr(pos.file,pos.line)
                    << "Illegal change of list style near column " << pos.column << ".\n";
                detail::outwarn(pos.file,pos.line)
                    << "Ignoring change of list style" << std::endl;
                ++actions.error_count;
            }
            
            actions.phrase << "<listitem>\n" << it->content;
        }

        while (!list_marks.empty())
        {
            actions.phrase << std::string("\n</listitem>");
            actions.phrase << std::string(list_marks.top().first == '#' ? "\n</orderedlist>" : "\n</itemizedlist>");
            list_marks.pop();
        }
    }
}