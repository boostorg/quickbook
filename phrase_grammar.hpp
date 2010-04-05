/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/fusion/include/adapt_struct.hpp>
#include "grammar_impl.hpp"
#include "phrase.hpp"

BOOST_FUSION_ADAPT_STRUCT( 
 quickbook::break_,
    (quickbook::file_position, position)
)