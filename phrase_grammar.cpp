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
#include "grammar_impl.hpp"
#include "phrase.hpp"
#include "code.hpp"
#include "actions.hpp"
#include "template.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"

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
        qi::rule<iterator, std::string(char)>& simple_format_body = store_.create();
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
                position                            [member_assign(&quickbook::call_template::position)]
            >>  qi::matches['`']                    [member_assign(&quickbook::call_template::escape)]
            >>  (                                   // Lookup the template name
                    (&qi::punct >> actions.templates.scope)
                |   (actions.templates.scope >> hard_space)
                )                                   [member_assign(&quickbook::call_template::symbol)]
            >>  template_args                       [member_assign(&quickbook::call_template::args)]
            >>  &qi::lit(']')
            ;

        template_args =
            qi::eps(qbk_before(105u)) >> -(template_arg_1_4 % "..") |
            qi::eps(qbk_since(105u)) >> -(template_arg_1_5 % "..");

        template_arg_1_4 =
                position                            [member_assign(&quickbook::template_value::position)]
            >>  qi::raw[+(brackets_1_4 | ~qi::char_(']') - "..")]
                                                    [member_assign(&quickbook::template_value::content)]
            ;

        brackets_1_4 =
            '[' >> +(brackets_1_4 | ~qi::char_(']') - "..") >> ']'
            ;

        template_arg_1_5 =
                position                            [member_assign(&quickbook::template_value::position)]
            >>  qi::raw[+(brackets_1_5 | '\\' >> qi::char_ | ~qi::char_("[]") - "..")]
                                                    [member_assign(&quickbook::template_value::content)]
            ;

        brackets_1_5 =
            '[' >> +(brackets_1_5 | '\\' >> qi::char_ | ~qi::char_("[]")) >> ']'
            ;

        break_ =
                position                            [member_assign(&quickbook::break_::position)]
            >>  "br"
            ;

        qi::rule<iterator, std::string()>& code_block1 = store_.create();
        qi::rule<iterator, std::string()>& code_block2 = store_.create();
        qi::rule<iterator, std::string()>& inline_code_block = store_.create();

        code_block =
                (
                    "```"
                >>  position                        [member_assign(&quickbook::code::position)]
                                                    [member_assign(&quickbook::code::block, true)]
                >>  code_block1                     [member_assign(&quickbook::code::content)]
                >>  "```"
                )
            |   (
                    "``"
                >>  position                        [member_assign(&quickbook::code::position)]
                                                    [member_assign(&quickbook::code::block, true)]
                >>  code_block2                     [member_assign(&quickbook::code::content)]
                >>  "``"
                )
            ;

        code_block1 = qi::raw[*(qi::char_ - "```")];
        code_block2 = qi::raw[*(qi::char_ - "``")];

        inline_code =
                '`'
            >>  position                            [member_assign(&quickbook::code::position)]
                                                    [member_assign(&quickbook::code::block, false)]
            >>  inline_code_block                   [member_assign(&quickbook::code::content)]
            >>  '`'
            ;

        inline_code_block =
            qi::raw
            [   *(  qi::char_ -
                    (   '`'
                    |   (eol >> eol)            // Make sure that we don't go
                    )                           // past a single block
                )
                >>  &qi::lit('`')
            ]
            ;

        qi::rule<iterator>& simple_phrase_end = store_.create();

        simple_format =
                qi::char_("*/_=")               [qi::_a = qi::_1]
            >>  simple_format_body(qi::_a)      [member_assign(&quickbook::simple_markup::raw_content)]
            >>  qi::char_(qi::_a)               [member_assign(&quickbook::simple_markup::symbol)]
            ;

        simple_format_body =
                qi::raw
                [   (   (   qi::graph               // A single char. e.g. *c*
                        >>  &(  qi::char_(qi::_r1)
                            >>  (qi::space | qi::punct | qi::eoi)
                            )
                        )
                    |
                        (   qi::graph               // qi::graph must follow qi::lit(qi::_r1)
                        >>  *(  qi::char_ -
                                (   (qi::graph >> qi::lit(qi::_r1))
                                |   simple_phrase_end // Make sure that we don't go
                                )                     // past a single block
                            )
                        >>  qi::graph               // qi::graph must precede qi::lit(qi::_r1)
                        >>  &(  qi::char_(qi::_r1)
                            >>  (qi::space | qi::punct | qi::eoi)
                            )
                        )
                    )
                ]
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
                position                            [member_assign(&quickbook::break_::position)]
            >>  "\\n"
            ;

        escape_punct =
                '\\'
            >>  qi::repeat(1)[qi::punct]            [member_assign(&quickbook::formatted::content)]
                                                    [member_assign(&quickbook::formatted::type, "")]
            ;

        escape_markup =
                ("'''" >> -eol)
            >>  (*(qi::char_ - "'''"))              [member_assign(&quickbook::formatted::content)]
                                                    [member_assign(&quickbook::formatted::type, "escape")]
            >>  "'''"
            ;

        escape_unicode16 =
                "\\u"
            >   qi::raw[qi::repeat(4)[qi::xdigit]]  [member_assign(&quickbook::unicode_char::value)]
            ;

        escape_unicode32 =
                "\\U"
            >   qi::raw[qi::repeat(8)[qi::xdigit]]  [member_assign(&quickbook::unicode_char::value)]
            ;

        phrase_end =
            ']' |
            qi::eps(ph::ref(no_eols)) >>
                eol >> eol                      // Make sure that we don't go
            ;                                   // past a single block, except
                                                // when preformatted.
    }
}
