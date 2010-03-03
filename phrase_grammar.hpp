/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/fusion/include/adapt_struct.hpp>
#include "grammars.hpp"
#include "phrase.hpp"
#include "rule_store.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;

    struct phrase_grammar::rules
    {
        rules(quickbook::actions& actions, bool& no_eols);
    
        quickbook::actions& actions;
        bool& no_eols;

        rule_store store_;
        qi::rule<iterator> common;        
        qi::rule<iterator, std::string()> phrase;
        qi::rule<iterator> phrase_markup;
        
        void init_phrase_markup();
    };
}

BOOST_FUSION_ADAPT_STRUCT( 
 quickbook::break_,
    (quickbook::file_position, position)
)