/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_PHRASE_HPP)
#define BOOST_SPIRIT_QUICKBOOK_PHRASE_HPP

#include "./grammars.hpp"
#include "./detail/quickbook.hpp"
#include "./detail/utils.hpp"
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_chset.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>
#include <boost/spirit/include/classic_clear_actor.hpp>
#include <boost/spirit/include/classic_if.hpp>

namespace quickbook
{
    using namespace boost::spirit;

    template <typename Rule, typename Action>
    inline void
    simple_markup(
        Rule& simple
      , char mark
      , Action const& action
      , Rule const& close
    )
    {
        simple =
            mark >>
            (
                (
                    classic::graph_p             // A single char. e.g. *c*
                    >> classic::eps_p(mark
                        >> (classic::space_p | classic::punct_p | classic::end_p))
                                                // space_p, punct_p or end_p
                )                               // must follow mark
            |
                (   classic::graph_p >>         // graph_p must follow mark
                    *(classic::anychar_p -
                        (   (classic::graph_p >> mark) 
                                                // Make sure that we don't go
                        |   close               // past a single block
                        )
                    ) >> classic::graph_p       // graph_p must precede mark
                    >> classic::eps_p(mark
                        >> (classic::space_p | classic::punct_p | classic::end_p))
                                                // space_p, punct_p or end_p
                )                               // must follow mark
            )                                   [action]
            >> mark
            ;
    }

