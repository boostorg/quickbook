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
#include "./parse_utils.hpp"
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_container.hpp>

namespace quickbook
{
    using namespace boost::spirit;
    namespace ph = boost::phoenix;

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
            qi::raw[
                (
                    qi::graph                   // A single char. e.g. *c*
                    >> &(mark
                        >> (qi::space | qi::punct | qi::eoi))
                                                // space, punct or end
                )                               // must follow mark
            |
                (   qi::graph >>                // qi::graph must follow mark
                    *(qi::char_ -
                        (   (qi::graph >> mark) // Make sure that we don't go
                        |   close               // past a single block
                        )
                    ) >> qi::graph              // qi::graph must precede mark
                    >> &(mark
                        >> (qi::space | qi::punct | qi::eoi))
                                                // space, punct or end
                )                               // must follow mark
            ]                                   [action]
            >> mark
            ;
    }

    template <typename Iterator, typename Actions>
    phrase_grammar<Iterator, Actions>::phrase_grammar(Actions& actions, bool& no_eols)
        : phrase_grammar::base_type(common, "phrase"),
        actions(actions),
        no_eols(no_eols)
    {
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
            ;                                   // past a single block, except
                                                // when preformatted.

        hard_space =
            (qi::eps - (qi::alnum | '_')) >> space
            ;                                   // must not be preceded by
                                                // alpha-numeric or underscore

        comment =
            "[/" >> *(dummy_block | (qi::char_ - ']')) >> ']'
            ;

        dummy_block =
            '[' >> *(dummy_block | (qi::char_ - ']')) >> ']'
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
            &(actions.macro                         // must not be followed by
                >> (qi::eps - (qi::alpha | '_')))   // alpha or underscore
            >> actions.macro                        [actions.do_macro]
            ;

        static const bool true_ = true;
        static const bool false_ = false;

        template_ =
            (
                qi::char_('`')                      [ph::ref(actions.template_escape) = true_]
                |
                qi::eps                             [ph::ref(actions.template_escape) = false_]
            )
            >>
            ( (
                qi::raw[&qi::punct >> actions.templates.scope]
                                                    [ph::push_back(ph::ref(actions.template_info), as_string(qi::_1))]
                >> -template_args
            ) | (
                qi::raw[actions.templates.scope]    [ph::push_back(ph::ref(actions.template_info), as_string(qi::_1))]
                >> -(hard_space >> template_args)
            ) )
            >> &qi::lit(']')
            ;

        template_args =
            qi::eps(qbk_before(105u)) >> template_args_1_4
            |
            qi::eps(qbk_since(105u)) >> template_args_1_5
            ;

        template_args_1_4 =
            qi::raw[template_arg_1_4]               [ph::push_back(ph::ref(actions.template_info), as_string(qi::_1))]
            % ".."
            ;

        template_arg_1_4 =
            +(brackets_1_4 | (qi::char_ - (qi::lit("..") | ']')))
            ;

        brackets_1_4 =
            '[' >> +template_arg_1_4 >> ']'
            ;

        template_args_1_5 =
            qi::raw[template_arg_1_5]               [ph::push_back(ph::ref(actions.template_info), as_string(qi::_1))]
            % ".."
            ;

        template_arg_1_5 =
            +(brackets_1_5 | ('\\' >> qi::char_) | (qi::char_ - (qi::lit("..") | '[' | ']')))
            ;

        template_inner_arg_1_5 =
            +(brackets_1_5 | ('\\' >> qi::char_) | (qi::char_ - (qi::lit('[') | ']')))
            ;

        brackets_1_5 =
            '[' >> +template_inner_arg_1_5 >> ']'
            ;

        inline_code =
            '`' >>
            qi::raw[
               *(qi::char_ -
                    (   '`'
                    |   (eol >> eol)                // Make sure that we don't go
                    )                               // past a single block
                ) >> &qi::lit('`')
            ]                                       [actions.inline_code]
            >>  '`'
            ;

        code_block =
                (
                    "```" >>
                    qi::raw[
                       *(qi::char_ - "```")
                            >> &qi::lit("```")
                    ]                               [actions.code_block]
                    >>  "```"
                )
            |   (
                    "``" >>
                    qi::raw[
                       *(qi::char_ - "``")
                            >> &qi::lit("``")
                    ]                               [actions.code_block]
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
            |   (qi::char_ - phrase_end)            [actions.plain_char]
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
                |   qi::raw[template_]              [actions.do_template]
                |   qi::raw["br"]                   [actions.break_]
                )
            >>  ']'
            ;

        escape =
                qi::raw["\\n"]                      [actions.break_]
            |   "\\ "                               // ignore an escaped char
            |   '\\' >> qi::punct                   [actions.raw_char]
            |   (
                    ("'''" >> -eol)                 [actions.escape_pre]
                >>  *(qi::char_ - "'''")            [actions.raw_char]
                >>  qi::lit("'''")                  [actions.escape_post]
                )
            ;

        macro_identifier =
            +(qi::char_ - (qi::space | ']'))
            ;

        cond_phrase =
                '?' >> blank
            >>  qi::raw[macro_identifier]           [actions.cond_phrase_pre]
            >>  qi::raw[-phrase]                    [actions.cond_phrase_post]
            ;

        image =
                '$' >> blank                        [ph::clear(ph::ref(actions.attributes))]
            >> (
                qi::eps(qbk_since(105u)) >> (
                        image_filename              [ph::ref(actions.image_fileref) = qi::_1]
                    >>  hard_space
                    >>  *(
                            '['
                        >>  qi::raw[*(qi::alnum | '_')]
                                                    [ph::ref(actions.attribute_name) = as_string(qi::_1)]
                        >>  space
                        >>  qi::raw[*(qi::char_ - (phrase_end | '['))]
                                                    [actions.attribute]
                        >>  ']'
                        >>  space
                        )
                ) |
                qi::eps(qbk_before(105u)) >> (
                        (*(qi::char_ -
                            phrase_end))            [ph::ref(actions.image_fileref) = as_string(qi::_1)]
                )
            )
            >>  &qi::lit(']')                       [actions.image]
            ;

        image_filename = qi::raw[
            +(
                *qi::space
            >>  +(qi::char_ - (qi::space | phrase_end | '['))
            )];

        url =
                '@'
            >>  qi::raw[*(qi::char_ -
                    (']' | qi::space))]             [actions.url_pre]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [actions.url_post]
            ;

        link =
                "link" >> hard_space
            >>  qi::raw[*(qi::char_ -
                    (']' | qi::space))]             [actions.link_pre]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [actions.link_post]
            ;

        anchor =
                '#'
            >>  blank
            >>  qi::raw[*(qi::char_ - phrase_end)]  [actions.anchor]
            ;

        funcref =
            "funcref" >> hard_space
            >>  qi::raw[*(qi::char_ -
                    (']' | hard_space))]            [actions.funcref_pre]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [actions.funcref_post]
            ;

        classref =
            "classref" >> hard_space
            >>  qi::raw[*(qi::char_ -
                    (']' | hard_space))]            [actions.classref_pre]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [actions.classref_post]
            ;

        memberref =
            "memberref" >> hard_space
            >>  qi::raw[*(qi::char_ -
                    (']' | hard_space))]            [actions.memberref_pre]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [actions.memberref_post]
            ;

        enumref =
            "enumref" >> hard_space
            >>  qi::raw[*(qi::char_ -
                    (']' | hard_space))]            [actions.enumref_pre]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [actions.enumref_post]
            ;

        macroref =
            "macroref" >> hard_space
            >>  qi::raw[*(qi::char_ -
                    (']' | hard_space))]            [actions.macroref_pre]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [actions.macroref_post]
            ;

        headerref =
            "headerref" >> hard_space
            >>  qi::raw[*(qi::char_ -
                    (']' | hard_space))]            [actions.headerref_pre]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [actions.headerref_post]
            ;

        conceptref =
            "conceptref" >> hard_space
            >>  qi::raw[*(qi::char_ -
                    (']' | hard_space))]            [actions.conceptref_pre]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [actions.conceptref_post]
            ;

        globalref =
            "globalref" >> hard_space
            >>  qi::raw[*(qi::char_ -
                    (']' | hard_space))]            [actions.globalref_pre]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [actions.globalref_post]
            ;

        bold =
                qi::char_('*')                      [actions.bold_pre]
            >>  blank >> phrase                     [actions.bold_post]
            ;

        italic =
                qi::char_('\'')                     [actions.italic_pre]
            >>  blank >> phrase                     [actions.italic_post]
            ;

        underline =
                qi::char_('_')                      [actions.underline_pre]
            >>  blank >> phrase                     [actions.underline_post]
            ;

        teletype =
                qi::char_('^')                      [actions.teletype_pre]
            >>  blank >> phrase                     [actions.teletype_post]
            ;

        strikethrough =
                qi::char_('-')                      [actions.strikethrough_pre]
            >>  blank >> phrase                     [actions.strikethrough_post]
            ;

        quote =
                qi::char_('"')                      [actions.quote_pre]
            >>  blank >> phrase                     [actions.quote_post]
            ;

        replaceable =
                qi::char_('~')                      [actions.replaceable_pre]
            >>  blank >> phrase                     [actions.replaceable_post]
            ;

        source_mode =
            (
                qi::string("c++")
            |   qi::string("python")
            |   qi::string("teletype")
            )                                       [ph::ref(actions.source_mode) = qi::_1]
            ;

        footnote =
                qi::lit("footnote")                 [actions.footnote_pre]
            >>  blank >> phrase                     [actions.footnote_post]
            ;
    }

    template <typename Iterator, typename Actions>
    simple_phrase_grammar<Iterator, Actions>::simple_phrase_grammar(Actions& actions)
        : simple_phrase_grammar::base_type(phrase, "simple_phrase")
        , actions(actions), unused(false), common(actions, unused),
        phrase("phrase"), comment("comment"), dummy_block("dummy_block")
    {
        phrase =
           *(   common
            |   comment
            |   (qi::char_ - ']')               [actions.plain_char]
            )
            ;

        comment =
            "[/" >> *(dummy_block | (qi::char_ - ']')) >> ']'
            ;

        dummy_block =
            '[' >> *(dummy_block | (qi::char_ - ']')) >> ']'
            ;
    }
}

#endif // BOOST_SPIRIT_QUICKBOOK_PHRASE_HPP

