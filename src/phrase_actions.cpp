/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/assert.hpp>
#include "phrase_actions.hpp"
#include "phrase.hpp"
#include "actions.hpp"
#include "state.hpp"
#include "utils.hpp"
#include "code.hpp"
#include "encoder.hpp"
#include "quickbook.hpp"

namespace quickbook
{    
    void process(quickbook::state& state, source_mode const& s) {
        state.source_mode = s.mode;
    }

    // TODO: If I used a different types for quickbook and the generated
    // output, I could possibly do something smarter here.
    void process(quickbook::state& state, macro const& x) {
        if (x.raw_markup == quickbook_get_date)
        {
            char strdate[64];
            strftime(strdate, sizeof(strdate), "%Y-%b-%d", current_time);
            state.encode(state.encoder->encode(strdate));
        }
        else if (x.raw_markup == quickbook_get_time)
        {
            char strdate[64];
            strftime(strdate, sizeof(strdate), "%I:%M:%S %p", current_time);
            state.encode(state.encoder->encode(strdate));
        }
        else
        {
            state.encode(x.raw_markup);
        }
    }

    void process(quickbook::state& state, link const& x) {
        link r = x;
        if(r.content.empty()) {
            r.content = state.encoder->encode(x.destination);
        }
        state.encode(r);
    }

    void process(quickbook::state& state, simple_markup const& x) {
        formatted r;
        switch(x.symbol) {
            case '*': r.type = "bold"; break;
            case '/': r.type = "italic"; break;
            case '_': r.type = "underline"; break;
            case '=': r.type = "teletype"; break;
            default: BOOST_ASSERT(false);
        }

        r.content = state.encoder->encode(x.raw_content);

        state.encode(r);
    }

    void process(quickbook::state& state, cond_phrase const& x) {
        bool symbol_found = state.macro.find(x.macro_id.c_str());

        if(!x.content.empty() && symbol_found) state.encode(x.content);
    }

    void process(quickbook::state& state, break_ const& x) {
        detail::outwarn(x.position.file,x.position.line)
            << "in column:" << x.position.column << ", "
            << "[br] and \\n are deprecated" << ".\n";
        state.encode(x);
    }

    void process(quickbook::state& state, code const& x) {
        if(x.flow == x.block) state.paragraph_output();
    
         std::string program = x.content;
    
        if(x.flow == x.block || x.flow == x.inline_block) {
            // preprocess the code section to remove the initial indentation
            detail::unindent(program);
            if (program.size() == 0)
                return; // Nothing left to do here. The program is empty.
        }

        iterator first_(program.begin(), program.end());
        iterator last_(program.end(), program.end());
        first_.set_position(x.position);

        // TODO: I don't need to save this, do I?
        std::string save;
        state.phrase.swap(save);

        // print the code with syntax coloring
        quickbook::actions actions(state);
        std::string str = syntax_highlight(
            first_, last_, actions, state.source_mode);

        state.phrase.swap(save);
        
        if(x.flow == x.block) {
            block_formatted r;
            r.type = "programlisting";
            r.content = str;
            state.encode(r);
        }
        else {
            formatted r;
            r.type = x.flow == x.inline_block ? "programlisting" : "code";
            r.content = str;
            state.encode(r);
        }
    }
}
