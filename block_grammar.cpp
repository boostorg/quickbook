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
#include <boost/spirit/include/phoenix_operator.hpp>
#include "grammar_impl.hpp"
#include "block.hpp"
#include "actions.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"
#include "state.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;

    struct block_grammar_local
    {
        qi::rule<iterator> blocks;
        qi::rule<iterator, qi::locals<qi::rule<iterator> > > block_markup;
        qi::rule<iterator, qi::rule<iterator>()> block_markup_start;
        qi::rule<iterator, quickbook::list()> list;
        qi::rule<iterator, quickbook::list_item()> list_item;
        qi::rule<iterator, std::string()> list_item_content;
        qi::rule<iterator, quickbook::hr()> hr;
        qi::rule<iterator> paragraph;
        qi::rule<iterator, quickbook::block_separator()> block_separator;
        qi::rule<iterator, quickbook::def_macro> command_line_macro_parse;
    };

    void quickbook_grammar::impl::init_block()
    {
        block_grammar_local& local = store_.create();

        block_start =
            local.blocks >> blank
            ;

        local.blocks =
           *(   local.block_markup
            |   indented_code                   [actions.process]
            |   local.list                      [actions.process]
            |   local.hr                        [actions.process]
            |   local.block_separator           [actions.process]
            |   eol
            |   local.paragraph
            )
            ;

        // Block markup

        local.block_markup
            =   local.block_markup_start        [qi::_a = qi::_1]
            >>  lazy(qi::_a)
            >>  (   (space >> ']' >> +eol)
                |   error
                )
            ;

        local.block_markup_start
            =   '['
            >>  space
            >>  (   block_keyword_rules >> !(qi::alnum | '_')
                |   block_symbol_rules
                )
            ;

        // List

        local.list =
                &qi::char_("*#")
            >>  +local.list_item
            ;
        
        local.list_item =
                position                                [member_assign(&quickbook::list_item::position)]
            >>  (*qi::blank)                            [member_assign(&quickbook::list_item::indent)]
            >>  qi::char_("*#")                         [member_assign(&quickbook::list_item::mark)]
            >>  *qi::blank
            >>  local.list_item_content                 [member_assign(&quickbook::list_item::content)]
            ;

        local.list_item_content =
            qi::eps[actions.phrase_push] >>
           *(   common
            |   (qi::char_ -
                    (   qi::eol >> *qi::blank >> &(qi::char_('*') | '#')
                    |   (eol >> *qi::blank >> qi::eol)
                    )
                )                                   [actions.process]
            )
            >> +eol
            >> qi::eps[actions.phrase_pop]
            ;

        // Horizontol rule

        local.hr =
            qi::omit[
                "----"
            >>  *(qi::char_ - eol)
            >>  +eol
            ] >> qi::attr(quickbook::hr())
            ;

        // Paragraph

        local.paragraph =
               +(   common
                |   (qi::char_ - (local.block_separator | local.block_markup_start))
                                                        [actions.process]
                )
            ;

        // Define block_separator using qi::eol/qi::blank rather than 'eol'
        // because we don't want any comments in the blank line.

        local.block_separator =
                qi::attr(quickbook::block_separator())
            >>  qi::omit
                [   qi::eol >> *qi::blank >> qi::eol
                ]
            ;

        // Parse command line macro definition. This is more here out of
        // convenience than anything.

        command_line_macro = local.command_line_macro_parse [actions.process];

        local.command_line_macro_parse =
                space
            >>  macro_identifier
            >>  space
            >>  (   '='
                >>  space
                >>  simple_phrase
                >>  space
                )
                |   qi::attr("")
            ;

        // Error

        error = position                            [actions.error];

        // Block contents

        inside_paragraph =
                qi::eps                             [actions.block_push][actions.phrase_push]
            >>  *(  common
                |   (qi::char_ - phrase_end)        [actions.process]
                |   local.block_separator           [actions.process]
                )
            >>  qi::attr(quickbook::block_separator())
                                                    [actions.process]
            >>  qi::eps                             [actions.phrase_pop][actions.block_pop]
            ;

        // Identifiers

        element_id =
            -(  ':'
            >>  -(qi::eps(qbk_since(105u)) >> space)
            >>  (
                    qi::raw[+(qi::alnum | '_')]     [qi::_val = qi::_1]
                |   position                        [actions.element_id_warning]
                )
            )
            ;
    }
}
