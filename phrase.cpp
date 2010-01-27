/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "./phrase.hpp"
#include "./grammars.hpp"
#include "./detail/quickbook.hpp"
#include "./detail/utils.hpp"
#include "./detail/markups.hpp"
#include "./detail/actions_class.hpp"
#include "./parse_utils.hpp"
#include <map>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::link,
    (quickbook::link_type, type)
    (std::string, destination)
    (std::string, content)
)

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;
    
    struct phrase_grammar::rules
    {
        rules(quickbook::actions& actions, bool& no_eols);
    
        quickbook::actions& actions;
        bool& no_eols;

        qi::rule<iterator>
                        space, blank, comment, phrase, phrase_markup,
                        phrase_end, bold, italic, underline, teletype,
                        strikethrough, escape, common,
                        anchor, hard_space, eol, inline_code, simple_format,
                        template_,
                        quote, code_block, footnote, replaceable, macro,
                        dummy_block, cond_phrase, macro_identifier,
                        brackets_1_4, template_inner_arg_1_5, brackets_1_5
                        ;

        qi::rule<iterator, std::string()> template_arg_1_4, template_arg_1_5;
        qi::rule<iterator, std::vector<std::string>() > template_args;

        qi::rule<iterator, std::string()> phrase_attr;

        qi::rule<iterator> image, image_1_4, image_1_5;
        qi::rule<iterator, std::string()> image_filename, image_attribute_key, image_attribute_value;
        qi::rule<iterator, std::multimap<std::string, std::string>()> image_attributes;
        qi::rule<iterator, std::pair<std::string, std::string>()> image_attribute;
        
        qi::rule<iterator, boost::iterator_range<iterator>(char)> simple_markup;
        qi::symbols<char, link_type> link_symbol;
        qi::rule<iterator, quickbook::link()> link, url;
        
        qi::symbols<char, quickbook::source_mode> source_mode;
    };

    phrase_grammar::phrase_grammar(quickbook::actions& actions, bool& no_eols)
        : phrase_grammar::base_type(start, "phrase")
        , rules_pimpl(new rules(actions, no_eols))
    {
        start = rules_pimpl->common;
    }

    phrase_grammar::~phrase_grammar() {}

    phrase_grammar::rules::rules(quickbook::actions& actions, bool& no_eols)
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

        phrase_attr =
                qi::eps                             [actions.phrase_push]
            >>  (   phrase
                |   qi::eps
                )                                   [actions.phrase_pop]
            ;

        phrase_markup =
                '['
            >>  (   cond_phrase
                |   image
                |   url                             [actions.process]
                |   link                            [actions.process]
                |   anchor
                |   source_mode                     [actions.process]
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

        anchor =
                '#'
            >>  blank
            >>  qi::raw[*(qi::char_ - phrase_end)]  [actions.anchor]
            ;

        link_symbol.add
            ("link", link_type(link_pre_, link_post_))
            ("funcref", link_type(funcref_pre_, funcref_post_))
            ("classref", link_type(classref_pre_, classref_post_))
            ("memberref", link_type(memberref_pre_, memberref_post_))
            ("enumref", link_type(enumref_pre_, enumref_post_)) 
            ("macroref", link_type(macroref_pre_, macroref_post_)) 
            ("headerref", link_type(headerref_pre_, headerref_post_)) 
            ("conceptref", link_type(conceptref_pre_, conceptref_post_)) 
            ("globalref", link_type(globalref_pre_, globalref_post_))
            ;

        link =
                link_symbol
            >>  hard_space
            >>  *(qi::char_ - (']' | qi::space))
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase_attr)
                )
            ;

        url =
                '@'
            >>  qi::attr(link_type(url_pre_, url_post_))
            >>  *(qi::char_ - (']' | qi::space))
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase_attr)
                )
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

        source_mode.add
            ("c++", quickbook::source_mode("c++"))
            ("python", quickbook::source_mode("python"))
            ("teletype", quickbook::source_mode("teletype"))
            ;

        footnote =
                qi::lit("footnote")                 [actions.footnote_pre]
            >>  blank >> phrase                     [actions.footnote_post]
            ;
    }

    struct simple_phrase_grammar::rules
    {
        rules(quickbook::actions& actions);

        quickbook::actions& actions;
        bool unused;
        phrase_grammar common;
        qi::rule<iterator> phrase, comment, dummy_block;
    };

    simple_phrase_grammar::simple_phrase_grammar(quickbook::actions& actions)
        : simple_phrase_grammar::base_type(start, "simple_phrase")
        , rules_pimpl(new rules(actions))
        , start(rules_pimpl->phrase) {}

    simple_phrase_grammar::~simple_phrase_grammar() {}

    simple_phrase_grammar::rules::rules(quickbook::actions& actions)
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
