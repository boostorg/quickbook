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
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include "block_grammar.hpp"
#include "template.hpp"
#include "actions.hpp"
#include "code.hpp"
#include "misc_rules.hpp"

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::title,
    (quickbook::raw_source, raw)
    (std::string, content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::begin_section,
    (boost::optional<std::string>, id)
    (quickbook::title, content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::end_section,
    (quickbook::file_position, position)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::heading,
    (int, level)
    (quickbook::title, content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::def_macro,
    (std::string, macro_identifier)
    (std::string, content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::define_template,
    (std::string, id)
    (std::vector<std::string>, params)
    (quickbook::template_value, body)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::template_value,
    (quickbook::file_position, position)
    (std::string, content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::variablelist,
    (std::string, title)
    (std::vector<quickbook::varlistentry>, entries)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::table,
    (boost::optional<std::string>, id)
    (std::string, title)
    (std::vector<quickbook::table_row>, rows)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::xinclude,
    (std::string, path)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::import,
    (std::string, path)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::include,
    (boost::optional<std::string>, id)
    (std::string, path)
)

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;

    void block_grammar::rules::init_block_markup()
    {
        qi::rule<iterator, quickbook::begin_section()>& begin_section = store_.create();
        qi::rule<iterator, quickbook::end_section()>& end_section = store_.create();
        qi::rule<iterator, quickbook::heading()>& heading = store_.create();
        qi::rule<iterator, quickbook::formatted()>& paragraph_block = store_.create();
        qi::rule<iterator, quickbook::formatted()>& blockquote = store_.create();
        qi::rule<iterator, quickbook::formatted()>& preformatted = store_.create();
        qi::rule<iterator, quickbook::def_macro()>& def_macro = store_.create();
        qi::rule<iterator, quickbook::table()>& table = store_.create();
        qi::rule<iterator, quickbook::variablelist()>& variablelist = store_.create();
        qi::rule<iterator, quickbook::xinclude()>& xinclude = store_.create();
        qi::rule<iterator, quickbook::include()>& include = store_.create();
        qi::rule<iterator, quickbook::import()>& import = store_.create();
        qi::rule<iterator, quickbook::define_template()>& define_template = store_.create();
        qi::rule<iterator, quickbook::title()>& title_phrase = store_.create();
        qi::rule<iterator, std::string()>& inside_paragraph = store_.create();
        qi::rule<iterator, std::string()>& phrase_attr = store_.create();
        qi::rule<iterator>& phrase_end = store_.create();
        qi::rule<iterator, boost::optional<std::string>()>& element_id = store_.create();
        qi::rule<iterator>& error = store_.create();

        block_markup =
                '[' >> space
            >>  (   begin_section
                |   end_section
                |   heading
                |   paragraph_block
                |   blockquote
                |   preformatted
                |   def_macro
                |   table
                |   variablelist
                |   xinclude
                |   include
                |   import
                |   define_template
                )                               [actions.process]
            >>  (   (space >> ']' >> +eol)
                |   error
                )
            ;
        
        begin_section =
                "section"
            >>  hard_space
            >>  element_id
            >>  title_phrase
            ;

        end_section =
                position
            >>  "endsect"
            >>  qi::attr(nothing())
            ;

        qi::symbols<char, int>& heading_symbol = store_.create();
        heading = heading_symbol >> hard_space >> title_phrase;

        heading_symbol.add
            ("h1", 1)
            ("h2", 2)
            ("h3", 3)
            ("h4", 4)
            ("h5", 5)
            ("h6", 6)
            ("heading", -1);

        qi::symbols<char, quickbook::formatted_type>& paragraph_blocks = store_.create();

        paragraph_block =
            paragraph_blocks >> hard_space >> inside_paragraph
            ;

        paragraph_blocks.add
            ("blurb", formatted_type("blurb"))
            ("warning", formatted_type("warning"))
            ("caution", formatted_type("caution"))
            ("important", formatted_type("important"))
            ("note", formatted_type("note"))
            ("tip", formatted_type("tip"))
            ;

        blockquote =
                ':'
            >>  blank
            >>  qi::attr(formatted_type("blockquote"))
            >>  inside_paragraph
            ;

        preformatted %=
                "pre"
            >>  hard_space                      [ph::ref(no_eols) = false]
            >>  -eol
            >>  qi::attr(formatted_type("preformatted"))
            >>  phrase_attr
            >>  qi::eps                         [ph::ref(no_eols) = true]
            ;

        def_macro =
                "def"
            >>  hard_space
            >>  macro_identifier
            >>  blank
            >>  phrase_attr
            ;

        qi::rule<iterator, quickbook::table_row()>& table_row = store_.create();
        qi::rule<iterator, quickbook::table_cell()>& table_cell = store_.create();
        qi::rule<iterator, quickbook::formatted()>& table_cell_body = store_.create();

        table =
                "table"
            >>  (&(*qi::blank >> qi::eol) | hard_space)
            >>  ((qi::eps(qbk_since(105u)) >> element_id) | qi::eps)
            >>  (&(*qi::blank >> qi::eol) | space)
            >>  *(qi::char_ - eol)
            >>  +eol
            >>  *table_row
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
                qi::attr(formatted_type("cell"))
            >>  inside_paragraph
            ;

        qi::rule<iterator, quickbook::varlistentry()>& varlistentry = store_.create();
        qi::rule<iterator, quickbook::formatted()>& varlistterm = store_.create();
        qi::rule<iterator, quickbook::formatted()>& varlistterm_body = store_.create();
        qi::rule<iterator, quickbook::formatted()>& varlistitem = store_.create();
        qi::rule<iterator, quickbook::formatted()>& varlistitem_body = store_.create();

        variablelist =
                "variablelist"
            >>  (&(*qi::blank >> qi::eol) | hard_space)
            >>  *(qi::char_ - eol)
            >>  +eol
            >>  *varlistentry
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
                qi::attr(formatted_type("varlistterm"))
            >>  phrase_attr
            ;

        varlistitem =
                space
            >>  '['
            >>  (   varlistitem_body >> ']' >> space
                |   error >> qi::attr(quickbook::formatted())
                )
            ;

        varlistitem_body =
                qi::attr(formatted_type("varlistitem"))
            >>  inside_paragraph
            ;

        // TODO: Why do these use phrase_end? It doesn't make any sense.
        xinclude =
                "xinclude"
            >>  hard_space
            >>  *(qi::char_ - phrase_end)
            >>  qi::attr(nothing())
            ;

        include =
                "include"
            >>  hard_space
            >>  -(
                    ':'
                >>  *((qi::alnum | '_') - qi::space)
                >>  space
                )
            >>  *(qi::char_ - phrase_end)
            ;

        import =
                "import"
            >>  hard_space
            >>  *(qi::char_ - phrase_end)
            >>  qi::attr(nothing())
            ;

        qi::rule<iterator, quickbook::template_value()>& template_body = store_.create();
        qi::rule<iterator>& template_body_recurse = store_.create();
        qi::rule<iterator, std::string()>& template_id = store_.create();

        define_template =
                "template"
            >>  hard_space
            >>  template_id
            >>  -(
                    space
                >>  '['
                >>  *(space >> template_id)
                >>  space
                >>  ']'
                )
            >>  template_body
            ;

        template_body =
                position
            >>  qi::raw[template_body_recurse]
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
                phrase_attr                     [ph::at_c<1>(qi::_val) = qi::_1]
            ]                                   [ph::at_c<0>(qi::_val) = qi::_1]
            ;

        qi::rule<iterator, quickbook::formatted()>& inside_paragraph2 = store_.create();

        inside_paragraph =
                qi::eps                             [actions.phrase_push]
            >>  inside_paragraph2                   [actions.process]
            >>  *(  eol
                >>  eol
                >>  inside_paragraph2               [actions.process]
                )
            >>  qi::eps                             [actions.phrase_pop]
            ;

        inside_paragraph2 =
                qi::attr(formatted_type("paragraph"))
            >>  phrase_attr;

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
            ']' | qi::eps(ph::ref(no_eols)) >> eol >> eol
            ;

        // Identifiers

        qi::rule<iterator, std::string()>& element_id_part = store_.create();

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

        element_id_part = +(qi::alnum | qi::char_('_'));

        error =
            qi::raw[qi::eps] [actions.error];
    }
}