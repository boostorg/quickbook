/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include "grammar_impl.hpp"
#include "phrase.hpp"
#include "code.hpp"
#include "actions.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;

    struct phrase_grammar_local
    {
        qi::rule<iterator> macro;
        qi::rule<iterator> phrase_markup;
        qi::rule<iterator, qi::locals<qi::rule<iterator> > > phrase_markup_impl;
        qi::rule<iterator, quickbook::break_()> break_;
        qi::rule<iterator, quickbook::simple_markup(), qi::locals<char> > simple_format;
        qi::rule<iterator, std::string(char)> simple_format_body;
        qi::rule<iterator, void(char)> simple_format_chars;
        qi::rule<iterator, void(char)> simple_format_end;
        qi::rule<iterator> simple_phrase_end;
        qi::rule<iterator> escape;
        qi::rule<iterator, quickbook::break_()> escape_break;
        qi::rule<iterator, quickbook::formatted()> escape_punct;
        qi::rule<iterator, quickbook::formatted()> escape_markup;
        qi::rule<iterator, quickbook::unicode_char()> escape_unicode16;
        qi::rule<iterator, quickbook::unicode_char()> escape_unicode32;
    };

    void quickbook_grammar::impl::init_phrase()
    {
        phrase_grammar_local& local = store_.create();

        simple_phrase =
           *(   common
            |   (qi::char_ - ']')               [actions.process]
            )
            ;

        phrase =
                qi::eps                         [actions.phrase_push]        
            >> *(   common
                |   (qi::char_ - phrase_end)    [actions.process]
                )
            >>  qi::eps                         [actions.phrase_pop]
            ;

        common =
                local.macro
            |   local.phrase_markup
            |   code_block                          [actions.process]
            |   inline_code                         [actions.process]
            |   local.simple_format                 [actions.process]
            |   local.escape
            |   comment
            ;

         local.macro =
            (   actions.macro                       // must not be followed by
            >>  !(qi::alpha | '_')                  // alpha or underscore
            )                                       [actions.process]
            ;

        local.phrase_markup =
            (   '['
            >>  (   local.phrase_markup_impl
                |   call_template
                |   local.break_                    [actions.process]
                )
            >>  ']'
            )                                       
            ;

        local.phrase_markup_impl
            =   (   phrase_keyword_rules >> !(qi::alnum | '_')
                |   phrase_symbol_rules
                ) [qi::_a = qi::_1]
                >> lazy(qi::_a)
                ;

        local.break_ =
                position                            [member_assign(&quickbook::break_::position)]
            >>  "br"
            ;

        local.simple_format
            =   qi::char_("*/_=")               [qi::_a = qi::_1]
            >>  local.simple_format_body(qi::_a)[member_assign(&quickbook::simple_markup::raw_content)]
            >>  qi::char_(qi::_a)               [member_assign(&quickbook::simple_markup::symbol)]
            ;

        local.simple_format_body
            =   qi::raw
                [   qi::graph
                >>  (   &local.simple_format_end(qi::_r1)
                    |   local.simple_format_chars(qi::_r1)
                    )
                ]
            ;

        local.simple_format_chars
            =   *(  qi::char_ -
                    (   (qi::graph >> qi::lit(qi::_r1))
                    |   local.simple_phrase_end // Make sure that we don't go
                    )                           // past a single block
                )
            >>  qi::graph                       // qi::graph must precede qi::lit(qi::_r1)
            >>  &local.simple_format_end(qi::_r1)
            ;

        local.simple_format_end
            =   qi::lit(qi::_r1)
            >>  (qi::space | qi::punct | qi::eoi)
            ;

        local.simple_phrase_end = '[' | phrase_end;

        local.escape =
            (   local.escape_break
            |   "\\ "                               // ignore an escaped char            
            |   local.escape_punct
            |   local.escape_unicode16
            |   local.escape_unicode32
            |   local.escape_markup                       
            )                                       [actions.process]
            ;
        
        local.escape_break =
                position                            [member_assign(&quickbook::break_::position)]
            >>  "\\n"
            ;

        local.escape_punct =
                '\\'
            >>  qi::repeat(1)[qi::punct]            [member_assign(&quickbook::formatted::content)]
                                                    [member_assign(&quickbook::formatted::type, "")]
            ;

        local.escape_markup =
                ("'''" >> -eol)
            >>  (*(qi::char_ - "'''"))              [member_assign(&quickbook::formatted::content)]
                                                    [member_assign(&quickbook::formatted::type, "escape")]
            >>  "'''"
            ;

        local.escape_unicode16 =
                "\\u"
            >   qi::raw[qi::repeat(4)[qi::xdigit]]  [member_assign(&quickbook::unicode_char::value)]
            ;

        local.escape_unicode32 =
                "\\U"
            >   qi::raw[qi::repeat(8)[qi::xdigit]]  [member_assign(&quickbook::unicode_char::value)]
            ;

        // Make sure that we don't go past a single block, except when
        // preformatted.
        phrase_end =
            ']' | qi::eps(ph::ref(no_eols)) >> eol >> *qi::blank >> qi::eol
            ;
    }
}
