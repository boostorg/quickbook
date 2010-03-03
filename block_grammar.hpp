/*=============================================================================
    Copyright (c) 2002 2004  2006Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "grammars.hpp"
#include "block.hpp"
#include "rule_store.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;

    struct block_grammar::rules
    {
        rules(quickbook::actions& actions_);

        quickbook::actions& actions;
        bool no_eols;
        phrase_grammar common;
        
        rule_store store_;
        qi::rule<iterator> start_;
        qi::rule<iterator> block_markup;

        void init_block_markup();
    };
}