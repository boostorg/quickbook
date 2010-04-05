/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_GRAMMARS_HPP)
#define BOOST_SPIRIT_QUICKBOOK_GRAMMARS_HPP

#include <boost/spirit/include/qi_core.hpp>
#include <boost/scoped_ptr.hpp>
#include "fwd.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;

    class quickbook_grammar
    {
    public:
        struct impl;

    private:
        boost::scoped_ptr<impl> impl_;

    public:
        qi::grammar<iterator> phrase;
        qi::grammar<iterator> simple_phrase;
        qi::grammar<iterator> block;
        qi::grammar<iterator, quickbook::doc_info()> doc_info;

        quickbook_grammar(quickbook::actions&);
        ~quickbook_grammar();
    };
}

#endif // BOOST_SPIRIT_QUICKBOOK_GRAMMARS_HPP
