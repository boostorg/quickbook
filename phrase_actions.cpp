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
#include "actions.hpp"
#include "state.hpp"
#include "utils.hpp"
#include "code.hpp"
#include "boostbook.hpp"
#include "quickbook.hpp"

namespace quickbook
{    
    nothing process(quickbook::state& state, source_mode const& s) {
        state.source_mode = s.mode;
        return nothing();
    }

    std::string process(quickbook::state& state, macro const& x) {
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

    link process(quickbook::state& state, link const& x) {
        link r = x;
        if(r.content.empty()) {
            r.content = encode(x.destination);
        }
        return r;
    }

    formatted process(quickbook::state& state, simple_markup const& x) {
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

    std::string process(quickbook::state& state, cond_phrase const& x) {
        bool symbol_found = state.macro.find(x.macro_id.c_str());

        return (!x.content.empty() && symbol_found) ? x.content : "";
    }

    break_ process(quickbook::state& state, break_ const& x) {
        detail::outwarn(x.position.file,x.position.line)
            << "in column:" << x.position.column << ", "
            << "[br] and \\n are deprecated" << ".\n";
        return x;
    }

    formatted process(quickbook::state& state, code const& x) {
        formatted r;
        r.type = "";

         std::string program = x.code;
    
        if(x.block) {
            // preprocess the code section to remove the initial indentation
            detail::unindent(program);
            if (program.size() == 0)
                return r; // Nothing left to do here. The program is empty.
        }

        iterator first_(program.begin(), program.end());
        iterator last_(program.end(), program.end());
        first_.set_position(x.position);

        // TODO: I don't need to save this, do I?
        std::string save;
        state.phrase.swap(save);

        // print the code with syntax coloring
        quickbook::actions actions(state);
        std::string str = actions.syntax_p(first_, last_);

        state.phrase.swap(save);
        
        r.type = x.block ? "programlisting" : "code";
        r.content = str;
        return r;
    }
}
