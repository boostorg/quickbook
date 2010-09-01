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

namespace quickbook
{
    void process(quickbook::state&, block_formatted const&);
    void process(quickbook::state&, paragraph const&);
    void process(quickbook::state&, block_separator const&);
    void process(quickbook::state&, begin_section const&);
    void process(quickbook::state&, end_section const&);
    void process(quickbook::state&, heading const&);
    void process(quickbook::state&, def_macro const&);
    void process(quickbook::state&, define_template const&);
    void process(quickbook::state&, table const&);
    void process(quickbook::state&, variablelist const&);
    void process(quickbook::state&, xinclude const&);
    void process(quickbook::state&, import const&);
    void process(quickbook::state&, include const&);
    void process(quickbook::state&, list const&);
}

#endif