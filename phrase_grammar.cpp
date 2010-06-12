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
#include "template.hpp"
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

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;

    void quickbook_grammar::impl::init_phrase()
    {
        qi::rule<iterator>& macro = store_.create();
        qi::rule<iterator>& phrase_markup = store_.create();
        qi::rule<iterator, quickbook::code()>& code_block = store_.create();
        qi::rule<iterator, quickbook::code()>& inline_code = store_.create();
        qi::rule<iterator, quickbook::simple_markup(), qi::locals<char> >& simple_format = store_.create();
        qi::rule<iterator>& escape = store_.create();
        qi::rule<iterator, quickbook::call_template()>& call_template = store_.create();
        qi::rule<iterator, quickbook::break_()>& break_ = store_.create();
        qi::rule<iterator>& phrase_end = store_.create();

        simple_phrase =
           *(   common
            |   comment
            |   (qi::char_ - ']')               [actions.process]
            )
            ;

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

        qi::rule<iterator, qi::locals<qi::rule<iterator> > >& phrase_markup_impl = store_.create();

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
        qi::rule<iterator, quickbook::unicode_char()>& escape_unicode16 = store_.create();
        qi::rule<iterator, quickbook::unicode_char()>& escape_unicode32 = store_.create();

        escape =
            (   escape_break
            |   "\\ "                               // ignore an escaped char            
            |   escape_punct
            |   escape_unicode16
            |   escape_unicode32
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

        escape_unicode16 =
                "\\u"
            >   qi::raw[qi::repeat(4)[qi::xdigit]]
            >   qi::attr(nothing())
            ;

        escape_unicode32 =
                "\\U"
            >   qi::raw[qi::repeat(8)[qi::xdigit]]
            >   qi::attr(nothing())
            ;

        phrase_end =
            ']' |
            qi::eps(ph::ref(no_eols)) >>
                eol >> eol                      // Make sure that we don't go
            ;                                   // past a single block, except
                                                // when preformatted.
    }
}
