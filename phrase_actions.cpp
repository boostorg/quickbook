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
#include "boostbook.hpp"

namespace quickbook
{    
    nothing process(quickbook::actions& actions, source_mode const& s) {
        actions.source_mode = s.mode;
        return nothing();
    }

    std::string process(quickbook::actions& actions, macro const& x) {
        if (x.raw_markup == quickbook_get_date)
        {
            char strdate[64];
            strftime(strdate, sizeof(strdate), "%Y-%b-%d", current_time);
            return encode(strdate);
        }
        else if (x.raw_markup == quickbook_get_time)
        {
            char strdate[64];
            strftime(strdate, sizeof(strdate), "%I:%M:%S %p", current_time);
            return encode(strdate);
        }
        else
        {
            return x.raw_markup;
        }
    }

    link process(quickbook::actions& actions, link const& x) {
        link r = x;
        if(r.content.empty()) {
            r.content = encode(x.destination);
        }
        return r;
    }

    formatted process(quickbook::actions& actions, simple_markup const& x) {
        formatted r;
        switch(x.symbol) {
            case '*': r.type = "bold"; break;
            case '/': r.type = "italic"; break;
            case '_': r.type = "underline"; break;
            case '=': r.type = "teletype"; break;
            default: BOOST_ASSERT(false);
        }

        r.content = encode(x.raw_content);

        return r;
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
