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

#include "./detail/quickbook.hpp"
#include "./detail/utils.hpp"
#include "./grammars.hpp"
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_chset.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>
#include <boost/spirit/include/classic_if.hpp>
#include <boost/spirit/include/classic_symbols.hpp>

namespace quickbook
{
    using namespace boost::spirit;

    template <typename Actions, bool skip_initial_spaces>
    template <typename Scanner>
    block_grammar<Actions, skip_initial_spaces>::
        definition<Scanner>::definition(block_grammar const& self)
        : no_eols(true)
        , common(self.actions, no_eols)
    {
        using detail::var;
        Actions& actions = self.actions;

        if (skip_initial_spaces)
        {
            start_ =
                *(classic::space_p | comment) >> blocks >> blank
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
            *(classic::space_p | comment)
            ;

        blank =
            *(classic::blank_p | comment)
            ;

        eol = blank >> classic::eol_p
            ;

        phrase_end =
            ']' |
            classic::if_p(var(no_eols))
            [
                eol >> eol                      // Make sure that we don't go
            ]                                   // past a single block, except
            ;                                   // when preformatted.

        hard_space =
            (classic::eps_p - (classic::alnum_p | '_')) >> space
                                                // must not be preceded by
            ;                                   // alpha-numeric or underscore

        comment =
            "[/" >> *(dummy_block | (classic::anychar_p - ']')) >> ']'
            ;

        dummy_block =
            '[' >> *(dummy_block | (classic::anychar_p - ']')) >> ']'
            ;

        hr =
            classic::str_p("----")
            >> *(classic::anychar_p - eol)
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
                |   classic::eps_p              [actions.error]
                )
            ;
        
        element_id =
                ':'
            >>
                (
                    classic::if_p(qbk_since(105u))
                                                [space]
                >>  (+(classic::alnum_p | '_')) [classic::assign_a(actions.element_id)]
                |   classic::eps_p              [actions.element_id_warning]
                                                [classic::assign_a(actions.element_id)]
                )
            | classic::eps_p                    [classic::assign_a(actions.element_id)]
            ;
        
        element_id_1_5 =
                classic::if_p(qbk_since(105u)) [
                    element_id
                ]
                .else_p [
                    classic::eps_p              [classic::assign_a(actions.element_id)]
                ]
                ;

        begin_section =
               "section"
            >> hard_space
            >> element_id
            >> phrase                           [actions.begin_section]
            ;

        end_section =
            classic::str_p("endsect")           [actions.end_section]
            ;

        headings =
            h1 | h2 | h3 | h4 | h5 | h6 | h
            ;

        h = "heading" >> hard_space >> phrase   [actions.h];
        h1 = "h1" >> hard_space >> phrase       [actions.h1];
        h2 = "h2" >> hard_space >> phrase       [actions.h2];
        h3 = "h3" >> hard_space >> phrase       [actions.h3];
        h4 = "h4" >> hard_space >> phrase       [actions.h4];
        h5 = "h5" >> hard_space >> phrase       [actions.h5];
        h6 = "h6" >> hard_space >> phrase       [actions.h6];

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
            >> classic::eps_p
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
            "pre" >> hard_space                 [classic::assign_a(no_eols, false_)]
            >> !eol >> phrase                   [actions.preformatted]
            >> classic::eps_p                   [classic::assign_a(no_eols, true_)]
            ;

        macro_identifier =
            +(classic::anychar_p - (classic::space_p | ']'))
            ;

        def_macro =
            "def" >> hard_space
            >> macro_identifier                 [actions.macro_identifier]
            >> blank >> phrase                  [actions.macro_definition]
            ;

        identifier =
            (classic::alpha_p | '_') >> *(classic::alnum_p | '_')
            ;

        template_id =
            identifier | (classic::punct_p - (classic::ch_p('[') | ']'))
            ;

        template_ =
            "template"
            >> hard_space >> template_id        [classic::push_back_a(actions.template_info)]
            >>
            !(
                space >> '['
                >> *(
                        space >> template_id    [classic::push_back_a(actions.template_info)]
                    )
                >> space >> ']'
            )
            >> template_body                    [actions.template_body]
            ;

        template_body =
           *(('[' >> template_body >> ']') | (classic::anychar_p - ']'))
            >> space >> classic::eps_p(']')
            ;

        variablelist =
            "variablelist"
            >>  (classic::eps_p(*classic::blank_p >> classic::eol_p) | hard_space)
            >>  (*(classic::anychar_p - eol))   [classic::assign_a(actions.table_title)]
            >>  +eol
            >>  *varlistentry
            >>  classic::eps_p                  [actions.variablelist]
            ;

        varlistentry =
            space
            >>  classic::ch_p('[')              [actions.start_varlistentry]
            >>
            (
                (
                    varlistterm
                    >> +varlistitem
                    >>  classic::ch_p(']')      [actions.end_varlistentry]
                    >>  space
                )
                | classic::eps_p                [actions.error]
            )
            ;

        varlistterm =
            space
            >>  classic::ch_p('[')              [actions.start_varlistterm]
            >>
            (
                (
                    phrase
                    >>  classic::ch_p(']')      [actions.end_varlistterm]
                    >>  space
                )
                | classic::eps_p                [actions.error]
            )
            ;

        varlistitem =
            space
            >>  classic::ch_p('[')              [actions.start_varlistitem]
            >>
            (
                (
                    inside_paragraph
                    >>  classic::ch_p(']')      [actions.end_varlistitem]
                    >>  space
                )
                | classic::eps_p                [actions.error]
            )
            ;

        table =
            "table"
            >>  (classic::eps_p(*classic::blank_p >> classic::eol_p) | hard_space)
            >> element_id_1_5
            >>  (classic::eps_p(*classic::blank_p >> classic::eol_p) | space)
            >>  (*(classic::anychar_p - eol))   [classic::assign_a(actions.table_title)]
            >>  +eol
            >>  *table_row
            >>  classic::eps_p                  [actions.table]
            ;

        table_row =
            space
            >>  classic::ch_p('[')              [actions.start_row]
            >>
            (
                (
                    *table_cell
                    >>  classic::ch_p(']')      [actions.end_row]
                    >>  space
                )
                | classic::eps_p                [actions.error]
            )
            ;

        table_cell =
            space
            >>  classic::ch_p('[')              [actions.start_cell]
            >>
            (
                (
                    inside_paragraph
                    >>  classic::ch_p(']')      [actions.end_cell]
                    >>  space
                )
                | classic::eps_p                [actions.error]
            )
            ;

        xinclude =
               "xinclude"
            >> hard_space
            >> (*(classic::anychar_p -
                    phrase_end))                [actions.xinclude]
            ;

        import =
               "import"
            >> hard_space
            >> (*(classic::anychar_p -
                    phrase_end))                [actions.import]
            ;

        include =
               "include"
            >> hard_space
            >>
           !(
                ':'
                >> (*((classic::alnum_p | '_') - classic::space_p))[classic::assign_a(actions.include_doc_id)]
                >> space
            )
            >> (*(classic::anychar_p -
                    phrase_end))                [actions.include]
            ;

        code =
            (
                code_line
                >> *(*eol >> code_line)
            )                                   [actions.code]
            >> +eol
            ;

        code_line =
            ((classic::ch_p(' ') | '\t'))
            >> *(classic::anychar_p - eol) >> eol
            ;

        list =
            classic::eps_p(classic::ch_p('*') | '#') >>
           +(
                (*classic::blank_p
                >> (classic::ch_p('*') | '#'))  [actions.list_format]
                >> *classic::blank_p
                >> list_item
            )                                   [actions.list_item]
            ;

        list_item =
           *(   common
            |   (classic::anychar_p -
                    (   classic::eol_p >> *classic::blank_p >> classic::eps_p(classic::ch_p('*') | '#')
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
            |   (classic::anychar_p -           // Make sure we don't go past
                    paragraph_end               // a single block.
                )                               [actions.plain_char]
            )
            >> (classic::eps_p('[') | +eol)
            ;

        phrase =
           *(   common
            |   comment
            |   (classic::anychar_p -
                    phrase_end)                 [actions.plain_char]
            )
            ;
    }
}

#endif // BOOST_SPIRIT_QUICKBOOK_BLOCK_HPP


