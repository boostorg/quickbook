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
#include "code_snippet_grammar.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;

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
        qi::rule<iterator, quickbook::escaped_comment()>
            escaped_comment;
        qi::rule<iterator, std::string()>
            escaped_comment_body;
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
                position                        [member_assign(&quickbook::code_snippet::position)]
            >>  "#["
            >>  qi::omit[*qi::space]
            >>  identifier                      [member_assign(&quickbook::code_snippet::identifier)]
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

        escaped_comment = escaped_comment_body [member_assign(&quickbook::escaped_comment::content)];

        escaped_comment_body =
                    qi::omit[*qi::space >> "#`"]
                >>  (
                        *(qi::char_ - qi::eol)
                    >>  qi::eol
                    )
            |       qi::omit[*qi::space >> "\"\"\"`"]
                >>  (
                        *(qi::char_ - "\"\"\"")
                    )
                >>  "\"\"\""
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
        qi::rule<iterator, std::string()>
            inline_callout_body, line_callout_body;
        qi::rule<iterator, quickbook::escaped_comment()>
            escaped_comment;
        qi::rule<iterator, std::string()>
            escaped_comment_body;
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
                position                        [member_assign(&quickbook::code_snippet::position)]
            >>  "//["
            >>  qi::omit[*qi::space]
            >>  identifier                      [member_assign(&quickbook::code_snippet::identifier)]
            >>  qi::omit[*(code_elements - "//]")]
            >>  "//]"
            |
                position                        [member_assign(&quickbook::code_snippet::position)]
            >>  "/*["
            >>  qi::omit[*qi::space]
            >>  identifier                      [member_assign(&quickbook::code_snippet::identifier)]
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
            >>  position                        [member_assign(&quickbook::callout::position)]
                                                [member_assign(&quickbook::callout::role, "callout_bug")]
            >>  inline_callout_body             [member_assign(&quickbook::callout::content)]
            >>  ">*/"
            ;

        inline_callout_body =
                *(qi::char_ - ">*/")
            ;

        line_callout =
                "/*<<"
            >>  position                        [member_assign(&quickbook::callout::position)]
            >>  line_callout_body               [member_assign(&quickbook::callout::content)]
            >>  ">>*/"
            >>  qi::omit[*qi::space]            [member_assign(&quickbook::callout::role, "line_callout_bug")]
            ;

        line_callout_body =
                *(qi::char_ - ">>*/")
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

        escaped_comment = escaped_comment_body [member_assign(&quickbook::escaped_comment::content)];

        escaped_comment_body =
                qi::omit[*qi::space]
            >>  "//`"
            >>  (
                    (*(qi::char_ - qi::eol))
                >>  qi::eol
                )
            |   qi::omit[*qi::space]
            >>  "/*`"
            >>  (
                    *(qi::char_ - "*/")
                )
            >> "*/"
            ;
    }
}
