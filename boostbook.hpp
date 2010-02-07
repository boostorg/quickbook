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
#include "doc_info.hpp"
#include "gen_types.hpp"

namespace quickbook
{
    // TODO: Sort this out:

    // Output function for boostbook, these should eventually become an
    // interface with implementations for boostbook and html.
    // They probably shouldn't use quickbook::state, instead they
    // should either take a stream/collector to write to, or return their
    // output by value.

    void output(quickbook::state&, doc_info const&);
    void output(quickbook::state&, doc_info_post const&);

    // Note: char is a plain quickbook character, string is an encoded
    // boostbook string. Oops.
    void output(quickbook::state&, char);
    void output(quickbook::state&, std::string const&);
    void output(quickbook::state&, anchor const&);
    void output(quickbook::state&, link const&);
    void output(quickbook::state&, formatted const&);
    void output(quickbook::state&, break_ const&);
    void output(quickbook::state&, image2 const&);

    void output(quickbook::state&, hr);
    void output(quickbook::state&, begin_section2 const&);
    void output(quickbook::state&, end_section2 const&);
    void output(quickbook::state&, heading2 const&);
    void output(quickbook::state&, variablelist const&);
    void output(quickbook::state&, table2 const&);
    void output(quickbook::state&, xinclude2 const&);
    void output(quickbook::state&, list2 const&);

    void output(quickbook::state&, code_token const&);

    std::string encode(std::string const&);
    std::string encode(char);
    std::string encode(char const*);
}

#endif
