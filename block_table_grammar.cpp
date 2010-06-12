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
#include "code.hpp"
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

    void quickbook_grammar::impl::init_block_table()
    {
        // Table

        qi::rule<iterator, quickbook::table()>& table = store_.create();
        qi::rule<iterator, quickbook::table_row()>& table_row = store_.create();
        qi::rule<iterator, quickbook::table_cell()>& table_cell = store_.create();
        qi::rule<iterator, quickbook::formatted()>& table_cell_body = store_.create();
        
        block_keyword_rules.add("table", table[actions.process]);

        table =
                (&(*qi::blank >> qi::eol) | space)
            >>  ((
                    qi::eps(qbk_since(105u))
                >>  element_id                  [member_assign(&quickbook::table::id)]
                ) | qi::eps)
            >>  (&(*qi::blank >> qi::eol) | space)
            >>  (*(qi::char_ - eol))            [member_assign(&quickbook::table::title)]
            >>  +eol
            >>  (*table_row)                    [member_assign(&quickbook::table::rows)]
            ;

        table_row =
                space
            >>  '['
            >>  (   *table_cell >> ']' >> space
                |   error >> qi::attr(quickbook::table_row())
                )
            ;

        table_cell =
                space
            >>  '['
            >>  (   table_cell_body >> ']' >> space
                |   error >> qi::attr(quickbook::table_cell())
                )
            ;

        table_cell_body =
                inside_paragraph                    [member_assign(&quickbook::formatted::content)]
                                                    [member_assign(&quickbook::formatted::type, "cell")]
            ;

        qi::rule<iterator, quickbook::variablelist()>& variablelist = store_.create();
        qi::rule<iterator, quickbook::varlistentry()>& varlistentry = store_.create();
        qi::rule<iterator, quickbook::formatted()>& varlistterm = store_.create();
        qi::rule<iterator, quickbook::formatted()>& varlistterm_body = store_.create();
        qi::rule<iterator, quickbook::formatted()>& varlistitem = store_.create();
        qi::rule<iterator, quickbook::formatted()>& varlistitem_body = store_.create();
        
        block_keyword_rules.add("variablelist", variablelist[actions.process]);

        variablelist =
                (&(*qi::blank >> qi::eol) | space)
            >>  (*(qi::char_ - eol))                [member_assign(&quickbook::variablelist::title)]
            >>  +eol
            >>  (*varlistentry)                     [member_assign(&quickbook::variablelist::entries)]
            ;
            
        varlistentry =
                space
            >>  '['
            >>  (   varlistterm
                >>  +varlistitem
                >>  ']'
                >>  space
                |   error >> qi::attr(quickbook::varlistentry())
                )
            ;

        varlistterm =
                space
            >>  '['
            >>  (   varlistterm_body >> ']' >> space
                |   error >> qi::attr(quickbook::formatted())
                )
            ;

        varlistterm_body =
                phrase_attr                         [member_assign(&quickbook::formatted::content)]
                                                    [member_assign(&quickbook::formatted::type, "varlistterm")]
            ;

        varlistitem =
                space
            >>  '['
            >>  (   varlistitem_body >> ']' >> space
                |   error >> qi::attr(quickbook::formatted())
                )
            ;

        varlistitem_body =
                inside_paragraph                    [member_assign(&quickbook::formatted::content)]
                                                    [member_assign(&quickbook::formatted::type, "varlistitem")]
            ;
    }
}