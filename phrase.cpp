/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <map>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include "code.hpp"
#include "phrase.hpp"
#include "grammars.hpp"
#include "actions_class.hpp"
#include "markups.hpp"
#include "quickbook.hpp"
#include "parse_utils.hpp"

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::anchor,
    (std::string, id)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::link,
    (quickbook::markup, type)
    (std::string, destination)
    (std::string, content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::simple_markup,
    (char, symbol)
    (std::string, raw_content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::break_,
    (quickbook::file_position, position)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::image,
    (quickbook::file_position, position)
    (std::string, image_filename)
    (quickbook::image::attribute_map, attributes)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::cond_phrase,
    (std::string, macro_id)
    (std::string, content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::call_template,
    (quickbook::file_position, position)
    (bool, escape)
    (quickbook::template_symbol const*, symbol)
    (std::vector<std::string>, args)
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

        qi::rule<iterator, std::string()> phrase;
        qi::rule<iterator> common;
        qi::rule<iterator> macro;
        qi::rule<iterator> phrase_markup;
        qi::rule<iterator, quickbook::code()> code_block;
        qi::rule<iterator, quickbook::code()> inline_code;
        qi::rule<iterator, quickbook::simple_markup(), qi::locals<char> > simple_format;
        qi::rule<iterator> escape;        
        qi::rule<iterator, quickbook::break_()> escape_break;
        qi::rule<iterator, quickbook::formatted()> escape_punct;
        qi::rule<iterator, quickbook::formatted()> escape_markup;
        qi::rule<iterator> comment;
        qi::rule<iterator> dummy_block;
        qi::rule<iterator, quickbook::cond_phrase()> cond_phrase;
        qi::rule<iterator, std::string()> macro_identifier;
        qi::rule<iterator, quickbook::image()> image, image_1_4, image_1_5;
        qi::rule<iterator, std::string()> image_filename;
        qi::rule<iterator, quickbook::image::attribute_map()> image_attributes;
        qi::rule<iterator, std::pair<std::string, std::string>()> image_attribute;
        qi::rule<iterator, std::string()> image_attribute_key, image_attribute_value;
        qi::rule<iterator, quickbook::link()> url;
        qi::rule<iterator, quickbook::link()> link;
        qi::symbols<char, markup> link_symbol;
        qi::rule<iterator, quickbook::anchor()> anchor;
        qi::symbols<char, quickbook::source_mode> source_mode;
        qi::rule<iterator, quickbook::formatted()> formatted;
        qi::symbols<char, markup> format_symbol;
        qi::rule<iterator, quickbook::formatted()> footnote;
        qi::rule<iterator, quickbook::call_template()> call_template;
        qi::rule<iterator, std::vector<std::string>() > template_args;
        qi::rule<iterator, std::string()> template_arg_1_4;
        qi::rule<iterator> brackets_1_4;
        qi::rule<iterator, std::string()> template_arg_1_5;
        qi::rule<iterator> template_inner_arg_1_5;
        qi::rule<iterator> brackets_1_5;
        qi::rule<iterator, quickbook::break_()> break_;
        qi::rule<iterator> space, blank, eol, phrase_end, hard_space;
        qi::rule<iterator, file_position()> position;
        
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
        phrase =
                qi::eps                         [actions.phrase_push]        
            >> *(   common
                |   comment
                |   (qi::char_ - phrase_end)    [actions.plain_char]
                )
            >>  qi::eps                         [actions.phrase_pop]
            ;

        common =
                macro
            |   phrase_markup
            |   code_block                          [actions.process]
            |   inline_code                         [actions.process]
            |   simple_format                       [actions.process]
            |   escape
            |   comment
            ;

         macro =
            (   actions.macro                       // must not be followed by
            >>  !(qi::alpha | '_')                  // alpha or underscore
            )                                       [actions.process]
            ;

        phrase_markup =
            (   '['
            >>  (   cond_phrase                     
                |   image
                |   url
                |   link
                |   anchor
                |   source_mode
                |   formatted
                |   footnote
                |   call_template
                |   break_
                )
            >>  ']'
            )                                       [actions.process]
            ;

        code_block =
                (
                    "```"
                >>  position
                >>  qi::raw[*(qi::char_ - "```")]
                >>  "```"
                >>  qi::attr(true)
                )
            |   (
                    "``"
                >>  position
                >>  qi::raw[*(qi::char_ - "``")]
                >>  "``"
                >>  qi::attr(true)
                )
            ;

        inline_code =
                '`'
            >>  position
            >>  qi::raw
                [   *(  qi::char_ -
                        (   '`'
                        |   (eol >> eol)            // Make sure that we don't go
                        )                           // past a single block
                    )
                    >>  &qi::lit('`')
                ]
            >>  '`'
            >>  qi::attr(false)
            ;

        simple_format %=
                qi::char_("*/_=")               [qi::_a = qi::_1]
            >>  qi::raw
                [   (   (   qi::graph               // A single char. e.g. *c*
                        >>  &(  qi::char_(qi::_a)
                            >>  (qi::space | qi::punct | qi::eoi)
                            )
                        )
                    |
                        (   qi::graph               // qi::graph must follow qi::lit(qi::_r1)
                        >>  *(  qi::char_ -
                                (   (qi::graph >> qi::lit(qi::_a))
                                |   phrase_end      // Make sure that we don't go
                                )                   // past a single block
                            )
                        >>  qi::graph               // qi::graph must precede qi::lit(qi::_r1)
                        >>  &(  qi::char_(qi::_a)
                            >>  (qi::space | qi::punct | qi::eoi)
                            )
                        )
                    )
                ]
            >> qi::omit[qi::char_(qi::_a)]
            ;

        escape =
            (   escape_break
            |   "\\ "                               // ignore an escaped char
            |   escape_punct
            |   escape_markup                       
            )                                       [actions.process]
            ;
        
        escape_break =
                position
            >>  "\\n"
            >>  qi::attr(nothing())
            ;

        escape_punct =
                qi::attr(markup())
            >>  '\\'
            >>  qi::repeat(1)[qi::punct]
            ;

        escape_markup =
                ("'''" >> -eol)
            >>  qi::attr(markup(escape_pre_, escape_post_))
            >>  *(qi::char_ - "'''")
            >>  "'''"
            ;

        comment =
            "[/" >> *(dummy_block | (qi::char_ - ']')) >> ']'
            ;

        dummy_block =
            '[' >> *(dummy_block | (qi::char_ - ']')) >> ']'
            ;

        cond_phrase =
                '?'
            >>  blank
            >>  macro_identifier
            >>  -phrase
            ;

        macro_identifier =
            +(qi::char_ - (qi::space | ']'))
            ;

        image =
            (qi::eps(qbk_since(105u)) >> image_1_5) |
            (qi::eps(qbk_before(105u)) >> image_1_4);
        
        image_1_4 =
                position
            >>  '$'
            >>  blank
            >>  *(qi::char_ - phrase_end)
            >>  &qi::lit(']')
            ;
        
        image_1_5 =
                position
            >>  '$'
            >>  blank
            >>  image_filename
            >>  hard_space
            >>  image_attributes
            >>  &qi::lit(']')
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
            >>  qi::attr(markup(url_pre_, url_post_))
            >>  *(qi::char_ - (']' | qi::space))
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )
            ;

        link =
                link_symbol
            >>  hard_space
            >>  *(qi::char_ - (']' | qi::space))
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )
            ;

        link_symbol.add
            ("link", markup(link_pre_, link_post_))
            ("funcref", markup(funcref_pre_, funcref_post_))
            ("classref", markup(classref_pre_, classref_post_))
            ("memberref", markup(memberref_pre_, memberref_post_))
            ("enumref", markup(enumref_pre_, enumref_post_)) 
            ("macroref", markup(macroref_pre_, macroref_post_)) 
            ("headerref", markup(headerref_pre_, headerref_post_)) 
            ("conceptref", markup(conceptref_pre_, conceptref_post_)) 
            ("globalref", markup(globalref_pre_, globalref_post_))
            ;

        anchor =
                '#'
            >>  blank
            >>  *(qi::char_ - phrase_end)
            >>  qi::attr(nothing())
            ;

        source_mode.add
            ("c++", quickbook::source_mode("c++"))
            ("python", quickbook::source_mode("python"))
            ("teletype", quickbook::source_mode("teletype"))
            ;

        formatted = format_symbol >> blank >> phrase;

        format_symbol.add
            ("*", markup(bold_pre_, bold_post_))
            ("'", markup(italic_pre_, italic_post_))
            ("_", markup(underline_pre_, underline_post_))
            ("^", markup(teletype_pre_, teletype_post_))
            ("-", markup(strikethrough_pre_, strikethrough_post_))
            ("\"", markup(quote_pre_, quote_post_))
            ("~", markup(replaceable_pre_, replaceable_post_))
            ;

        footnote =
                "footnote"
            >>  qi::attr(markup(footnote_pre_, footnote_post_))
            >>  blank
            >>  phrase
            ;

        // Template call

        call_template =
                position
            >>  (   '`' >> qi::attr(true)
                |   qi::attr(false)
                )
            >>  (                                   // Lookup the template name
                    (&qi::punct >> actions.templates.scope)
                |   (actions.templates.scope >> hard_space)
                )
            >>  template_args
            >>  &qi::lit(']')
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

        break_ =
                position
            >>  "br"
            >>  qi::attr(nothing())
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
            ;                                   // past a single block, except
                                                // when preformatted.

        hard_space =
            !(qi::alnum | '_') >> space
            ;                                   // must not be preceded by
                                                // alpha-numeric or underscore
         position = qi::raw[qi::eps] [get_position];
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
