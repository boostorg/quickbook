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

    struct phrase_markup_grammar_local
    {
        qi::rule<iterator, quickbook::callout_link()> callout_link;
        qi::rule<iterator, quickbook::cond_phrase()> cond_phrase;
        qi::rule<iterator, quickbook::link()> url;
        qi::symbols<char, formatted_type> link_symbol;
        qi::rule<iterator, quickbook::link(formatted_type)> link;
        qi::rule<iterator, quickbook::anchor()> anchor;
        qi::rule<iterator, quickbook::formatted(formatted_type)> formatted;
    };

    void quickbook_grammar::impl::init_phrase_markup()
    {
        phrase_markup_grammar_local& local = store_.create();

        // Callouts

        // Don't use this, it's meant to be private.        
        phrase_symbol_rules.add("[callout]", local.callout_link [actions.process]);

        local.callout_link =
                (*~qi::char_(' '))          [member_assign(&quickbook::callout_link::role)]
            >>  ' '
            >>  (*~qi::char_(']'))          [member_assign(&quickbook::callout_link::identifier)]
            ;

        // Conditional Phrase

        phrase_symbol_rules.add("?", local.cond_phrase [actions.process]);

        local.cond_phrase =
                blank
            >>  macro_identifier            [member_assign(&quickbook::cond_phrase::macro_id)]
            >>  -phrase                     [member_assign(&quickbook::cond_phrase::content)]
            ;

        // URL

        phrase_symbol_rules.add("@", local.url [actions.process]);

        local.url
            =   (*(qi::char_ - (']' | qi::space)))      [member_assign(&quickbook::link::destination)]
                                                        [member_assign(&quickbook::link::type, "url")]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)              [member_assign(&quickbook::link::content)]
                )
            ;

        // Link
        
        phrase_keyword_rules.add
            ("link", local.link(formatted_type("link")) [actions.process])
            ("funcref", local.link(formatted_type("funcref")) [actions.process])
            ("classref", local.link(formatted_type("classref")) [actions.process])
            ("memberref", local.link(formatted_type("memberref")) [actions.process])
            ("enumref", local.link(formatted_type("enumref")) [actions.process]) 
            ("macroref", local.link(formatted_type("macroref")) [actions.process]) 
            ("headerref", local.link(formatted_type("headerref")) [actions.process]) 
            ("conceptref", local.link(formatted_type("conceptref")) [actions.process])
            ("globalref", local.link(formatted_type("globalref")) [actions.process])
            ;

        local.link =
                qi::attr(qi::_r1)                       [member_assign(&quickbook::link::type)]
            >>  space
            >>  (*(qi::char_ - (']' | qi::space)))      [member_assign(&quickbook::link::destination)]
            >>  (   &qi::lit(']')
                |   (hard_space >> phrase)              [member_assign(&quickbook::link::content)]
                )
            ;

        // Anchor

        phrase_symbol_rules.add("#", local.anchor [actions.process]);

        local.anchor =
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

        phrase_symbol_rules.add
            ("*", local.formatted(formatted_type("bold")) [actions.process])
            ("'", local.formatted(formatted_type("italic")) [actions.process])
            ("_", local.formatted(formatted_type("underline")) [actions.process])
            ("^", local.formatted(formatted_type("teletype")) [actions.process])
            ("-", local.formatted(formatted_type("strikethrough")) [actions.process])
            ("\"", local.formatted(formatted_type("quote")) [actions.process])
            ("~", local.formatted(formatted_type("replaceable")) [actions.process])
            ;

        phrase_keyword_rules.add
            ("footnote", local.formatted(formatted_type("footnote")) [actions.process])
            ;

        local.formatted =
                qi::attr(qi::_r1)               [member_assign(&quickbook::formatted::type)]
            >>  blank
            >>  phrase                          [member_assign(&quickbook::formatted::content)]
            ;
    }
}