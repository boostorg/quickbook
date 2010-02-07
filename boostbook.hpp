/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(BOOST_SPIRIT_QUICKBOOK_BOOSTBOOK_HPP)
#define BOOST_SPIRIT_QUICKBOOK_BOOSTBOOK_HPP

#include "fwd.hpp"
#include "phrase.hpp"
#include "block.hpp"
#include "syntax_highlight.hpp"
#include "gen_types.hpp"

namespace quickbook
{
    // TODO: Sort this out:

    // Output function for boostbook, these should eventually become an
    // interface with implementations for boostbook and html.
    // They probably shouldn't use quickbook::actions, instead they
    // should either take a stream/collector to write to, or return their
    // output by value.

    void output(quickbook::actions&, std::string const&);
    void output(quickbook::actions&, anchor const&);
    void output(quickbook::actions&, link const&);
    void output(quickbook::actions&, formatted const&);
    void output(quickbook::actions&, break_ const&);
    void output(quickbook::actions&, image2 const&);

    void output(quickbook::actions&, hr);
    void output(quickbook::actions&, begin_section2 const&);
    void output(quickbook::actions&, end_section2 const&);
    void output(quickbook::actions&, heading2 const&);
    void output(quickbook::actions&, variablelist const&);
    void output(quickbook::actions&, table2 const&);
    void output(quickbook::actions&, xinclude2 const&);
    void output(quickbook::actions&, list2 const&);

    void output(quickbook::actions&, code_token const&);

    std::string encode(std::string const&);
    std::string encode(char);
    std::string encode(char const*);
}

#endif