    template <typename Actions>
    template <typename Scanner>
    phrase_grammar<Actions>::definition<Scanner>::definition(
        phrase_grammar const& self)
    {
        using detail::var;
        Actions& actions = self.actions;

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
            classic::if_p(var(self.no_eols))
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

        common =
                macro
            |   phrase_markup
            |   code_block
            |   inline_code
            |   simple_format
            |   escape
            |   comment
            ;

        macro =
            classic::eps_p(actions.macro        // must not be followed by
                                                // alpha or underscore
                >> (classic::eps_p - (classic::alpha_p | '_')))
            >> actions.macro                    [actions.do_macro]
            ;

        static const bool true_ = true;
        static const bool false_ = false;

        template_ =
            (
                classic::ch_p('`')              [classic::assign_a(actions.template_escape,true_)]
                |
                classic::eps_p                  [classic::assign_a(actions.template_escape,false_)]
            )
            >>
            ( (
                (classic::eps_p(classic::punct_p)
                    >> actions.templates.scope
                )                               [classic::push_back_a(actions.template_info)]
                >> !template_args
            ) | (
                (actions.templates.scope
                    >> classic::eps_p
                )                               [classic::push_back_a(actions.template_info)]
                >> !(hard_space
                    >> template_args)
            ) )
            >> classic::eps_p(']')
            ;

        template_args =
            classic::if_p(qbk_since(105u)) [
                template_args_1_5
            ].else_p [
                template_args_1_4
            ]
            ;

        template_args_1_4 =
            template_arg_1_4                    [classic::push_back_a(actions.template_info)]
            >> *(
                    ".." >> template_arg_1_4    [classic::push_back_a(actions.template_info)]
                )
            ;

        template_arg_1_4 =
            +(brackets_1_4 | (classic::anychar_p - (classic::str_p("..") | ']')))
            ;

        brackets_1_4 =
            '[' >> +template_arg_1_4 >> ']'
            ;

        template_args_1_5 =
            template_arg_1_5                    [classic::push_back_a(actions.template_info)]
            >> *(
                    ".." >> template_arg_1_5    [classic::push_back_a(actions.template_info)]
                )
            ;

        template_arg_1_5 =
            +(brackets_1_5 | ('\\' >> classic::anychar_p) | (classic::anychar_p - (classic::str_p("..") | '[' | ']')))
            ;

        template_inner_arg_1_5 =
            +(brackets_1_5 | ('\\' >> classic::anychar_p) | (classic::anychar_p - (classic::str_p('[') | ']')))
            ;

        brackets_1_5 =
            '[' >> +template_inner_arg_1_5 >> ']'
            ;

        inline_code =
            '`' >>
            (
               *(classic::anychar_p -
                    (   '`'
                    |   (eol >> eol)            // Make sure that we don't go
                    )                           // past a single block
                ) >> classic::eps_p('`')
            )                                   [actions.inline_code]
            >>  '`'
            ;

        code_block =
                (
                    "```" >>
                    (
                       *(classic::anychar_p - "```")
                            >> classic::eps_p("```")
                    )                           [actions.code_block]
                    >>  "```"
                )
            |   (
                    "``" >>
                    (
                       *(classic::anychar_p - "``")
                            >> classic::eps_p("``")
                    )                           [actions.code_block]
                    >>  "``"
                )
            ;

        simple_format =
                simple_bold
            |   simple_italic
            |   simple_underline
            |   simple_teletype
            ;

        simple_markup(simple_bold,
            '*', actions.simple_bold, phrase_end);
        simple_markup(simple_italic,
            '/', actions.simple_italic, phrase_end);
        simple_markup(simple_underline,
            '_', actions.simple_underline, phrase_end);
        simple_markup(simple_teletype,
            '=', actions.simple_teletype, phrase_end);

        phrase =
           *(   common
            |   comment
            |   (classic::anychar_p - phrase_end)
                                                [actions.plain_char]
            )
            ;

        phrase_markup =
                '['
            >>  (   cond_phrase
                |   image
                |   url
                |   link
                |   anchor
                |   source_mode
                |   funcref
                |   classref
                |   memberref
                |   enumref
                |   macroref
                |   headerref
                |   conceptref
                |   globalref
                |   bold
                |   italic
                |   underline
                |   teletype
                |   strikethrough
                |   quote
                |   replaceable
                |   footnote
                |   template_                   [actions.do_template]
                |   classic::str_p("br")        [actions.break_]
                )
            >>  ']'
            ;

        escape =
                classic::str_p("\\n")           [actions.break_]
            |   "\\ "                           // ignore an escaped char
            |   '\\' >> classic::punct_p        [actions.raw_char]
            |   (
                    ("'''" >> !eol)             [actions.escape_pre]
                >>  *(classic::anychar_p - "'''")
                                                [actions.raw_char]
                >>  classic::str_p("'''")       [actions.escape_post]
                )
            ;

        macro_identifier =
            +(classic::anychar_p - (classic::space_p | ']'))
            ;

        cond_phrase =
                '?' >> blank
            >>  macro_identifier                [actions.cond_phrase_pre]
            >>  (!phrase)                       [actions.cond_phrase_post]
            ;

        image =
                '$' >> blank                    [classic::clear_a(actions.attributes)]
            >>  classic::if_p(qbk_since(105u)) [
                        (+(
                            *classic::space_p
                        >>  +(classic::anychar_p - (classic::space_p | phrase_end | '['))
                        ))                       [classic::assign_a(actions.image_fileref)]
                    >>  hard_space
                    >>  *(
                            '['
                        >>  (*(classic::alnum_p | '_'))  [classic::assign_a(actions.attribute_name)]
                        >>  space
                        >>  (*(classic::anychar_p - (phrase_end | '[')))
                                                [actions.attribute]
                        >>  ']'
                        >>  space
                        )
                ].else_p [
                        (*(classic::anychar_p -
                            phrase_end))        [classic::assign_a(actions.image_fileref)]
                ]
            >>  classic::eps_p(']')             [actions.image]
            ;
            
        url =
                '@'
            >>  (*(classic::anychar_p -
                    (']' | hard_space)))        [actions.url_pre]
            >>  (   classic::eps_p(']')
                |   (hard_space >> phrase)
                )                               [actions.url_post]
            ;

        link =
                "link" >> hard_space
            >>  (*(classic::anychar_p -
                    (']' | hard_space)))        [actions.link_pre]
            >>  (   classic::eps_p(']')
                |   (hard_space >> phrase)
                )                               [actions.link_post]
            ;

        anchor =
                '#'
            >>  blank
            >>  (   *(classic::anychar_p -
                        phrase_end)
                )                               [actions.anchor]
            ;

        funcref =
            "funcref" >> hard_space
            >>  (*(classic::anychar_p -
                    (']' | hard_space)))        [actions.funcref_pre]
            >>  (   classic::eps_p(']')
                |   (hard_space >> phrase)
                )                               [actions.funcref_post]
            ;

        classref =
            "classref" >> hard_space
            >>  (*(classic::anychar_p -
                    (']' | hard_space)))        [actions.classref_pre]
            >>  (   classic::eps_p(']')
                |   (hard_space >> phrase)
                )                               [actions.classref_post]
            ;

        memberref =
            "memberref" >> hard_space
            >>  (*(classic::anychar_p -
                    (']' | hard_space)))        [actions.memberref_pre]
            >>  (   classic::eps_p(']')
                |   (hard_space >> phrase)
                )                               [actions.memberref_post]
            ;

        enumref =
            "enumref" >> hard_space
            >>  (*(classic::anychar_p -
                    (']' | hard_space)))        [actions.enumref_pre]
            >>  (   classic::eps_p(']')
                |   (hard_space >> phrase)
                )                               [actions.enumref_post]
            ;

        macroref =
            "macroref" >> hard_space
            >>  (*(classic::anychar_p -
                    (']' | hard_space)))        [actions.macroref_pre]
            >>  (   classic::eps_p(']')
                |   (hard_space >> phrase)
                )                               [actions.macroref_post]
            ;

        headerref =
            "headerref" >> hard_space
            >>  (*(classic::anychar_p -
                    (']' | hard_space)))        [actions.headerref_pre]
            >>  (   classic::eps_p(']')
                |   (hard_space >> phrase)
                )                               [actions.headerref_post]
            ;

        conceptref =
            "conceptref" >> hard_space
            >>  (*(classic::anychar_p -
                    (']' | hard_space)))        [actions.conceptref_pre]
            >>  (   classic::eps_p(']')
                |   (hard_space >> phrase)
                )                               [actions.conceptref_post]
            ;

        globalref =
            "globalref" >> hard_space
            >>  (*(classic::anychar_p -
                    (']' | hard_space)))        [actions.globalref_pre]
            >>  (   classic::eps_p(']')
                |   (hard_space >> phrase)
                )                               [actions.globalref_post]
            ;

        bold =
                classic::ch_p('*')              [actions.bold_pre]
            >>  blank >> phrase                 [actions.bold_post]
            ;

        italic =
                classic::ch_p('\'')             [actions.italic_pre]
            >>  blank >> phrase                 [actions.italic_post]
            ;

        underline =
                classic::ch_p('_')              [actions.underline_pre]
            >>  blank >> phrase                 [actions.underline_post]
            ;

        teletype =
                classic::ch_p('^')              [actions.teletype_pre]
            >>  blank >> phrase                 [actions.teletype_post]
            ;

        strikethrough =
                classic::ch_p('-')              [actions.strikethrough_pre]
            >>  blank >> phrase                 [actions.strikethrough_post]
            ;

        quote =
                classic::ch_p('"')              [actions.quote_pre]
            >>  blank >> phrase                 [actions.quote_post]
            ;

        replaceable =
                classic::ch_p('~')              [actions.replaceable_pre]
            >>  blank >> phrase                 [actions.replaceable_post]
            ;

        source_mode =
            (
                classic::str_p("c++")
            |   "python"
            |   "teletype"
            )                                   [classic::assign_a(actions.source_mode)]
            ;

        footnote =
                classic::str_p("footnote")      [actions.footnote_pre]
            >>  blank >> phrase                 [actions.footnote_post]
            ;
    }

    template <typename Actions>
    template <typename Scanner>
    simple_phrase_grammar<Actions>::definition<Scanner>::definition(
        simple_phrase_grammar const& self)
        : unused(false), common(self.actions, unused)
    {
        Actions& actions = self.actions;

        phrase =
           *(   common
            |   comment
            |   (classic::anychar_p - ']')      [actions.plain_char]
            )
            ;

        comment =
            "[/" >> *(dummy_block | (classic::anychar_p - ']')) >> ']'
            ;

        dummy_block =
            '[' >> *(dummy_block | (classic::anychar_p - ']')) >> ']'
            ;
    }
}

#endif // BOOST_SPIRIT_QUICKBOOK_PHRASE_HPP

