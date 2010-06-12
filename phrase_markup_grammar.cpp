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
    }
}