/*=============================================================================
    Copyright (c) 2002 2004  2006Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include "grammar_impl.hpp"
#include "template.hpp"
#include "actions.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    
    struct template_grammar_local
    {
        qi::rule<iterator, quickbook::define_template()> define_template;
        qi::rule<iterator, std::vector<std::string>()> define_template_params;
        qi::rule<iterator, quickbook::template_value()> template_body;
        qi::rule<iterator> template_body_recurse;
        qi::rule<iterator, std::string()> template_id;
        qi::rule<iterator, quickbook::call_template()> call_template;
        qi::rule<iterator, std::vector<quickbook::template_value>()> template_args;
        qi::rule<iterator, quickbook::template_value()> template_arg_1_4;
        qi::rule<iterator> brackets_1_4;
        qi::rule<iterator, quickbook::template_value()> template_arg_1_5;
        qi::rule<iterator> brackets_1_5;
    };

    void quickbook_grammar::impl::init_template()
    {
        template_grammar_local& local = store_.create();
        
        block_keyword_rules.add("template", local.define_template[actions.process]);

        local.define_template =
                space
            >>  local.template_id                   [member_assign(&quickbook::define_template::id)]
            >>  -local.define_template_params       [member_assign(&quickbook::define_template::params)]
            >>  local.template_body                 [member_assign(&quickbook::define_template::body)]
            ;

        local.define_template_params =
                space
            >>  '['
            >>  *(space >> local.template_id)
            >>  space
            >>  ']'
            ;

        local.template_body =
                position                            [member_assign(&quickbook::template_value::position)]
            >>  qi::raw[local.template_body_recurse]
                                                    [member_assign(&quickbook::template_value::content)]
            ;

        local.template_body_recurse =
                *(  ('[' >> local.template_body_recurse >> ']')
                |   (qi::char_ - ']')
                )
            >>  space
            >>  &qi::lit(']')
            ;

        local.template_id
            =   (qi::alpha | '_') >> *(qi::alnum | '_')
            |   qi::repeat(1)[qi::punct - qi::char_("[]")]
            ;

		call_template = local.call_template [actions.process];

        local.call_template =
                position                            [member_assign(&quickbook::call_template::position)]
            >>  qi::matches['`']                    [member_assign(&quickbook::call_template::escape)]
            >>  (                                   // Lookup the template name
                    (&qi::punct >> actions.templates.scope)
                |   (actions.templates.scope >> hard_space)
                )                                   [member_assign(&quickbook::call_template::symbol)]
            >>  local.template_args                 [member_assign(&quickbook::call_template::args)]
            >>  &qi::lit(']')
            ;

        local.template_args =
            qi::eps(qbk_before(105u)) >> -(local.template_arg_1_4 % "..") |
            qi::eps(qbk_since(105u)) >> -(local.template_arg_1_5 % "..");

        local.template_arg_1_4 =
                position                            [member_assign(&quickbook::template_value::position)]
            >>  qi::raw[+(local.brackets_1_4 | ~qi::char_(']') - "..")]
                                                    [member_assign(&quickbook::template_value::content)]
            ;

        local.brackets_1_4 =
            '[' >> +(local.brackets_1_4 | ~qi::char_(']') - "..") >> ']'
            ;

        local.template_arg_1_5 =
                position                            [member_assign(&quickbook::template_value::position)]
            >>  qi::raw[+(local.brackets_1_5 | '\\' >> qi::char_ | ~qi::char_("[]") - "..")]
                                                    [member_assign(&quickbook::template_value::content)]
            ;

        local.brackets_1_5 =
            '[' >> +(local.brackets_1_5 | '\\' >> qi::char_ | ~qi::char_("[]")) >> ']'
            ;
    }
}
