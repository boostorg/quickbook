/*=============================================================================
    Copyright (c) 2006 Joel de Guzman
    Copyright (c) 2010 Daniel James
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
#include <boost/scoped_ptr.hpp>
#include "fwd.hpp"
#include "code_snippet_types.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"

namespace quickbook
{
    // TODO: End of file as well as end of line?

    namespace qi = boost::spirit::qi;
    namespace repo = boost::spirit::repository;

    struct python_code_snippet_grammar
        : qi::grammar<iterator>
    {
        typedef snippet_actions actions_type;
    
        python_code_snippet_grammar(actions_type& actions);
        ~python_code_snippet_grammar();

        struct rules;
        boost::scoped_ptr<rules> rules_pimpl;
        qi::rule<iterator> start;
    private:
        python_code_snippet_grammar(python_code_snippet_grammar const&);
        python_code_snippet_grammar& operator=(python_code_snippet_grammar const&);
    };

    struct cpp_code_snippet_grammar
        : qi::grammar<iterator>
    {
        typedef snippet_actions actions_type;
    
        cpp_code_snippet_grammar(actions_type& actions);
        ~cpp_code_snippet_grammar();

        struct rules;
        boost::scoped_ptr<rules> rules_pimpl;
        qi::rule<iterator> start;
    private:
        cpp_code_snippet_grammar(cpp_code_snippet_grammar const&);
        cpp_code_snippet_grammar& operator=(cpp_code_snippet_grammar const&);
    };

    struct python_code_snippet_grammar::rules
    {
        typedef snippet_actions actions_type;
  
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

        code_elements =
                start_snippet                   [actions]
            |   end_snippet                     [actions]
            |   escaped_comment                 [actions]
            |   ignore
            |   +qi::blank                      [actions]
            |   qi::char_                       [actions]
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
        typedef snippet_actions actions_type;
  
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
                start_snippet                   [actions]
            |   end_snippet                     [actions]
            |   escaped_comment                 [actions]
            |   ignore
            |   line_callout                    [actions]
            |   inline_callout                  [actions]
            |   +qi::blank                      [actions]
            |   qi::char_                       [actions]
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

    void load_code_snippets(
        snippet_actions& actions
      , std::vector<define_template>& storage   // for storing snippets are stored in a
                                                // vector of define_templates
      , bool is_python
      , iterator& first
      , iterator last)
    {
        // TODO: Should I check that parse succeeded?
        if(is_python) {
            python_code_snippet_grammar g(actions);
            boost::spirit::qi::parse(first, last, g);
        }
        else {
            cpp_code_snippet_grammar g(actions);
            boost::spirit::qi::parse(first, last, g);
        }
    }
}
