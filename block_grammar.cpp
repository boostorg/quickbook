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
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include "block_grammar.hpp"
#include "template.hpp"
#include "actions.hpp"
#include "code.hpp"
#include "misc_rules.hpp"

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::paragraph,
    (std::string, content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::list_item,
    (quickbook::file_position, position)
    (std::string, indent)
    (char, mark)
    (std::string, content)
)

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;

    block_grammar::block_grammar(quickbook::actions& actions_)
        : block_grammar::base_type(start, "block")
        , rules_pimpl(new rules(actions_))
        , start(rules_pimpl->start_) {}

    block_grammar::~block_grammar() {}

    block_grammar::rules::rules(quickbook::actions& actions_)
        : actions(actions_), no_eols(true), common(actions, no_eols)
    {
        init_block_markup();

        qi::rule<iterator>& blocks = store_.create();
        qi::rule<iterator, quickbook::code()>& code = store_.create();
        qi::rule<iterator, quickbook::list()>& list = store_.create();
        qi::rule<iterator, quickbook::hr()>& hr = store_.create();
        qi::rule<iterator, quickbook::paragraph()>& paragraph = store_.create();

        start_ =
            blocks >> blank
            ;

        blocks =
           +(   block_markup
            |   code                            [actions.process]
            |   list                            [actions.process]
            |   hr                              [actions.process]
            |   comment >> *eol
            |   paragraph                       [actions.process]
            |   eol
            )
            ;

        // Blocks indicated by text layout (indentation, leading characters etc.)

        qi::rule<iterator>& code_line = store_.create();

        code =
                position
            >>  qi::raw[
                    code_line
                >>  *(*eol >> code_line)
                ]
            >>  +eol
            >>  qi::attr(true)
            ;

        code_line =
                qi::char_(" \t")
            >>  *(qi::char_ - eol)
            >>  eol
            ;

        qi::rule<iterator, quickbook::list_item()>& list_item = store_.create();
        qi::rule<iterator, std::string()>& list_item_content = store_.create();

        list =
                &qi::char_("*#")
            >>  +list_item
            ;
        
        list_item =
                position
            >>  *qi::blank
            >>  qi::char_("*#")
            >>  qi::omit[*qi::blank]
            >>  list_item_content
            ;

        list_item_content =
            qi::eps[actions.phrase_push] >>
           *(   common
            |   (qi::char_ -
                    (   qi::eol >> *qi::blank >> &(qi::char_('*') | '#')
                    |   (eol >> eol)
                    )
                )                               [actions.process]
            )
            >> +eol
            >> qi::eps[actions.phrase_pop]
            ;

        hr =
            qi::omit[
                "----"
            >>  *(qi::char_ - eol)
            >>  +eol
            ] >> qi::attr(quickbook::hr())
            ;

        qi::rule<iterator, std::string()>& paragraph_content = store_.create();
        qi::rule<iterator>& paragraph_end = store_.create();
        qi::symbols<>& paragraph_end_markups = store_.create();

        paragraph = paragraph_content >> qi::attr(nothing());

        paragraph_content =
                qi::eps                         [actions.phrase_push]
            >> *(   common
                |   (qi::char_ -                // Make sure we don't go past
                        paragraph_end           // a single block.
                    )                           [actions.process]
                )
            >>  qi::eps                         [actions.phrase_pop]
            >> (&qi::lit('[') | +eol)
            ;

        paragraph_end =
            '[' >> space >> paragraph_end_markups >> hard_space | eol >> eol
            ;

        paragraph_end_markups =
            "section", "endsect", "h1", "h2", "h3", "h4", "h5", "h6",
            "blurb", ":", "pre", "def", "table", "include", "xinclude",
            "variablelist", "import", "template", "warning", "caution",
            "important", "note", "tip", ":"
            ;
    }
}
