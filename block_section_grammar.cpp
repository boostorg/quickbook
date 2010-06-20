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
    
    // Workaround for clang:
    namespace {
        struct dummmy {
            qi::rule<iterator, raw_string()> a1;
        };
    }

    void quickbook_grammar::impl::init_block_section()
    {
        qi::rule<iterator, quickbook::title()>& title_phrase = store_.create();

        // Sections

        qi::rule<iterator, quickbook::begin_section()>& begin_section = store_.create();
        qi::rule<iterator, quickbook::end_section()>& end_section = store_.create();
        block_keyword_rules.add("section", begin_section[actions.process]);
        block_keyword_rules.add("endsect", end_section[actions.process]);

        begin_section =
                space
            >>  element_id                          [member_assign(&quickbook::begin_section::id)]
            >>  title_phrase                        [member_assign(&quickbook::begin_section::content)]
            ;

        end_section =
                space
            >>  position                            [member_assign(&quickbook::end_section::position)]
            ;

        // Headings

        qi::rule<iterator, quickbook::heading(int)>& heading = store_.create();

        block_keyword_rules.add
            ("h1", heading(1) [actions.process])
            ("h2", heading(2) [actions.process])
            ("h3", heading(3) [actions.process])
            ("h4", heading(4) [actions.process])
            ("h5", heading(5) [actions.process])
            ("h6", heading(6) [actions.process])
            ("heading", heading(-1) [actions.process]);

        heading =
                qi::attr(qi::_r1)                   [member_assign(&quickbook::heading::level)]
            >>  space
            >>  title_phrase                        [member_assign(&quickbook::heading::content)]
                ;

        title_phrase =
            qi::raw[
                phrase                              [member_assign(&quickbook::title::content)]
            ]                                       [member_assign(&quickbook::title::raw)]
            ;
    }
}