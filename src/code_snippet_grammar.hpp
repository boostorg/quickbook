/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_CODE_SNIPPET_GRAMMARS_HPP)
#define BOOST_SPIRIT_QUICKBOOK_CODE_SNIPPET_GRAMMARS_HPP

#include <boost/spirit/include/qi_core.hpp>
#include <boost/scoped_ptr.hpp>
#include "fwd.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;

    struct python_code_snippet_grammar
        : qi::grammar<iterator>
    {
        typedef code_snippet_actions actions_type;
    
        python_code_snippet_grammar(actions_type& actions);
        ~python_code_snippet_grammar();

        struct rules;
        boost::scoped_ptr<rules> rules_pimpl;
        qi::rule<iterator> start;
    private:
        python_code_snippet_grammar(python_code_snippet_grammar const&);
        python_code_snippet_grammar& operator=(python_code_snippet_grammar const&);
    };

    struct cpp_code_snippet_grammar
        : qi::grammar<iterator>
    {
        typedef code_snippet_actions actions_type;
    
        cpp_code_snippet_grammar(actions_type& actions);
        ~cpp_code_snippet_grammar();

        struct rules;
        boost::scoped_ptr<rules> rules_pimpl;
        qi::rule<iterator> start;
    private:
        cpp_code_snippet_grammar(cpp_code_snippet_grammar const&);
        cpp_code_snippet_grammar& operator=(cpp_code_snippet_grammar const&);
    };
}

#endif // BOOST_SPIRIT_QUICKBOOK_CODE_SNIPPET_GRAMMARS_HPP
