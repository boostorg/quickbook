/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "fwd.hpp"
#include "phrase_actions.hpp"
#include "actions.hpp"
#include "parse_types.hpp"
#include "block.hpp"
#include "code.hpp"
#include "syntax_highlight.hpp"
#include "template.hpp"
#include "boostbook.hpp"

namespace quickbook
{
    template <typename T>
    void process_action::operator()(T const& x) const
    {
        output(actions, process(actions, x));
    }

    template <typename T>
    T const& process(quickbook::actions&, T const& x)
    {
        return x;
    }

    void output(quickbook::actions&, nothing) {
    }

    void output(quickbook::actions&, std::string const&);
    nothing process(quickbook::actions&, call_template const&);
    nothing process(quickbook::actions&, cond_phrase const&);
    nothing process(quickbook::actions&, break_ const&);
    nothing process(quickbook::actions&, image const&);
    nothing process(quickbook::actions&, hr);
    nothing process(quickbook::actions&, paragraph const&);
    nothing process(quickbook::actions&, list const&);
    nothing process(quickbook::actions&, begin_section const&);
    nothing process(quickbook::actions&, end_section const&);
    nothing process(quickbook::actions&, heading const&);
    nothing process(quickbook::actions&, def_macro const&);
    nothing process(quickbook::actions&, variablelist const&);
    nothing process(quickbook::actions&, table const&);
    nothing process(quickbook::actions&, xinclude const&);
    nothing process(quickbook::actions&, import const&);
    nothing process(quickbook::actions&, include const&);
    nothing process(quickbook::actions&, code const&);
    nothing process(quickbook::actions&, define_template const&);
    nothing process(quickbook::actions&, code_token const&);

    template void process_action::operator()<formatted>(formatted const&) const;
    template void process_action::operator()<source_mode>(source_mode const&) const;
    template void process_action::operator()<macro>(macro const&) const;
    template void process_action::operator()<call_template>(call_template const&) const;
    template void process_action::operator()<anchor>(anchor const&) const;
    template void process_action::operator()<link>(link const&) const;
    template void process_action::operator()<simple_markup>(simple_markup const&) const;
    template void process_action::operator()<cond_phrase>(cond_phrase const&) const;
    template void process_action::operator()<break_>(break_ const&) const;
    template void process_action::operator()<image>(image const&) const;
    template void process_action::operator()<hr>(hr const&) const;
    template void process_action::operator()<paragraph>(paragraph const&) const;
    template void process_action::operator()<list>(list const&) const;
    template void process_action::operator()<begin_section>(begin_section const&) const;
    template void process_action::operator()<end_section>(end_section const&) const;
    template void process_action::operator()<heading>(heading const&) const;
    template void process_action::operator()<def_macro>(def_macro const&) const;
    template void process_action::operator()<variablelist>(variablelist const&) const;
    template void process_action::operator()<table>(table const&) const;
    template void process_action::operator()<xinclude>(xinclude const&) const;
    template void process_action::operator()<import>(import const&) const;
    template void process_action::operator()<include>(include const&) const;
    template void process_action::operator()<code>(code const&) const;
    template void process_action::operator()<define_template>(define_template const&) const;
    template void process_action::operator()<code_token>(code_token const&) const;
}