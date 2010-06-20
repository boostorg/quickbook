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
    
    void quickbook_grammar::impl::init_template()
    {
        // Define Template

        qi::rule<iterator, quickbook::define_template()>& define_template = store_.create();
        qi::rule<iterator, std::vector<std::string>()>& define_template_params = store_.create();
        qi::rule<iterator, quickbook::template_value()>& template_body = store_.create();
        qi::rule<iterator>& template_body_recurse = store_.create();
        qi::rule<iterator, std::string()>& template_id = store_.create();
        
        block_keyword_rules.add("template", define_template[actions.process]);

        define_template =
                space
            >>  template_id                         [member_assign(&quickbook::define_template::id)]
            >>  -define_template_params             [member_assign(&quickbook::define_template::params)]
            >>  template_body                       [member_assign(&quickbook::define_template::body)]
            ;

        define_template_params =
                space
            >>  '['
            >>  *(space >> template_id)
            >>  space
            >>  ']'
            ;

        template_body =
                position                            [member_assign(&quickbook::template_value::position)]
            >>  qi::raw[template_body_recurse]      [member_assign(&quickbook::template_value::content)]
            ;

        template_body_recurse =
                *(  ('[' >> template_body_recurse >> ']')
                |   (qi::char_ - ']')
                )
            >>  space
            >>  &qi::lit(']')
            ;

        template_id
            =   (qi::alpha | '_') >> *(qi::alnum | '_')
            |   qi::repeat(1)[qi::punct - qi::char_("[]")]
            ;

		// Call Template

        qi::rule<iterator, quickbook::call_template()>& call_template_impl = store_.create();
        qi::rule<iterator, std::vector<quickbook::template_value>()>& template_args = store_.create();
        qi::rule<iterator, quickbook::template_value()>& template_arg_1_4 = store_.create();
        qi::rule<iterator>& brackets_1_4 = store_.create();
        qi::rule<iterator, quickbook::template_value()>& template_arg_1_5 = store_.create();
        qi::rule<iterator>& brackets_1_5 = store_.create();

		call_template = call_template_impl[actions.process];

        call_template_impl =
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
    }
}
