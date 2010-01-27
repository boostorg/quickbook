/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "./phrase.hpp"
#include "./detail/actions_class.hpp"
#include "./detail/markups.hpp"

namespace quickbook
{    
    void process(quickbook::actions& actions, source_mode const& s) {
        actions.source_mode = s.mode;
    }

    void process(quickbook::actions& actions, anchor const& x) {
        actions.phrase << "<anchor id=\"";
        detail::print_string(x.id, actions.phrase.get());
        actions.phrase << "\" />\n";
    }

    void process(quickbook::actions& actions, link const& x) {
        actions.phrase << x.type.pre;
        detail::print_string(x.destination, actions.phrase.get());
        actions.phrase << "\">";
        if(x.content.empty())
            detail::print_string(x.destination, actions.phrase.get());
        else
            actions.phrase << x.content;
        actions.phrase << x.type.post;
    }

    void process(quickbook::actions& actions, formatted const& x) {
        actions.phrase << x.type.pre << x.content << x.type.post;
    }

    void process(quickbook::actions& actions, cond_phrase const& x) {
        bool symbol_found = actions.macro.find(x.macro_id.c_str());

        if (!x.content.empty() && symbol_found) {
            actions.phrase << x.content; // print the body
        }
    }

    void process(quickbook::actions& actions, break_ const& x) {
        detail::outwarn(x.position.file,x.position.line)
            << "in column:" << x.position.column << ", "
            << "[br] and \\n are deprecated" << ".\n";
        actions.phrase << break_mark;

    }
}
