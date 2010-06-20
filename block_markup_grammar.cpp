/*=============================================================================
    Copyright (c) 2002 2004  2006Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include "grammar_impl.hpp"
#include "block.hpp"
#include "actions.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;
    
    // Workaround for clang:
    namespace {
        struct dummmy {
            qi::rule<iterator, raw_string()> a1;
        };
    }

    void quickbook_grammar::impl::init_block_markup()
    {
        // Paragraph Blocks

        qi::rule<iterator, quickbook::block_formatted(formatted_type)>& paragraph_block = store_.create();

        block_keyword_rules.add
            ("blurb", paragraph_block(formatted_type("blurb")) [actions.process])
            ("warning", paragraph_block(formatted_type("warning")) [actions.process])
            ("caution", paragraph_block(formatted_type("caution")) [actions.process])
            ("important", paragraph_block(formatted_type("important")) [actions.process])
            ("note", paragraph_block(formatted_type("note")) [actions.process])
            ("tip", paragraph_block(formatted_type("tip")) [actions.process])
            ;

        block_symbol_rules.add
            (":", paragraph_block(formatted_type("blockquote")) [actions.process])
            ;

        paragraph_block =
                qi::attr(qi::_r1)                   [member_assign(&quickbook::block_formatted::type)]
            >>  space
            >>  inside_paragraph                    [member_assign(&quickbook::block_formatted::content)]
            ;

        // Preformatted

        qi::rule<iterator, quickbook::block_formatted()>& preformatted = store_.create();

        block_keyword_rules.add("pre", preformatted [actions.process]);
        
        preformatted =
                space                           [ph::ref(no_eols) = false]
                                                [member_assign(&quickbook::block_formatted::type, "preformatted")]
            >>  -eol
            >>  phrase                          [member_assign(&quickbook::block_formatted::content)]
            >>  qi::eps                         [ph::ref(no_eols) = true]
            ;

        // Define Macro

        qi::rule<iterator, quickbook::def_macro()>& def_macro = store_.create();

        block_keyword_rules.add("def", def_macro[actions.process]);
        
        def_macro =
                space
            >>  macro_identifier                [member_assign(&quickbook::def_macro::macro_identifier)]
            >>  blank
            >>  phrase                          [member_assign(&quickbook::def_macro::content)]
            ;

        // xinclude

        qi::rule<iterator, quickbook::xinclude()>& xinclude = store_.create();

        block_keyword_rules.add("xinclude", xinclude[actions.process]);

        // TODO: Why do these use phrase_end? It doesn't make any sense.
        xinclude =
                space
            >>  (*(qi::char_ - phrase_end))         [member_assign(&quickbook::xinclude::path)]
            ;

        qi::rule<iterator, quickbook::include()>& include = store_.create();
        qi::rule<iterator, raw_string()>& include_id = store_.create();
        
        block_keyword_rules.add("include", include[actions.process]);

        // Include

        include =
                space
            >>  -(
                    ':'
                >>  include_id
                >>  space
                )                                   [member_assign(&quickbook::include::id)]
            >>  (*(qi::char_ - phrase_end))         [member_assign(&quickbook::include::path)]
            ;

        include_id = qi::raw[*((qi::alnum | '_') - qi::space)]
                                            [qi::_val = qi::_1];

        // Import

        qi::rule<iterator, quickbook::import()>& import = store_.create();

        block_keyword_rules.add("import", import[actions.process]);
        
        import =
                space
            >>  (*(qi::char_ - phrase_end))         [member_assign(&quickbook::import::path)]
            ;
    }
}