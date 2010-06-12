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
#include <boost/spirit/include/phoenix_operator.hpp>
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

    void quickbook_grammar::impl::init_phrase_markup()
    {
        qi::rule<iterator, quickbook::call_template()>& call_template = store_.create();
        qi::rule<iterator, quickbook::break_()>& break_ = store_.create();
        qi::rule<iterator>& phrase_end = store_.create();

        qi::rule<iterator, qi::locals<qi::rule<iterator> > >& phrase_markup_impl = store_.create();
        qi::symbols<char, qi::rule<iterator> >& phrase_keyword_rules = store_.create();
        qi::symbols<char, qi::rule<iterator> >& phrase_symbol_rules = store_.create();

        phrase_markup =
            (   '['
            >>  (   phrase_markup_impl
                |   call_template   [actions.process]
                |   break_          [actions.process]
                )
            >>  ']'
            )                                       
            ;

        phrase_markup_impl
            =   (   phrase_keyword_rules >> !(qi::alnum | '_')
                |   phrase_symbol_rules
                ) [qi::_a = qi::_1]
                >> lazy(qi::_a)
                ;

        // Callouts

        // Don't use this, it's meant to be private.
        qi::rule<iterator, quickbook::callout_link()>& callout_link = store_.create();
        
        phrase_symbol_rules.add("[callout]", callout_link [actions.process]);

        callout_link =
                *~qi::char_(' ')
            >>  ' '
            >>  *~qi::char_(']')
            >>  qi::attr(nothing())
            ;

        // Conditional Phrase

        qi::rule<iterator, quickbook::cond_phrase()>& cond_phrase = store_.create();
        
        phrase_symbol_rules.add("?", cond_phrase [actions.process]);

        cond_phrase =
                blank
            >>  macro_identifier
            >>  -phrase
            ;

        // Images

        qi::rule<iterator, quickbook::image()>& image = store_.create();
        qi::rule<iterator, quickbook::image()>& image_1_4 = store_.create();
        qi::rule<iterator, quickbook::image()>& image_1_5 = store_.create();
        qi::rule<iterator, std::string()>& image_filename = store_.create();
        qi::rule<iterator, quickbook::image::attribute_map()>& image_attributes = store_.create();
        qi::rule<iterator, std::pair<std::string, std::string>()>& image_attribute = store_.create();
        qi::rule<iterator, std::string()>& image_attribute_key = store_.create();
        qi::rule<iterator, std::string()>& image_attribute_value = store_.create();
        
        phrase_symbol_rules.add("$", image [actions.process]);

        image =
            (qi::eps(qbk_since(105u)) >> image_1_5) |
            (qi::eps(qbk_before(105u)) >> image_1_4);
        
        image_1_4 =
                position
            >>  blank
            >>  *(qi::char_ - phrase_end)
            >>  &qi::lit(']')
            ;
        
        image_1_5 =
                position
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

        // URL

        qi::rule<iterator, quickbook::link()>& url = store_.create();
        
        phrase_symbol_rules.add("@", url [actions.process]);

        url =   qi::attr("url")
            >>  *(qi::char_ - (']' | qi::space))
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )
            ;

        qi::symbols<char, formatted_type>& link_symbol = store_.create();

        // Link

        qi::rule<iterator, quickbook::link(formatted_type)>& link = store_.create();
        
        phrase_keyword_rules.add
            ("link", link(formatted_type("link")) [actions.process])
            ("funcref", link(formatted_type("funcref")) [actions.process])
            ("classref", link(formatted_type("classref")) [actions.process])
            ("memberref", link(formatted_type("memberref")) [actions.process])
            ("enumref", link(formatted_type("enumref")) [actions.process]) 
            ("macroref", link(formatted_type("macroref")) [actions.process]) 
            ("headerref", link(formatted_type("headerref")) [actions.process]) 
            ("conceptref", link(formatted_type("conceptref")) [actions.process])
            ("globalref", link(formatted_type("globalref")) [actions.process])
            ;

        link =
                qi::attr(qi::_r1)
            >>  space
            >>  *(qi::char_ - (']' | qi::space))
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)
                )
            ;

        // Anchor

        qi::rule<iterator, quickbook::anchor()>& anchor = store_.create();
        
        phrase_symbol_rules.add("#", anchor [actions.process]);

        anchor =
                blank
            >>  *(qi::char_ - phrase_end)
            >>  qi::attr(nothing())
            ;

        // Source Mode

        phrase_keyword_rules.add
            ("c++", qi::attr(quickbook::source_mode("c++")) [actions.process])
            ("python", qi::attr(quickbook::source_mode("python"))  [actions.process])
            ("teletype", qi::attr(quickbook::source_mode("teletype")) [actions.process])
            ;

        // Formatted

        qi::rule<iterator, quickbook::formatted(formatted_type)>& formatted = store_.create();

        phrase_symbol_rules.add
            ("*", formatted(formatted_type("bold")) [actions.process])
            ("'", formatted(formatted_type("italic")) [actions.process])
            ("_", formatted(formatted_type("underline")) [actions.process])
            ("^", formatted(formatted_type("teletype")) [actions.process])
            ("-", formatted(formatted_type("strikethrough")) [actions.process])
            ("\"", formatted(formatted_type("quote")) [actions.process])
            ("~", formatted(formatted_type("replaceable")) [actions.process])
            ;

        phrase_keyword_rules.add
            ("footnote", formatted(formatted_type("footnote")) [actions.process])
            ;

        formatted = qi::attr(qi::_r1) >> blank >> phrase;

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