/*=============================================================================
    Copyright (c) 2002 2004  2006Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_BLOCK_HPP)
#define BOOST_SPIRIT_QUICKBOOK_BLOCK_HPP

#include "./grammars.hpp"
#include "./detail/quickbook.hpp"
#include "./detail/utils.hpp"
#include "./parse_utils.hpp"
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>

namespace quickbook
{
    using namespace boost::spirit;
    namespace ph = boost::phoenix;

    template <typename Iterator, typename Actions, bool skip_initial_spaces>
    struct block_grammar<Iterator, Actions, skip_initial_spaces>::rules
    {
        rules(Actions& actions_);

        Actions& actions;
        bool no_eols;
        phrase_grammar<Iterator, Actions> common;
        qi::symbols<>   paragraph_end_markups;
        qi::rule<Iterator>
                        start_, blocks, block_markup, code, code_line,
                        paragraph, space, blank, comment, headings, h, h1, h2,
                        h3, h4, h5, h6, hr, blurb, blockquote, admonition,
                        phrase, list, phrase_end, ordered_list, def_macro,
                        macro_identifier, table, table_row, variablelist,
                        varlistentry, varlistterm, varlistitem, table_cell,
                        preformatted, list_item, begin_section, end_section,
                        xinclude, include, hard_space, eol, paragraph_end,
                        template_, template_id, template_formal_arg,
                        template_body, identifier, dummy_block, import,
                        inside_paragraph;
        qi::rule<Iterator, boost::optional<std::string>()>  element_id, element_id_1_5;
    };

    template <typename Iterator, typename Actions, bool skip_initial_spaces>
    block_grammar<Iterator, Actions, skip_initial_spaces>::block_grammar(Actions& actions_)
        : block_grammar::base_type(start, "block")
        , rules_pimpl(new rules(actions_))
        , start(rules_pimpl->start_) {}

    template <typename Iterator, typename Actions, bool skip_initial_spaces>
    block_grammar<Iterator, Actions, skip_initial_spaces>::~block_grammar() {}

    template <typename Iterator, typename Actions, bool skip_initial_spaces>
    block_grammar<Iterator, Actions, skip_initial_spaces>::rules::rules(Actions& actions_)
        : actions(actions_), no_eols(true), common(actions, no_eols)
    {
        if (skip_initial_spaces)
        {
            start_ =
                *(qi::space | comment) >> blocks >> blank
                ;
        }
        else
        {
            start_ =
                blocks >> blank
                ;
        }

        blocks =
           +(   block_markup
            |   code
            |   list                            [actions.list]
            |   hr                              [actions.hr]
            |   comment >> *eol
            |   paragraph                       [actions.paragraph]
            |   eol
            )
            ;

        space =
            *(qi::space | comment)
            ;

        blank =
            *(qi::blank | comment)
            ;

        eol = blank >> qi::eol
            ;

        phrase_end =
            ']' |
            qi::eps(ph::ref(no_eols)) >>
                eol >> eol                      // Make sure that we don't go
                                                // past a single block, except
            ;                                   // when preformatted.

        hard_space =
            (qi::eps - (qi::alnum | '_')) >> space  // must not be preceded by
            ;                                      // alpha-numeric or underscore

        comment =
            "[/" >> *(dummy_block | (qi::char_ - ']')) >> ']'
            ;

        dummy_block =
            '[' >> *(dummy_block | (qi::char_ - ']')) >> ']'
            ;

        hr =
            qi::lit("----")
            >> *(qi::char_ - eol)
            >> +eol
            ;

        block_markup =
                '[' >> space
            >>  (   begin_section
                |   end_section
                |   headings
                |   blurb
                |   blockquote
                |   admonition
                |   preformatted
                |   def_macro
                |   table
                |   variablelist
                |   xinclude
                |   include
                |   import
                |   template_
                )
            >>  (   (space >> ']' >> +eol)
                |   qi::raw[qi::eps]            [actions.error]
                )
            ;
        
        element_id =
            -(
                ':'
            >>  -(qi::eps(qbk_since(105u)) >> space) 
            >>  (
                    (+(qi::alnum | qi::char_('_'))) [qi::_val = as_string(qi::_1)]
                |   qi::raw[qi::eps]                [actions.element_id_warning]
                )
            );

        
        element_id_1_5 =
            -(qi::eps(qbk_since(105u)) >> element_id [qi::_val = qi::_1]);

        begin_section = (
               "section"
            >> hard_space
            >> element_id
            >> qi::raw[phrase])                 [ph::bind(actions.begin_section, qi::_1, qi::_2)]
            ;

        end_section =
            qi::raw["endsect"]                  [actions.end_section]
            ;

        headings =
            h1 | h2 | h3 | h4 | h5 | h6 | h
            ;

        h = "heading" >> hard_space >> qi::raw[phrase]   [actions.h];
        h1 = "h1" >> hard_space >> qi::raw[phrase]       [actions.h1];
        h2 = "h2" >> hard_space >> qi::raw[phrase]       [actions.h2];
        h3 = "h3" >> hard_space >> qi::raw[phrase]       [actions.h3];
        h4 = "h4" >> hard_space >> qi::raw[phrase]       [actions.h4];
        h5 = "h5" >> hard_space >> qi::raw[phrase]       [actions.h5];
        h6 = "h6" >> hard_space >> qi::raw[phrase]       [actions.h6];

        static const bool true_ = true;
        static const bool false_ = false;

        inside_paragraph =
            phrase                              [actions.inside_paragraph]
            >> *(
                eol >> eol >> phrase            [actions.inside_paragraph]
            )
            ;

        blurb =
            "blurb" >> hard_space
            >> inside_paragraph                 [actions.blurb]
            >> qi::eps
            ;

        blockquote =
            ':' >> blank >>
            inside_paragraph                    [actions.blockquote]
            ;

        admonition =
            "warning" >> blank >>
            inside_paragraph                    [actions.warning]
            |
            "caution" >> blank >>
            inside_paragraph                    [actions.caution]
            |
            "important" >> blank >>
            inside_paragraph                    [actions.important]
            |
            "note" >> blank >>
            inside_paragraph                    [actions.note]
            |
            "tip" >> blank >>
            inside_paragraph                    [actions.tip]
            ;

        preformatted =
            "pre" >> hard_space                 [ph::ref(no_eols) = false_]
            >> -eol >> phrase                   [actions.preformatted]
            >> qi::eps                          [ph::ref(no_eols) = true_]
            ;

        macro_identifier =
            +(qi::char_ - (qi::space | ']'))
            ;

        def_macro =
            "def" >> hard_space
            >> qi::raw[macro_identifier]        [actions.macro_identifier]
            >> blank >> phrase                  [actions.macro_definition]
            ;

        identifier =
            (qi::alpha | '_') >> *(qi::alnum | '_')
            ;

        template_id =
            identifier | (qi::punct - (qi::char_('[') | ']'))
            ;

        template_ =
            "template"
            >> hard_space >> qi::raw[template_id]
                                                [ph::push_back(ph::ref(actions.template_info), as_string(qi::_1))]
            >>
            -(
                space >> '['
                >> *(
                        space >> qi::raw[template_id]
                                                [ph::push_back(ph::ref(actions.template_info), as_string(qi::_1))]
                    )
                >> space >> ']'
            )
            >> qi::raw[template_body]           [actions.template_body]
            ;

        template_body =
           *(('[' >> template_body >> ']') | (qi::char_ - ']'))
            >> space >> &qi::lit(']')
            ;

        variablelist = (
            "variablelist"
            >>  qi::omit[&(*qi::blank >> qi::eol) | hard_space]
            >>  *(qi::char_ - eol)
            >>  +eol
            >>  *varlistentry
            )                                   [ph::bind(actions.variablelist, as_string(qi::_1))]
            ;

        varlistentry =
            space
            >>  qi::char_('[')                  [actions.start_varlistentry]
            >>
            (
                (
                    varlistterm
                    >> +varlistitem
                    >>  qi::char_(']')          [actions.end_varlistentry]
                    >>  space
                )
                | qi::raw[qi::eps]              [actions.error]
            )
            ;

        varlistterm =
            space
            >>  qi::char_('[')                  [actions.start_varlistterm]
            >>
            (
                (
                    phrase
                    >>  qi::char_(']')          [actions.end_varlistterm]
                    >>  space
                )
                | qi::raw[qi::eps]              [actions.error]
            )
            ;

        varlistitem =
            space
            >>  qi::char_('[')                  [actions.start_varlistitem]
            >>
            (
                (
                    inside_paragraph
                    >>  qi::char_(']')          [actions.end_varlistitem]
                    >>  space
                )
                | qi::raw[qi::eps]              [actions.error]
            )
            ;

        table = (
            "table"
            >>  (&(*qi::blank >> qi::eol) | hard_space)
            >>  element_id_1_5
            >>  (&(*qi::blank >> qi::eol) | space)
            >>  *(qi::char_ - eol)
            >>  +eol
            >>  *table_row
            )                                   [ph::bind(actions.table, qi::_1, as_string(qi::_2))]
            ;

        table_row =
            space
            >>  qi::char_('[')                  [actions.start_row]
            >>
            (
                (
                    *table_cell
                    >>  qi::char_(']')          [actions.end_row]
                    >>  space
                )
                | qi::raw[qi::eps]              [actions.error]
            )
            ;

        table_cell =
            space
            >>  qi::char_('[')                  [actions.start_cell]
            >>
            (
                (
                    inside_paragraph
                    >>  qi::char_(']')          [actions.end_cell]
                    >>  space
                )
                | qi::raw[qi::eps]              [actions.error]
            )
            ;

        xinclude =
               "xinclude"
            >> hard_space
            >> qi::raw[*(qi::char_ -
                    phrase_end)]                [actions.xinclude]
            ;

        import =
               "import"
            >> hard_space
            >> qi::raw[*(qi::char_ -
                    phrase_end)]                [actions.import]
            ;

        include =
               "include"
            >> hard_space
            >>
           -(
                ':'
                >> qi::raw[*((qi::alnum | '_') - qi::space)]
                                                [ph::ref(actions.include_doc_id) = as_string(qi::_1)]
                >> space
            )
            >> qi::raw[*(qi::char_ -
                    phrase_end)]                [actions.include]
            ;

        code =
            qi::raw[
                code_line
                >> *(*eol >> code_line)
            ]                                   [actions.code]
            >> +eol
            ;

        code_line =
            ((qi::char_(' ') | '\t'))
            >> *(qi::char_ - eol) >> eol
            ;

        list =
            &(qi::char_('*') | '#') >>
           +qi::raw[
                qi::raw[*qi::blank
                >> (qi::char_('*') | '#')]
                                                [actions.list_format]
                >> *qi::blank
                >> list_item
            ]                                   [actions.list_item]
            ;

        list_item =
           *(   common
            |   (qi::char_ -
                    (   qi::eol >> *qi::blank >> &(qi::char_('*') | '#')
                    |   (eol >> eol)
                    )
                )                               [actions.plain_char]
            )
            >> +eol
            ;

        paragraph_end_markups =
            "section", "endsect", "h1", "h2", "h3", "h4", "h5", "h6",
            "blurb", ":", "pre", "def", "table", "include", "xinclude",
            "variablelist", "import", "template", "warning", "caution",
            "important", "note", "tip", ":"
            ;

        paragraph_end =
            '[' >> space >> paragraph_end_markups >> hard_space | eol >> eol
            ;

        paragraph =
           *(   common
            |   (qi::char_ -                    // Make sure we don't go past
                    paragraph_end               // a single block.
                )                               [actions.plain_char]
            )
            >> (&qi::lit('[') | +eol)
            ;

        phrase =
           *(   common
            |   comment
            |   (qi::char_ -
                    phrase_end)                 [actions.plain_char]
            )
            ;
    }
}

#endif // BOOST_SPIRIT_QUICKBOOK_BLOCK_HPP


