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

    void quickbook_grammar::impl::init_block()
    {
        qi::rule<iterator>& block_markup = store_.create();
        qi::rule<iterator>& blocks = store_.create();
        qi::rule<iterator, quickbook::list()>& list = store_.create();
        qi::rule<iterator, quickbook::hr()>& hr = store_.create();
        qi::rule<iterator>& paragraph = store_.create();
        qi::rule<iterator, quickbook::block_separator()>& block_separator = store_.create();

        block_start =
            blocks >> blank
            ;

        blocks =
           +(   block_markup
            |   indented_code                   [actions.process]
            |   list                            [actions.process]
            |   hr                              [actions.process]
            |   block_separator                 [actions.process]
            |   eol
            |   paragraph
            )
            ;

        // Block markup

        qi::rule<iterator, qi::locals<qi::rule<iterator> > >& block_markup_impl = store_.create();

        block_markup =
                '[' >> space
            >>  block_markup_impl
            >>  (   (space >> ']' >> +eol)
                |   error
                )
            ;

        block_markup_impl
            =   (   block_keyword_rules >> !(qi::alnum | '_')
                |   block_symbol_rules
                ) [qi::_a = qi::_1]
                >> lazy(qi::_a)
                ;

        // List

        qi::rule<iterator, quickbook::list_item()>& list_item = store_.create();
        qi::rule<iterator, std::string()>& list_item_content = store_.create();

        list =
                &qi::char_("*#")
            >>  +list_item
            ;
        
        list_item =
                position                                [member_assign(&quickbook::list_item::position)]
            >>  (*qi::blank)                            [member_assign(&quickbook::list_item::indent)]
            >>  qi::char_("*#")                         [member_assign(&quickbook::list_item::mark)]
            >>  *qi::blank
            >>  list_item_content                       [member_assign(&quickbook::list_item::content)]
            ;

        list_item_content =
            qi::eps[actions.phrase_push] >>
           *(   common
            |   (qi::char_ -
                    (   qi::eol >> *qi::blank >> &(qi::char_('*') | '#')
                    |   (eol >> *qi::blank >> qi::eol)
                    )
                )                               [actions.process]
            )
            >> +eol
            >> qi::eps[actions.phrase_pop]
            ;

        // Horizontol rule

        hr =
            qi::omit[
                "----"
            >>  *(qi::char_ - eol)
            >>  +eol
            ] >> qi::attr(quickbook::hr())
            ;

        qi::rule<iterator>& paragraph_end = store_.create();
        qi::symbols<>& paragraph_end_markups = store_.create();

        // Paragraph

        paragraph =
               +(   common
                |   (qi::char_ -                // Make sure we don't go past
                        paragraph_end           // a single block.
                    )                           [actions.process]
                )
            ;

        paragraph_end =
            '[' >> space >> paragraph_end_markups >> hard_space | block_separator
            ;

        // Define block_seperator using qi::eol/qi::blank rather than 'eol'
        // because we don't want any comments in the blank line.

        block_separator =
                qi::attr(quickbook::block_separator())
            >>  qi::omit
                [   qi::eol >> *qi::blank >> qi::eol
                ]
            ;

        paragraph_end_markups =
            "section", "endsect", "h1", "h2", "h3", "h4", "h5", "h6",
            "blurb", ":", "pre", "def", "table", "include", "xinclude",
            "variablelist", "import", "template", "warning", "caution",
            "important", "note", "tip", ":"
            ;

        // Parse command line macro definition. This is more here out of
        // convenience than anything.

        qi::rule<iterator, quickbook::def_macro>& command_line_macro_parse = store_.create();

        command_line_macro = command_line_macro_parse [actions.process];

        command_line_macro_parse =
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

        error =
            qi::raw[qi::eps] [actions.error];

        // Block contents

        qi::rule<iterator>& inside_paragraph2 = store_.create();

        inside_paragraph =
                qi::eps                             [actions.block_push][actions.phrase_push]
            >>  (
                    inside_paragraph2               [actions.process]
                %   block_separator                 [actions.process]
                )
            >>  qi::attr(quickbook::block_separator())
                                                    [actions.process]
            >>  qi::eps                             [actions.phrase_pop][actions.block_pop]
            ;

        inside_paragraph2 =
               *(   common
                |   (qi::char_ - phrase_end)    [actions.process]
                )
            ;

        phrase_attr =
                qi::eps                         [actions.phrase_push]        
            >> *(   common
                |   comment
                |   (qi::char_ - phrase_end)    [actions.process]
                )
            >>  qi::eps                         [actions.phrase_pop]
            ;

        // Make sure that we don't go past a single block, except when
        // preformatted.
        phrase_end =
            ']' | qi::eps(ph::ref(no_eols)) >> eol >> *qi::blank >> qi::eol
            ;

        // Identifiers

        qi::rule<iterator, raw_string()>& element_id_part = store_.create();

        element_id =
            (   ':'
            >>  -(qi::eps(qbk_since(105u)) >> space)
            >>  (
                    element_id_part
                |   qi::omit[
                        qi::raw[qi::eps]        [actions.element_id_warning]
                    ]
                )
            )
            | qi::eps
            ;

        element_id_part = qi::raw[+(qi::alnum | qi::char_('_'))]
                                                [qi::_val = qi::_1];
    }
}
