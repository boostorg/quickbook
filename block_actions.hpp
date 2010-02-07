/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_BLOCK_ACTIONS_HPP)
#define BOOST_SPIRIT_QUICKBOOK_BLOCK_ACTIONS_HPP

#include "block.hpp"
#include "gen_types.hpp"

namespace quickbook
{
    // TODO: Just generate formatted.
    formatted process(quickbook::actions&, paragraph const&);
    begin_section2 process(quickbook::actions&, begin_section const&);
    end_section2 process(quickbook::actions&, end_section const&);
    heading2 process(quickbook::actions&, heading const&);
    nothing process(quickbook::actions&, def_macro const&);
    nothing process(quickbook::actions&, define_template const&);
    table2 process(quickbook::actions&, table const&);
    xinclude2 process(quickbook::actions&, xinclude const&);
    nothing process(quickbook::actions&, import const&);
    nothing process(quickbook::actions&, include const&);
    list2 process(quickbook::actions&, list const&);
}

#endif