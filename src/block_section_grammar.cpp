/*=============================================================================
    Copyright (c) 2002 2004  2006Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include "grammar_impl.hpp"
#include "block.hpp"
#include "actions.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;
    
    struct block_section_grammar_local
    {
        qi::rule<iterator, quickbook::title()> title_phrase;
        qi::rule<iterator, quickbook::begin_section()> begin_section;
        qi::rule<iterator, quickbook::end_section()> end_section;
        qi::rule<iterator, quickbook::heading(int)> heading_impl;
        
        qi::rule<iterator> section_block, endsect_block,
            h1, h2, h3, h4, h5, h6, heading;
    };

    void quickbook_grammar::impl::init_block_section()
    {
        block_section_grammar_local& local = store_.create();

        // Sections

        block_keyword_rules.add("section", &local.section_block);
        block_keyword_rules.add("endsect", &local.endsect_block);

        local.section_block = local.begin_section[actions.process];
        local.endsect_block = local.end_section[actions.process];

        local.begin_section =
                space
            >>  element_id                          [member_assign(&quickbook::begin_section::id)]
            >>  space
            >>  local.title_phrase                  [member_assign(&quickbook::begin_section::content)]
            ;

        local.end_section =
                space
            >>  position                            [member_assign(&quickbook::end_section::position)]
            ;

        // Headings

        block_keyword_rules.add
            ("h1", &local.h1)
            ("h2", &local.h2)
            ("h3", &local.h3)
            ("h4", &local.h4)
            ("h5", &local.h5)
            ("h6", &local.h6)
            ("heading", &local.heading);

        local.h1 = local.heading_impl(1) [actions.process];
        local.h2 = local.heading_impl(2) [actions.process];
        local.h3 = local.heading_impl(3) [actions.process];
        local.h4 = local.heading_impl(4) [actions.process];
        local.h5 = local.heading_impl(5) [actions.process];
        local.h6 = local.heading_impl(6) [actions.process];
        local.heading = local.heading_impl(-1) [actions.process];

        local.heading_impl =
                qi::attr(qi::_r1)                   [member_assign(&quickbook::heading::level)]
            >>  space
            >>  -(  qi::eps(qbk_since(106u))
                >>  element_id                      [member_assign(&quickbook::heading::id)]
                >>  space
                )
            >>  local.title_phrase                  [member_assign(&quickbook::heading::content)]
                ;

        local.title_phrase =
            qi::raw[
                phrase                              [member_assign(&quickbook::title::content)]
            ]                                       [member_assign(&quickbook::title::raw)]
            ;
    }
}