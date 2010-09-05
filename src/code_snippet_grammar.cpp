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
#include <boost/spirit/repository/include/qi_confix.hpp>
#include "fwd.hpp"
#include "code_snippet_types.hpp"
#include "code_snippet_grammar.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"
#include "utils.hpp" // For 'detail::load', rewrite so it isn't used here?

namespace quickbook
{
    // TODO: End of file as well as end of line?

    namespace qi = boost::spirit::qi;
    namespace repo = boost::spirit::repository;

    struct python_code_snippet_grammar::rules
    {
        typedef code_snippet_actions actions_type;
  
        rules(actions_type & actions);

        actions_type& actions;

        qi::rule<iterator>
            start_, ignore;
        qi::rule<iterator, quickbook::start_snippet()>
            start_snippet;
        qi::rule<iterator, quickbook::end_snippet()>
            end_snippet;
        qi::rule<iterator>
            code_elements;
        qi::rule<iterator, std::string()>
            identifier;
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
        start_ = *code_elements;

        identifier =
            (qi::alpha | '_') >> *(qi::alnum | '_')
            ;

        /*
        snippet =
                position                        [member_assign(&quickbook::code_snippet::position)]
            >>  repo::confix("#[", "#]")
                [   *qi::space
                >>  identifier                  [member_assign(&quickbook::code_snippet::identifier)]
                >>  *(!qi::lit("#]") >> code_elements)
                ]
            ;
        */

        code_elements =
                start_snippet                   [actions.process]
            |   end_snippet                     [actions.process]
            |   escaped_comment                 [actions.process]
            |   ignore
            |   +qi::blank                      [actions.process]
            |   qi::char_                       [actions.process]
            ;

        start_snippet =
                position                        [member_assign(&quickbook::start_snippet::position)]
            >>  "#["
            >>  *qi::space
            >>  identifier                      [member_assign(&quickbook::start_snippet::identifier)]
            ;

        end_snippet =
                qi::lit("#]")
            >>  qi::attr(quickbook::end_snippet());

        ignore
            =   (   repo::confix(*qi::blank >> "#<-", "#->" >> *qi::blank >> qi::eol)
                        [*(qi::char_ - "#->")]
                |   repo::confix("\"\"\"<-\"\"\"", "\"\"\"->\"\"\"")
                        [*(qi::char_ - "\"\"\"->\"\"\"")]
                |   repo::confix("\"\"\"<-", "->\"\"\"")
                        [*(qi::char_ - "->\"\"\"")]
                )
            ;

        escaped_comment
            =   (   repo::confix(*qi::space >> "#`", qi::eol)
                        [*(qi::char_ - qi::eol)]    
                |   repo::confix(*qi::space >> "\"\"\"`", "\"\"\"")
                        [*(qi::char_ - "\"\"\"")]
                )   [member_assign(&quickbook::escaped_comment::content)]
            ;
    }

    struct cpp_code_snippet_grammar::rules
    {
        typedef code_snippet_actions actions_type;
  
        rules(actions_type & actions);

        actions_type& actions;

        qi::rule<iterator>
            start_, code_elements, ignore;
        qi::rule<iterator, quickbook::start_snippet()>
            start_snippet;
        qi::rule<iterator, quickbook::end_snippet()>
            end_snippet;
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
        start_ = *code_elements;

        identifier =
            (qi::alpha | '_') >> *(qi::alnum | '_')
            ;

        code_elements =
                start_snippet                   [actions.process]
            |   end_snippet                     [actions.process]
            |   escaped_comment                 [actions.process]
            |   ignore
            |   line_callout                    [actions.process]
            |   inline_callout                  [actions.process]
            |   +qi::blank                      [actions.process]
            |   qi::char_                       [actions.process]
            ;

        start_snippet
            =   position                        [member_assign(&quickbook::start_snippet::position)]
            >>  (   "//["
                >>  *qi::space
                >>  identifier                  [member_assign(&quickbook::start_snippet::identifier)]
                |   "/*["
                >>  *qi::space
                >>  identifier                  [member_assign(&quickbook::start_snippet::identifier)]
                >>  *qi::space
                >>  "*/"
                )
            ;

        end_snippet =
                (qi::lit("//]") | qi::lit("/*]*/"))
            >>  qi::attr(quickbook::end_snippet());
            ;

        inline_callout
            =   repo::confix("/*<" >> *qi::space, ">*/")
                [   position                    [member_assign(&quickbook::callout::position)]
                                                [member_assign(&quickbook::callout::role, "callout_bug")]
                >>  (*(qi::char_ - ">*/"))      [member_assign(&quickbook::callout::content)]
                ]
            ;

        line_callout
            =   repo::confix("/*<<" >> *qi::space, ">>*/" >> *qi::space)
                [   position                    [member_assign(&quickbook::callout::position)]
                                                [member_assign(&quickbook::callout::role, "line_callout_bug")]
                >>  (*(qi::char_ - ">>*/"))     [member_assign(&quickbook::callout::content)]
                ]
            ;

        ignore
            =   repo::confix(*qi::blank >> "//<-", "//->" >> *qi::blank >> qi::eol)
                    [*(qi::char_ - "//->")]
            |   repo::confix("/*<-*/", "/*->*/")
                    [*(qi::char_ - "/*->*/")]
            |   repo::confix("/*<-", "->*/")
                    [*(qi::char_ - "->*/")]
            ;

        escaped_comment =
                *qi::space
            >>  (   repo::confix("//`", qi::eol)
                        [*(qi::char_ - qi::eol)]
                |   repo::confix("/*`", "*/")
                        [*(qi::char_ - "*/")]
                )   [member_assign(&quickbook::escaped_comment::content)]
            ;
    }

    int load_snippets(
        std::string const& file
      , std::vector<define_template>& storage   // for storing snippets are stored in a
                                                // vector of define_templates
      , std::string const& extension
      , std::string const& doc_id)
    {
        std::string code;
        int err = detail::load(file, code);
        if (err != 0)
            return err; // return early on error

        iterator first(code.begin(), code.end(), file.c_str());
        iterator last(code.end(), code.end());

        size_t fname_len = file.size();
        bool is_python = fname_len >= 3
            && file[--fname_len]=='y' && file[--fname_len]=='p' && file[--fname_len]=='.';
        code_snippet_actions a(storage, doc_id, is_python ? "[python]" : "[c++]");
        // TODO: Should I check that parse succeeded?
        if(is_python) {
            python_code_snippet_grammar g(a);
            boost::spirit::qi::parse(first, last, g);
        }
        else {
            cpp_code_snippet_grammar g(a);
            boost::spirit::qi::parse(first, last, g);
        }

        return 0;
    }
}
