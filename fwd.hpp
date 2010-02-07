/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_FWD_HPP)
#define BOOST_SPIRIT_FWD_HPP

#include <boost/spirit/include/classic_iterator.hpp>

namespace quickbook
{
    struct nothing {};

    struct template_stack;
    struct macro;
    struct doc_info;

    struct state;
    struct actions;
    struct code_snippet_actions;

    struct collector;
    struct string_stream;
 
    typedef boost::spirit::classic::position_iterator<
        std::string::const_iterator> iterator;
    typedef boost::spirit::classic::file_position file_position;

    // templates

    struct call_template;
    struct define_template;
    struct template_symbol;
}

#endif