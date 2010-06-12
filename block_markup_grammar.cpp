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
#include "template.hpp"
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

    void quickbook_grammar::impl::init_block_markup()
    {
        qi::rule<iterator, quickbook::title()>& title_phrase = store_.create();
        qi::rule<iterator, std::string()>& inside_paragraph = store_.create();
        qi::rule<iterator, std::string()>& phrase_attr = store_.create();
        qi::rule<iterator>& phrase_end = store_.create();
        qi::rule<iterator, boost::optional<raw_string>()>& element_id = store_.create();

        // Sections

        qi::rule<iterator, quickbook::begin_section()>& begin_section = store_.create();
        qi::rule<iterator, quickbook::end_section()>& end_section = store_.create();
        block_keyword_rules.add("section", begin_section[actions.process]);
        block_keyword_rules.add("endsect", end_section[actions.process]);

        begin_section =
                space
            >>  element_id                          [member_assign(&quickbook::begin_section::id)]
            >>  title_phrase                        [member_assign(&quickbook::begin_section::content)]
            ;

        end_section =
                space
            >>  position                            [member_assign(&quickbook::end_section::position)]
            ;

        // Headings

        qi::rule<iterator, quickbook::heading(int)>& heading = store_.create();

        block_keyword_rules.add
            ("h1", heading(1) [actions.process])
            ("h2", heading(2) [actions.process])
            ("h3", heading(3) [actions.process])
            ("h4", heading(4) [actions.process])
            ("h5", heading(5) [actions.process])
            ("h6", heading(6) [actions.process])
            ("heading", heading(-1) [actions.process]);

        heading =
                qi::attr(qi::_r1)                   [member_assign(&quickbook::heading::level)]
            >>  space
            >>  title_phrase                        [member_assign(&quickbook::heading::content)]
                ;
        
        // Paragraph Blocks

        qi::rule<iterator, quickbook::formatted(formatted_type)>& paragraph_block = store_.create();

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
                qi::attr(qi::_r1)                   [member_assign(&quickbook::formatted::type)]
            >>  space
            >>  inside_paragraph                    [member_assign(&quickbook::formatted::content)]
            ;

        // Preformatted

        qi::rule<iterator, quickbook::formatted()>& preformatted = store_.create();

        block_keyword_rules.add("pre", preformatted [actions.process]);
        
        preformatted =
                space                           [ph::ref(no_eols) = false]
                                                [member_assign(&quickbook::formatted::type, "preformatted")]
            >>  -eol
            >>  phrase_attr                     [member_assign(&quickbook::formatted::content)]
            >>  qi::eps                         [ph::ref(no_eols) = true]
            ;

        // Define Macro

        qi::rule<iterator, quickbook::def_macro()>& def_macro = store_.create();

        block_keyword_rules.add("def", def_macro[actions.process]);
        
        def_macro =
                space
            >>  macro_identifier                [member_assign(&quickbook::def_macro::macro_identifier)]
            >>  blank
            >>  phrase_attr                     [member_assign(&quickbook::def_macro::content)]
            ;

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

        // Block contents

        // Used when the title is used both to generate the title text and
        // possibly to generate an id (based on the raw source).
        title_phrase =
            qi::raw[
                phrase_attr                         [member_assign(&quickbook::title::content)]
            ]                                       [member_assign(&quickbook::title::raw)]
            ;

        qi::rule<iterator, quickbook::formatted()>& inside_paragraph2 = store_.create();

        inside_paragraph =
                qi::eps                             [actions.phrase_push]
            >>  inside_paragraph2                   [actions.process]
            >>  *(  +eol
                >>  inside_paragraph2               [actions.process]
                )
            >>  qi::eps                             [actions.phrase_pop]
            ;

        inside_paragraph2 =
                phrase_attr                         [member_assign(&quickbook::formatted::content)]
                                                    [member_assign(&quickbook::formatted::type, "paragraph")]
            ;

        phrase_attr =
                qi::eps                         [actions.phrase_push]        
            >> *(   common
                |   comment
                |   (qi::char_ - phrase_end)    [actions.process]
                )
            >>  qi::eps                         [actions.phrase_pop]
            ;

        // Make sure that we don't go past a single block, except when
        // preformatted.
        phrase_end =
            ']' | qi::eps(ph::ref(no_eols)) >> eol >> *qi::blank >> qi::eol
            ;

        // Identifiers

        qi::rule<iterator, raw_string()>& element_id_part = store_.create();

        element_id =
            (   ':'
            >>  -(qi::eps(qbk_since(105u)) >> space)
            >>  (
                    element_id_part
                |   qi::omit[
                        qi::raw[qi::eps]        [actions.element_id_warning]
                    ]
                )
            )
            | qi::eps
            ;

        element_id_part = qi::raw[+(qi::alnum | qi::char_('_'))]
                                                [qi::_val = qi::_1];
    }
}