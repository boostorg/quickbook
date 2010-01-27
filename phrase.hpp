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
#include "./detail/markups.hpp"
#include "./parse_utils.hpp"
#include <map>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/fusion/include/std_pair.hpp>

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;
    
    template <typename Iterator, typename Actions>
    struct phrase_grammar<Iterator, Actions>::rules
    {
        rules(Actions& actions, bool& no_eols);
    
        Actions& actions;
        bool& no_eols;

        qi::rule<Iterator>
                        space, blank, comment, phrase, phrase_markup,
                        phrase_end, bold, italic, underline, teletype,
                        strikethrough, escape, url, common, funcref, classref,
                        memberref, enumref, macroref, headerref, conceptref, globalref,
                        anchor, link, hard_space, eol, inline_code, simple_format,
                        source_mode, template_,
                        quote, code_block, footnote, replaceable, macro,
                        dummy_block, cond_phrase, macro_identifier,
                        brackets_1_4, template_inner_arg_1_5, brackets_1_5
                        ;

        qi::rule<Iterator, std::string()> template_arg_1_4, template_arg_1_5;
        qi::rule<Iterator, std::vector<std::string>() > template_args;

        qi::rule<Iterator> image, image_1_4, image_1_5;
        qi::rule<Iterator, std::string()> image_filename, image_attribute_key, image_attribute_value;
        qi::rule<Iterator, std::multimap<std::string, std::string>()> image_attributes;
        qi::rule<Iterator, std::pair<std::string, std::string>()> image_attribute;
        
        qi::rule<Iterator, boost::iterator_range<Iterator>(char)> simple_markup;
        qi::rule<Iterator, void(char const*, char const*, char const*)> generic_link;
    };

    template <typename Iterator, typename Actions>
    phrase_grammar<Iterator, Actions>::phrase_grammar(Actions& actions, bool& no_eols)
        : phrase_grammar::base_type(start, "phrase")
        , rules_pimpl(new rules(actions, no_eols))
    {
        start = rules_pimpl->common;
    }

    template <typename Iterator, typename Actions>
    phrase_grammar<Iterator, Actions>::~phrase_grammar() {}

    template <typename Iterator, typename Actions>
    phrase_grammar<Iterator, Actions>::rules::rules(Actions& actions, bool& no_eols)
        : actions(actions), no_eols(no_eols)
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

        // Template call

        template_ =
            (   qi::raw[qi::eps]                    // For the position of the template
            >>  -qi::char_('`')                     // Attribute implicitly cast to bool
            >>  (                                   // Lookup the template name
                    (&qi::punct >> actions.templates.scope)
                |   (actions.templates.scope >> hard_space)
                )
            >>  template_args
            >>  &qi::lit(']')
            ) [ph::bind(actions.do_template, ph::begin(qi::_1), qi::_2, qi::_3, qi::_4)]
            ;

        template_args =
            qi::eps(qbk_before(105u)) >> -(template_arg_1_4 % "..") |
            qi::eps(qbk_since(105u)) >> -(template_arg_1_5 % "..");

        template_arg_1_4 =
            qi::raw[+(brackets_1_4 | (qi::char_ - (qi::lit("..") | ']')))]
            ;

        brackets_1_4 =
            '[' >> +template_arg_1_4 >> ']'
            ;

        template_arg_1_5 =
            qi::raw[+(brackets_1_5 | ('\\' >> qi::char_) | (qi::char_ - (qi::lit("..") | '[' | ']')))]
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

        simple_markup =
            qi::omit[qi::char_(qi::_r1)] >> 
            qi::raw[
                (
                    qi::graph                   // A single char. e.g. *c*
                    >> &(qi::char_(qi::_r1)
                        >> (qi::space | qi::punct | qi::eoi))
                                                // space, punct or end
                )                               // must follow qi::char_(qi::_r1)
            |
                (   qi::graph >>                // qi::graph must follow qi::char_(qi::_r1)
                    *(qi::char_ -
                        (   (qi::graph >> qi::char_(qi::_r1)) // Make sure that we don't go
                        |   phrase_end          // past a single block
                        )
                    ) >> qi::graph              // qi::graph must precede qi::char_(qi::_r1)
                    >> &(qi::char_(qi::_r1)
                        >> (qi::space | qi::punct | qi::eoi))
                                                // space, punct or end
                )                               // must follow qi::char_(qi::_r1)
            ]
            >> qi::omit[qi::char_(qi::_r1)]
            ;


        simple_format =
                simple_markup('*')              [actions.simple_bold]
            |   simple_markup('/')              [actions.simple_italic]
            |   simple_markup('_')              [actions.simple_underline]
            |   simple_markup('=')              [actions.simple_teletype]
            ;

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
                |   template_
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
            (qi::eps(qbk_since(105u)) >> image_1_5) |
            (qi::eps(qbk_before(105u)) >> image_1_4);
        
        image_1_4 = (
                qi::raw['$']
            >>  blank
            >>  *(qi::char_ - phrase_end)
            >>  &qi::lit(']')
            ) [ph::bind(actions.image, ph::begin(qi::_1), as_string(qi::_2))]
            ;
        
        image_1_5 = (
                qi::raw['$']
            >>  blank
            >>  image_filename
            >>  hard_space
            >>  image_attributes
            >>  &qi::lit(']')
            ) [ph::bind(actions.image, ph::begin(qi::_1), qi::_2, qi::_3)]
            ;

        image_filename = qi::raw[
                +(qi::char_ - (qi::space | phrase_end | '['))
            >>  *(
                    +qi::space
                >>  +(qi::char_ - (qi::space | phrase_end | '['))
             )];

        image_attributes = *(image_attribute >> space);
        
        image_attribute =
                '['
            >>  image_attribute_key
            >>  space
            >>  image_attribute_value
            >>  ']'
            ;
            
        image_attribute_key = *(qi::alnum | '_');
        image_attribute_value = *(qi::char_ - (phrase_end | '['));

        url =
                '@'
            >>  qi::raw[*(qi::char_ -
                    (']' | qi::space))]             [ph::bind(actions.generic_link_pre, actions.url_pre, qi::_1)]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [ph::bind(actions.generic_link_post, actions.url_post)]
            ;

        anchor =
                '#'
            >>  blank
            >>  qi::raw[*(qi::char_ - phrase_end)]  [actions.anchor]
            ;

        generic_link =
                qi::string(qi::_r1)
            >>  hard_space
            >>  qi::raw[*(qi::char_ -
                    (']' | qi::space))]             [ph::bind(actions.generic_link_pre, qi::_r2, qi::_1)]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )                                   [ph::bind(actions.generic_link_post, qi::_r3)]
            ;

        link    = generic_link((char const*)"link", link_pre_, link_post_);
        funcref = generic_link((char const*)"funcref", funcref_pre_, funcref_post_);
        classref = generic_link((char const*)"classref", classref_pre_, classref_post_);
        memberref = generic_link((char const*)"memberref", memberref_pre_, memberref_post_);
        enumref = generic_link((char const*)"enumref", enumref_pre_, enumref_post_); 
        macroref = generic_link((char const*)"macroref", macroref_pre_, macroref_post_); 
        headerref = generic_link((char const*)"headerref", headerref_pre_, headerref_post_); 
        conceptref = generic_link((char const*)"conceptref", conceptref_pre_, conceptref_post_); 
        globalref = generic_link((char const*)"globalref", globalref_pre_, globalref_post_); 

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
    struct simple_phrase_grammar<Iterator, Actions>::rules
    {
        rules(Actions& actions);

        Actions& actions;
        bool unused;
        phrase_grammar<Iterator, Actions> common;
        qi::rule<Iterator> phrase, comment, dummy_block;
    };

    template <typename Iterator, typename Actions>
    simple_phrase_grammar<Iterator, Actions>::simple_phrase_grammar(Actions& actions)
        : simple_phrase_grammar::base_type(start, "simple_phrase")
        , rules_pimpl(new rules(actions))
        , start(rules_pimpl->phrase) {}

    template <typename Iterator, typename Actions>
    simple_phrase_grammar<Iterator, Actions>::~simple_phrase_grammar() {}

    template <typename Iterator, typename Actions>
    simple_phrase_grammar<Iterator, Actions>::rules::rules(Actions& actions)
        : actions(actions), unused(false), common(actions, unused)
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

