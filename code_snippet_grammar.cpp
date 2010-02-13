/*=============================================================================
    Copyright (c) 2006 Joel de Guzman
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include "fwd.hpp"
#include "code_snippet_types.hpp"
#include "grammars.hpp"
#include "parse_utils.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;

    namespace
    {
        // Shared rules
        
        qi::rule<iterator, file_position()>
            position = qi::raw[qi::eps] [get_position];
    }

    struct python_code_snippet_grammar::rules
    {
        typedef code_snippet_actions actions_type;
  
        rules(actions_type & actions);

        actions_type& actions;

        qi::rule<iterator>
            start_, ignore;
        qi::rule<iterator, quickbook::code_snippet()>
            snippet;
        qi::rule<iterator>
            code_elements;
        qi::rule<iterator, std::string()>
            identifier;
        qi::rule<iterator, quickbook::callout()>
            inline_callout, line_callout;
        qi::rule<iterator, quickbook::escaped_comment()>
            escaped_comment;
    };  

    python_code_snippet_grammar::python_code_snippet_grammar(actions_type & actions)
        : python_code_snippet_grammar::base_type(start)
        , rules_pimpl(new rules(actions))
        , start(rules_pimpl->start_) {}

    python_code_snippet_grammar::~python_code_snippet_grammar() {}

    python_code_snippet_grammar::rules::rules(actions_type& actions)
        : actions(actions)
    {
        start_ =
            +(
                    snippet                     [actions.output]
                |   qi::char_
            )
            ;

        identifier =
            (qi::alpha | '_') >> *(qi::alnum | '_')
            ;

        snippet =
                position
            >>  "#["
            >>  qi::omit[*qi::space]
            >>  identifier
            >>  qi::omit[*(code_elements - "#]")]
            >>  "#]"
            ;

        code_elements =
                escaped_comment                 [actions.process]
            |   ignore
            |   (qi::char_ - "#]")              [actions.process]
            ;

        ignore =
                *qi::blank >> "#<-"
                >> (*(qi::char_ - "#->"))
                >> "#->" >> *qi::blank >> qi::eol
            |   "\"\"\"<-\"\"\""
                >> (*(qi::char_ - "\"\"\"->\"\"\""))
                >> "\"\"\"->\"\"\""
            |   "\"\"\"<-"
                >> (*(qi::char_ - "->\"\"\""))
                >> "->\"\"\""
            ;

        escaped_comment =
                    qi::omit[*qi::space >> "#`"]
                >>  (
                        *(qi::char_ - qi::eol)
                    >>  qi::eol
                    )
                >>  qi::attr(nothing())
            |       qi::omit[*qi::space >> "\"\"\"`"]
                >>  (
                        *(qi::char_ - "\"\"\"")
                    )
                >>  "\"\"\""
                >>  qi::attr(nothing())
            ;
    }

    struct cpp_code_snippet_grammar::rules
    {
        typedef code_snippet_actions actions_type;
  
        rules(actions_type & actions);

        actions_type& actions;

        qi::rule<iterator>
            start_, code_elements, ignore;
        qi::rule<iterator, quickbook::code_snippet()>
            snippet;
        qi::rule<iterator, std::string()>
            identifier;
        qi::rule<iterator, quickbook::callout()>
            inline_callout, line_callout;
        qi::rule<iterator, quickbook::escaped_comment()>
            escaped_comment;
    };

    cpp_code_snippet_grammar::cpp_code_snippet_grammar(actions_type & actions)
        : cpp_code_snippet_grammar::base_type(start)
        , rules_pimpl(new rules(actions))
        , start(rules_pimpl->start_) {}

    cpp_code_snippet_grammar::~cpp_code_snippet_grammar() {}

    cpp_code_snippet_grammar::rules::rules(actions_type & actions)
        : actions(actions)
    {
        start_ =
            +(
                    snippet                     [actions.output]
                |   qi::char_
            )
            ;

        identifier =
            (qi::alpha | '_') >> *(qi::alnum | '_')
            ;

        snippet =
                position
            >>  "//["
            >>  qi::omit[*qi::space]
            >>  identifier
            >>  qi::omit[*(code_elements - "//]")]
            >>  "//]"
            |
                position
            >>  "/*["
            >>  qi::omit[*qi::space]
            >>  identifier
            >>  qi::omit[*qi::space >> "*/"]
            >>  qi::omit[*(code_elements - "/*]*")]
            >>  "/*]*/"
            ;

        code_elements =
                escaped_comment                 [actions.process]
            |   ignore
            |   line_callout                    [actions.process]
            |   inline_callout                  [actions.process]
            |   (qi::char_ - "//]" - "/*]*/")   [actions.process]
            ;

        inline_callout =
                "/*<"
            >>  position
            >>  *(qi::char_ - ">*/")
            >>  ">*/"
            >>  qi::attr("callout_bug")
            ;

        line_callout =
                "/*<<"
            >>  position
            >>  *(qi::char_ - ">>*/")
            >>  ">>*/"
            >>  qi::omit[*qi::space]
            >>  qi::attr("line_callout_bug")
            ;

        ignore =
                *qi::blank >> "//<-"
                >> (*(qi::char_ - "//->"))
                >> "//->" >> *qi::blank >> qi::eol
            |   "/*<-*/"
                >> (*(qi::char_ - "/*->*/"))
                >> "/*->*/"
            |   "/*<-"
                >> (*(qi::char_ - "->*/"))
                >> "->*/"
            ;

        escaped_comment =
                qi::omit[*qi::space]
            >>  "//`"
            >>  (
                    (*(qi::char_ - qi::eol))
                >>  qi::eol
                )
            >>  qi::attr(nothing())
            |   qi::omit[*qi::space]
            >>  "/*`"
            >>  (
                    *(qi::char_ - "*/")
                )
            >> "*/"
            >>  qi::attr(nothing())
            ;
    }
}
