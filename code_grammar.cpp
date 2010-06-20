/*=============================================================================
    Copyright (c) 2002 2004  2006Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include "grammar_impl.hpp"
#include "actions.hpp"
#include "code.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;

    void quickbook_grammar::impl::init_code()
    {
        // Indented code
    
        qi::rule<iterator, quickbook::code()>& indented_code_impl = store_.create();
        qi::rule<iterator>& code_line = store_.create();

        indented_code = indented_code_impl [actions.process];

        indented_code_impl =
                position                                [member_assign(&quickbook::code::position)]
                                                        [member_assign(&quickbook::code::flow, quickbook::code::block)]
            >>  qi::raw[code_line >> *(*eol >> code_line)]
                                                        [member_assign(&quickbook::code::content)]
            >>  +eol
            ;

        code_line =
                qi::char_(" \t")
            >>  *(qi::char_ - eol)
            >>  eol
            ;

        qi::rule<iterator, quickbook::code()>& code_block1 = store_.create();
        qi::rule<iterator, quickbook::code()>& code_block2 = store_.create();

        code_block = (code_block1 | code_block2) [actions.process];

        code_block1
            =   "```"
            >>  position                            [member_assign(&quickbook::code::position)]
                                                    [member_assign(&quickbook::code::flow, quickbook::code::inline_block)]
            >>  qi::raw[*(qi::char_ - "```")]       [member_assign(&quickbook::code::content)]
            >>  "```"
            ;

        code_block2
            =   "``"
            >>  position                            [member_assign(&quickbook::code::position)]
                                                    [member_assign(&quickbook::code::flow, quickbook::code::inline_block)]
            >>  qi::raw[*(qi::char_ - "``")]        [member_assign(&quickbook::code::content)]
            >>  "``"
            ;

        qi::rule<iterator, quickbook::code()>& inline_code_impl = store_.create();
        qi::rule<iterator, std::string()>& inline_code_block = store_.create();

        inline_code = inline_code_impl [actions.process];

        inline_code_impl =
                '`'
            >>  position                            [member_assign(&quickbook::code::position)]
                                                    [member_assign(&quickbook::code::flow, quickbook::code::inline_)]
            >>  inline_code_block                   [member_assign(&quickbook::code::content)]
            >>  '`'
            ;

        inline_code_block =
            qi::raw
            [   *(  ~qi::char_('`') -
                    (qi::eol >> *qi::blank >> qi::eol)  // Make sure that we don't go
                )
                >>  &qi::lit('`')
            ]
            ;
    }
}
