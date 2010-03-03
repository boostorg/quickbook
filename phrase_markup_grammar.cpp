/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include "phrase_grammar.hpp"
#include "actions.hpp"
#include "template.hpp"
#include "misc_rules.hpp"

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::anchor,
    (std::string, id)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::link,
    (quickbook::formatted_type, type)
    (std::string, destination)
    (std::string, content)
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
    (std::vector<quickbook::template_value>, args)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::template_value,
    (quickbook::file_position, position)
    (std::string, content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::callout_link,
    (std::string, role)
    (std::string, identifier)
)

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;

    void phrase_grammar::rules::init_phrase_markup()
    {
        qi::rule<iterator, quickbook::callout_link()>& callout_link = store_.create();
        qi::rule<iterator, quickbook::cond_phrase()>& cond_phrase = store_.create();
        qi::rule<iterator, quickbook::image()>& image = store_.create();
        qi::rule<iterator, quickbook::link()>& url = store_.create();
        qi::rule<iterator, quickbook::link()>& link = store_.create();
        qi::rule<iterator, quickbook::anchor()>& anchor = store_.create();
        qi::symbols<char, quickbook::source_mode>& source_mode = store_.create();
        qi::rule<iterator, quickbook::formatted()>& formatted = store_.create();
        qi::rule<iterator, quickbook::formatted()>& footnote = store_.create();
        qi::rule<iterator, quickbook::call_template()>& call_template = store_.create();
        qi::rule<iterator, quickbook::break_()>& break_ = store_.create();
        qi::rule<iterator>& phrase_end = store_.create();

        phrase_markup =
            (   '['
            >>  (   callout_link
                |   cond_phrase
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

        // Don't use this, it's meant to be private.
        callout_link =
                "[callout]"
            >>  *~qi::char_(' ')
            >>  ' '
            >>  *~qi::char_(']')
            >>  qi::attr(nothing())
            ;

        cond_phrase =
                '?'
            >>  blank
            >>  macro_identifier
            >>  -phrase
            ;

        qi::rule<iterator, quickbook::image()>& image_1_4 = store_.create();
        qi::rule<iterator, quickbook::image()>& image_1_5 = store_.create();
        qi::rule<iterator, std::string()>& image_filename = store_.create();
        qi::rule<iterator, quickbook::image::attribute_map()>& image_attributes = store_.create();
        qi::rule<iterator, std::pair<std::string, std::string>()>& image_attribute = store_.create();
        qi::rule<iterator, std::string()>& image_attribute_key = store_.create();
        qi::rule<iterator, std::string()>& image_attribute_value = store_.create();

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
            >>  qi::attr("url")
            >>  *(qi::char_ - (']' | qi::space))
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )
            ;

        qi::symbols<char, formatted_type>& link_symbol = store_.create();

        link =
                link_symbol
            >>  hard_space
            >>  *(qi::char_ - (']' | qi::space))
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )
            ;

        link_symbol.add
            ("link", formatted_type("link"))
            ("funcref", formatted_type("funcref"))
            ("classref", formatted_type("classref"))
            ("memberref", formatted_type("memberref"))
            ("enumref", formatted_type("enumref")) 
            ("macroref", formatted_type("macroref")) 
            ("headerref", formatted_type("headerref")) 
            ("conceptref", formatted_type("conceptref"))
            ("globalref", formatted_type("globalref"))
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

        qi::symbols<char, formatted_type>& format_symbol = store_.create();

        formatted = format_symbol >> blank >> phrase;

        format_symbol.add
            ("*", "bold")
            ("'", "italic")
            ("_", "underline")
            ("^", "teletype")
            ("-", "strikethrough")
            ("\"", "quote")
            ("~", "replaceable")
            ;

        footnote =
                "footnote"
            >>  qi::attr("footnote")
            >>  blank
            >>  phrase
            ;

        // Template call

        qi::rule<iterator, std::vector<quickbook::template_value>()>& template_args = store_.create();
        qi::rule<iterator, quickbook::template_value()>& template_arg_1_4 = store_.create();
        qi::rule<iterator>& brackets_1_4 = store_.create();
        qi::rule<iterator, quickbook::template_value()>& template_arg_1_5 = store_.create();
        qi::rule<iterator>& brackets_1_5 = store_.create();

        call_template =
                position
            >>  qi::matches['`']
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
            position >>
            qi::raw[+(brackets_1_4 | ~qi::char_(']') - "..")]
            ;

        brackets_1_4 =
            '[' >> +(brackets_1_4 | ~qi::char_(']') - "..") >> ']'
            ;

        template_arg_1_5 =
            position >>
            qi::raw[+(brackets_1_5 | '\\' >> qi::char_ | ~qi::char_("[]") - "..")]
            ;

        brackets_1_5 =
            '[' >> +(brackets_1_5 | '\\' >> qi::char_ | ~qi::char_("[]")) >> ']'
            ;

        break_ =
                position
            >>  "br"
            >>  qi::attr(nothing())
            ;

        phrase_end =
            ']' |
            qi::eps(ph::ref(no_eols)) >>
                eol >> eol                      // Make sure that we don't go
            ;                                   // past a single block, except
                                                // when preformatted.
    }
}