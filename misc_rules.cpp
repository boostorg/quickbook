/*=============================================================================
    Copyright (c) 2002 2004  2006Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include "misc_rules.hpp"
#include "parse_utils.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;

    qi::rule<iterator, std::string()> macro_identifier;
    qi::rule<iterator> dummy_block;
    qi::rule<iterator> comment;
    qi::rule<iterator> hard_space;
    qi::rule<iterator> space;
    qi::rule<iterator> blank;
    qi::rule<iterator> eol;
    qi::rule<iterator, file_position()> position;

    void init_misc_rules() {
        macro_identifier =
            +(qi::char_ - (qi::space | ']'))
            ;
    
        dummy_block =
            '[' >> *(dummy_block | (qi::char_ - ']')) >> ']'
            ;
    
        comment =
            "[/" >> *(dummy_block | (qi::char_ - ']')) >> ']'
            ;
    
        // Used after an identifier that must not be immediately
        // followed by an alpha-numeric character or underscore.
        hard_space =
            !(qi::alnum | '_') >> space
            ;
    
        space =
            *(qi::space | comment)
            ;
    
        blank =
            *(qi::blank | comment)
            ;
    
        eol =
            blank >> qi::eol
            ;
    
        position =
            qi::raw[qi::eps] [get_position];
    }
}
