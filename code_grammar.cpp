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

    struct code_grammar_local
    {
        qi::rule<iterator, quickbook::code()> indented_code;
        qi::rule<iterator> code_line;
        qi::rule<iterator, quickbook::code()> code_block1;
        qi::rule<iterator, quickbook::code()> code_block2;
        qi::rule<iterator, quickbook::code()> inline_code;
        qi::rule<iterator, std::string()> inline_code_block;
    };

    void quickbook_grammar::impl::init_code()
    {
        code_grammar_local& local = store_.create();
    
        // Indented code
    
        indented_code = local.indented_code [actions.process];

        local.indented_code =
                position                                [member_assign(&quickbook::code::position)]
                                                        [member_assign(&quickbook::code::flow, quickbook::code::block)]
            >>  qi::raw[local.code_line >> *(*eol >> local.code_line)]
                                                        [member_assign(&quickbook::code::content)]
            >>  +eol
            ;

        local.code_line =
                qi::char_(" \t")
            >>  *(qi::char_ - eol)
            >>  eol
            ;

        code_block = (local.code_block1 | local.code_block2) [actions.process];

        local.code_block1
            =   "```"
            >>  position                            [member_assign(&quickbook::code::position)]
                                                    [member_assign(&quickbook::code::flow, quickbook::code::inline_block)]
            >>  qi::raw[*(qi::char_ - "```")]       [member_assign(&quickbook::code::content)]
            >>  "```"
            ;

        local.code_block2
            =   "``"
            >>  position                            [member_assign(&quickbook::code::position)]
                                                    [member_assign(&quickbook::code::flow, quickbook::code::inline_block)]
            >>  qi::raw[*(qi::char_ - "``")]        [member_assign(&quickbook::code::content)]
            >>  "``"
            ;

        inline_code = local.inline_code [actions.process];

        local.inline_code =
                '`'
            >>  position                            [member_assign(&quickbook::code::position)]
                                                    [member_assign(&quickbook::code::flow, quickbook::code::inline_)]
            >>  local.inline_code_block             [member_assign(&quickbook::code::content)]
            >>  '`'
            ;

        local.inline_code_block =
            qi::raw
            [   *(  ~qi::char_('`') -
                    (qi::eol >> *qi::blank >> qi::eol)  // Make sure that we don't go
                )
                >>  &qi::lit('`')
            ]
            ;
    }
}
