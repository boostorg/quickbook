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
#include "grammar_impl.hpp"
#include "phrase.hpp"
#include "actions.hpp"
#include "template.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;

    void quickbook_grammar::impl::init_phrase_markup()
    {
        qi::rule<iterator>& phrase_end = store_.create();

        // Callouts

        // Don't use this, it's meant to be private.
        qi::rule<iterator, quickbook::callout_link()>& callout_link = store_.create();
        
        phrase_symbol_rules.add("[callout]", callout_link [actions.process]);

        callout_link =
                (*~qi::char_(' '))          [member_assign(&quickbook::callout_link::role)]
            >>  ' '
            >>  (*~qi::char_(']'))          [member_assign(&quickbook::callout_link::identifier)]
            ;

        // Conditional Phrase

        qi::rule<iterator, quickbook::cond_phrase()>& cond_phrase = store_.create();
        
        phrase_symbol_rules.add("?", cond_phrase [actions.process]);

        cond_phrase =
                blank
            >>  macro_identifier            [member_assign(&quickbook::cond_phrase::macro_id)]
            >>  -phrase                     [member_assign(&quickbook::cond_phrase::content)]
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
                position                            [member_assign(&quickbook::image::position)]
            >>  blank
            >>  (*(qi::char_ - phrase_end))         [member_assign(&quickbook::image::image_filename)]
            >>  &qi::lit(']')
            ;
        
        image_1_5 =
                position                            [member_assign(&quickbook::image::position)]
            >>  blank
            >>  image_filename                      [member_assign(&quickbook::image::image_filename)]
            >>  hard_space
            >>  image_attributes                    [member_assign(&quickbook::image::attributes)]
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
            >>  image_attribute_key                 [member_assign(&std::pair<std::string, std::string>::first)]
            >>  space
            >>  image_attribute_value               [member_assign(&std::pair<std::string, std::string>::second)]
            >>  ']'
            ;
            
        image_attribute_key = *(qi::alnum | '_');
        image_attribute_value = *(qi::char_ - (phrase_end | '['));

        // URL

        qi::rule<iterator, quickbook::link()>& url = store_.create();
        
        phrase_symbol_rules.add("@", url [actions.process]);

        url =   (*(qi::char_ - (']' | qi::space)))      [member_assign(&quickbook::link::destination)]
                                                        [member_assign(&quickbook::link::type, "url")]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)              [member_assign(&quickbook::link::content)]
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
                qi::attr(qi::_r1)                       [member_assign(&quickbook::link::type)]
            >>  space
            >>  (*(qi::char_ - (']' | qi::space)))      [member_assign(&quickbook::link::destination)]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)              [member_assign(&quickbook::link::content)]
                )
            ;

        // Anchor

        qi::rule<iterator, quickbook::anchor()>& anchor = store_.create();
        
        phrase_symbol_rules.add("#", anchor [actions.process]);

        anchor =
                blank
            >>  (*(qi::char_ - phrase_end))         [member_assign(&quickbook::anchor::id)]
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

        formatted =
                qi::attr(qi::_r1)               [member_assign(&quickbook::formatted::type)]
            >>  blank
            >>  phrase                          [member_assign(&quickbook::formatted::content)]
            ;

        // phrase_end

        phrase_end =
            ']' |
            qi::eps(ph::ref(no_eols)) >>
                eol >> eol                      // Make sure that we don't go
            ;                                   // past a single block, except
                                                // when preformatted.
    }
}