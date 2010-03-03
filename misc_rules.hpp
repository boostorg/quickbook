/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_MISC_RULES_HPP)
#define BOOST_SPIRIT_QUICKBOOK_MISC_RULES_HPP

#include <boost/spirit/include/qi_core.hpp>
#include "fwd.hpp"

// Just a few stateless parser rules that are used in a lot of places.

namespace quickbook
{
    extern boost::spirit::qi::rule<iterator, std::string()> macro_identifier;
    extern boost::spirit::qi::rule<iterator> comment;
    extern boost::spirit::qi::rule<iterator> eol;
    extern boost::spirit::qi::rule<iterator> hard_space;
    extern boost::spirit::qi::rule<iterator> space;
    extern boost::spirit::qi::rule<iterator> blank;
    extern boost::spirit::qi::rule<iterator, file_position()> position;
}

#endif