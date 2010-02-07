/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/assert.hpp>
#include "phrase.hpp"
#include "actions_class.hpp"
#include "markups.hpp"
#include "utils.hpp"
#include "code.hpp"

namespace quickbook
{    
    nothing process(quickbook::actions& actions, source_mode const& s) {
        actions.source_mode = s.mode;
        return nothing();
    }

    nothing process(quickbook::actions& actions, macro const& x) {
        if (x.raw_markup == quickbook_get_date)
        {
            char strdate[64];
            strftime(strdate, sizeof(strdate), "%Y-%b-%d", current_time);
            actions.phrase << strdate;
        }
        else if (x.raw_markup == quickbook_get_time)
        {
            char strdate[64];
            strftime(strdate, sizeof(strdate), "%I:%M:%S %p", current_time);
            actions.phrase << strdate;
        }
        else
        {
            actions.phrase << x.raw_markup;
        }
        return nothing();
    }

    nothing process(quickbook::actions& actions, anchor const& x) {
        actions.phrase << "<anchor id=\"";
        detail::print_string(x.id, actions.phrase.get());
        actions.phrase << "\" />\n";
        return nothing();
    }

    nothing process(quickbook::actions& actions, link const& x) {
        actions.phrase << x.type.pre;
        detail::print_string(x.destination, actions.phrase.get());
        actions.phrase << "\">";
        if(x.content.empty())
            detail::print_string(x.destination, actions.phrase.get());
        else
            actions.phrase << x.content;
        actions.phrase << x.type.post;
        return nothing();
    }

    nothing process(quickbook::actions& actions, formatted const& x) {
        actions.phrase << x.type.pre << x.content << x.type.post;
        return nothing();
    }

    nothing process(quickbook::actions& actions, simple_markup const& x) {
        markup type;
        switch(x.symbol) {
            case '*': type = markup(bold_pre_, bold_post_); break;
            case '/': type = markup(italic_pre_, italic_post_); break;
            case '_': type = markup(underline_pre_, underline_post_); break;
            case '=': type = markup(teletype_pre_, teletype_post_); break;
            default: BOOST_ASSERT(false);
        }
        actions.phrase << type.pre;
        detail::print_string(x.raw_content, actions.phrase.get());
        actions.phrase << type.post;
        return nothing();
    }

    nothing process(quickbook::actions& actions, cond_phrase const& x) {
        bool symbol_found = actions.macro.find(x.macro_id.c_str());

        if (!x.content.empty() && symbol_found) {
            actions.phrase << x.content; // print the body
        }
        return nothing();
    }

    nothing process(quickbook::actions& actions, break_ const& x) {
        detail::outwarn(x.position.file,x.position.line)
            << "in column:" << x.position.column << ", "
            << "[br] and \\n are deprecated" << ".\n";
        actions.phrase << break_mark;
        return nothing();
    }

    nothing process(quickbook::actions& actions, code const& x) {
         std::string program = x.code;
    
        if(x.block) {
            // preprocess the code section to remove the initial indentation
            detail::unindent(program);
            if (program.size() == 0)
                return nothing(); // Nothing left to do here. The program is empty.
        }

        iterator first_(program.begin(), program.end());
        iterator last_(program.end(), program.end());
        first_.set_position(x.position);

        std::string save;
        actions.phrase.swap(save);

        // print the code with syntax coloring
        std::string str = actions.syntax_p(first_, last_);

        actions.phrase.swap(save);

        if(x.block) {    
            //
            // We must not place a \n after the <programlisting> tag
            // otherwise PDF output starts code blocks with a blank line:
            //
            actions.phrase << "<programlisting>";
            actions.phrase << str;
            actions.phrase << "</programlisting>\n";
        }
        else {
            actions.phrase << "<code>";
            actions.phrase << str;
            actions.phrase << "</code>";
        }
        return nothing();
    }
}
