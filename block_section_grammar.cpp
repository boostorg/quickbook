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
        qi::rule<iterator, quickbook::heading(int)> heading;
    };

    void quickbook_grammar::impl::init_block_section()
    {
        block_section_grammar_local& local = store_.create();

        // Sections

        block_keyword_rules.add("section", local.begin_section[actions.process]);
        block_keyword_rules.add("endsect", local.end_section[actions.process]);

        local.begin_section =
                space
            >>  element_id                          [member_assign(&quickbook::begin_section::id)]
            >>  local.title_phrase                  [member_assign(&quickbook::begin_section::content)]
            ;

        local.end_section =
                space
            >>  position                            [member_assign(&quickbook::end_section::position)]
            ;

        // Headings

        block_keyword_rules.add
            ("h1", local.heading(1) [actions.process])
            ("h2", local.heading(2) [actions.process])
            ("h3", local.heading(3) [actions.process])
            ("h4", local.heading(4) [actions.process])
            ("h5", local.heading(5) [actions.process])
            ("h6", local.heading(6) [actions.process])
            ("heading", local.heading(-1) [actions.process]);

        local.heading =
                qi::attr(qi::_r1)                   [member_assign(&quickbook::heading::level)]
            >>  space
            >>  local.title_phrase                  [member_assign(&quickbook::heading::content)]
                ;

        local.title_phrase =
            qi::raw[
                phrase                              [member_assign(&quickbook::title::content)]
            ]                                       [member_assign(&quickbook::title::raw)]
            ;
    }
}