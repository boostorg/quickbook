/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_PHRASE_ACTIONS_HPP)
#define BOOST_SPIRIT_QUICKBOOK_PHRASE_ACTIONS_HPP

#include "phrase.hpp"
#include "code.hpp"
#include "gen_types.hpp"

namespace quickbook
{
    nothing process(quickbook::actions&, source_mode const&);
    std::string process(quickbook::actions&, macro const&);
    link process(quickbook::actions&, link const&);
    formatted process(quickbook::actions&, simple_markup const&);
    std::string process(quickbook::actions&, cond_phrase const&);
    break_ process(quickbook::actions&, break_ const&);
    formatted process(quickbook::actions&, code const&);
    image2 process(quickbook::actions&, image const&);
    std::string process(quickbook::actions&, call_template const&);
}

#endif
