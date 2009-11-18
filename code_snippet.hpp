/*=============================================================================
    Copyright (c) 2006 Joel de Guzman
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_CODE_SNIPPET_HPP)
#define BOOST_SPIRIT_QUICKBOOK_CODE_SNIPPET_HPP

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_actor.hpp>
#include <boost/bind.hpp>
#include "./grammars.hpp"
#include "./detail/template_stack.hpp"
#include "./detail/actions.hpp"

namespace quickbook
{
    template <typename Scanner>
    python_code_snippet_grammar::definition<Scanner>::definition(
        python_code_snippet_grammar const& self)
    {
        actions_type& actions = self.actions;
    
        start_ =
            +(
                    snippet                     [boost::bind(&actions_type::compile, &actions, _1, _2)]
                |   classic::anychar_p
            )
            ;

        identifier =
            (classic::alpha_p | '_') >> *(classic::alnum_p | '_')
            ;

        snippet =
            "#[" >> *classic::space_p
            >> identifier                       [classic::assign_a(actions.id)]
            >> (*(code_elements - "#]"))
            >> "#]"
            ;

        code_elements =
                escaped_comment
            |   ignore
            |   (classic::anychar_p - "#]")     [boost::bind(&actions_type::pass_thru, &actions, _1, _2)]
            ;

        ignore =
                *classic::blank_p >> "#<-"
                >> (*(classic::anychar_p - "#->"))
                >> "#->" >> *classic::blank_p >> classic::eol_p
            |   "\"\"\"<-\"\"\""
                >> (*(classic::anychar_p - "\"\"\"->\"\"\""))
                >> "\"\"\"->\"\"\""
            |   "\"\"\"<-"
                >> (*(classic::anychar_p - "->\"\"\""))
                >> "->\"\"\""
            ;

        escaped_comment =
                *classic::space_p >> "#`"
                >> ((*(classic::anychar_p - classic::eol_p))
                    >> classic::eol_p)          [boost::bind(&actions_type::escaped_comment, &actions, _1, _2)]
            |   *classic::space_p >> "\"\"\"`"
                >> (*(classic::anychar_p - "\"\"\""))
                                                [boost::bind(&actions_type::escaped_comment, &actions, _1, _2)]
                >> "\"\"\""
            ;
    }

    template <typename Scanner>
    cpp_code_snippet_grammar::definition<Scanner>::definition(
        cpp_code_snippet_grammar const& self)
    {
        actions_type& actions = self.actions;
    
        start_ =
            +(
                    snippet                     [boost::bind(&actions_type::compile, &actions, _1, _2)]
                |   classic::anychar_p
            )
            ;

        identifier =
            (classic::alpha_p | '_') >> *(classic::alnum_p | '_')
            ;

        snippet =
                "//[" >> *classic::space_p
                >> identifier                   [classic::assign_a(actions.id)]
                >> (*(code_elements - "//]"))
                >> "//]"
            |
                "/*[" >> *classic::space_p
                >> identifier                   [classic::assign_a(actions.id)]
                >> *classic::space_p >> "*/"
                >> (*(code_elements - "/*]*"))
                >> "/*]*/"
            ;

        code_elements =
                escaped_comment
            |   ignore
            |   line_callout
            |   inline_callout
            |   (classic::anychar_p - "//]" - "/*]*/")
                                                [boost::bind(&actions_type::pass_thru, &actions, _1, _2)]
            ;

        inline_callout =
            "/*<"
            >> (*(classic::anychar_p - ">*/"))  [boost::bind(&actions_type::inline_callout, &actions, _1, _2)]
            >> ">*/"
            ;

        line_callout =
            "/*<<"
            >> (*(classic::anychar_p - ">>*/")) [boost::bind(&actions_type::line_callout, &actions, _1, _2)]
            >> ">>*/"
            >> *classic::space_p
            ;

        ignore =
                *classic::blank_p >> "//<-"
                >> (*(classic::anychar_p - "//->"))
                >> "//->" >> *classic::blank_p >> classic::eol_p
            |   "/*<-*/"
                >> (*(classic::anychar_p - "/*->*/"))
                >> "/*->*/"
            |   "/*<-"
                >> (*(classic::anychar_p - "->*/"))
                >> "->*/"
            ;

        escaped_comment =
                *classic::space_p >> "//`"
                >> ((*(classic::anychar_p - classic::eol_p))
                    >> classic::eol_p)          [boost::bind(&actions_type::escaped_comment, &actions, _1, _2)]
            |   *classic::space_p >> "/*`"
                >> (*(classic::anychar_p - "*/"))
                                                [boost::bind(&actions_type::escaped_comment, &actions, _1, _2)]
                >> "*/"
            ;
    }
}

#endif // BOOST_SPIRIT_QUICKBOOK_CODE_SNIPPET_HPP

