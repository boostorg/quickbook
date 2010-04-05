/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_GRAMMARS_IMPL_HPP)
#define BOOST_SPIRIT_QUICKBOOK_GRAMMARS_IMPL_HPP

#include <boost/spirit/include/qi_core.hpp>
#include <boost/scoped_ptr.hpp>
#include "fwd.hpp"
#include "rule_store.hpp"
#include "grammar.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;

    struct quickbook_grammar::impl
    {
        quickbook::actions& actions;
        bool no_eols;
        rule_store store_;

        // phrase
        qi::rule<iterator> common;        
        qi::rule<iterator> simple_phrase;
        qi::rule<iterator, std::string()> phrase;
        qi::rule<iterator> phrase_markup;
        
        // block
        qi::rule<iterator> block_start;
        qi::rule<iterator> block_markup;
        qi::rule<iterator> command_line_macro;

        // doc_info
        qi::rule<iterator, quickbook::doc_info()> doc_info_details;
        
        impl(quickbook::actions&);

    private:

        void init_phrase();
        void init_phrase_markup();
        void init_block();
        void init_block_markup();
        void init_doc_info();
    };
}

#endif // BOOST_SPIRIT_QUICKBOOK_GRAMMARS_HPP
