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
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include "phrase_grammar.hpp"
#include "code.hpp"
#include "actions.hpp"
#include "misc_rules.hpp"

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::simple_markup,
    (char, symbol)
    (std::string, raw_content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::unicode_char,
    (std::string, value)
)

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;

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
        init_phrase_markup();

        qi::rule<iterator>& macro = store_.create();
        qi::rule<iterator, quickbook::code()>& code_block = store_.create();
        qi::rule<iterator, quickbook::code()>& inline_code = store_.create();
        qi::rule<iterator, quickbook::simple_markup(), qi::locals<char> >& simple_format = store_.create();
        qi::rule<iterator>& escape = store_.create();
        qi::rule<iterator>& phrase_end = store_.create();

        phrase =
                qi::eps                         [actions.phrase_push]        
            >> *(   common
                |   comment
                |   (qi::char_ - phrase_end)    [actions.process]
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

        qi::rule<iterator>& simple_phrase_end = store_.create();

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
                                |   simple_phrase_end // Make sure that we don't go
                                )                     // past a single block
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

        simple_phrase_end = '[' | phrase_end;

        qi::rule<iterator, quickbook::break_()>& escape_break = store_.create();
        qi::rule<iterator, quickbook::formatted()>& escape_punct = store_.create();
        qi::rule<iterator, quickbook::formatted()>& escape_markup = store_.create();
        qi::rule<iterator, quickbook::unicode_char()>& escape_unicode = store_.create();

        escape =
            (   escape_break
            |   "\\ "                               // ignore an escaped char            
            |   escape_punct
            |   escape_unicode
            |   escape_markup                       
            )                                       [actions.process]
            ;
        
        escape_break =
                position
            >>  "\\n"
            >>  qi::attr(nothing())
            ;

        escape_punct =
                qi::attr(formatted_type(""))
            >>  '\\'
            >>  qi::repeat(1)[qi::punct]
            ;

        escape_markup =
                ("'''" >> -eol)
            >>  qi::attr("escape")
            >>  *(qi::char_ - "'''")
            >>  "'''"
            ;

        escape_unicode =
                "\\u"
            >>  qi::raw[qi::repeat(1,4)[qi::hex]]
            >>  qi::attr(nothing())
            ;

        phrase_end =
            ']' |
            qi::eps(ph::ref(no_eols)) >>
                eol >> eol                      // Make sure that we don't go
            ;                                   // past a single block, except
                                                // when preformatted.
    }

    struct simple_phrase_grammar::rules
    {
        rules(quickbook::actions& actions);

        quickbook::actions& actions;
        bool unused;
        phrase_grammar common;
        qi::rule<iterator> phrase;
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
            |   (qi::char_ - ']')               [actions.process]
            )
            ;
    }
}
