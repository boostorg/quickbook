/*=============================================================================
    Copyright (c) 2002 2004  2006Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_attr_cast.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include "grammars.hpp"
#include "block.hpp"
#include "quickbook.hpp"
#include "utils.hpp"
#include "actions_class.hpp"
#include "parse_utils.hpp"
#include "markups.hpp"
#include "code.hpp"

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::paragraph,
    (std::string, content)
    (char const*, dummy)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::list_item,
    (quickbook::file_position, position)
    (std::string, indent)
    (char, mark)
    (std::string, content)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::title,
    (std::string, raw_markup)
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
    (char const*, dummy)
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
    (quickbook::file_position, position)
    (std::string, body)
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
    (char const*, dummy)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::import,
    (std::string, path)
    (char const*, dummy)
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

    struct block_grammar::rules
    {
        rules(quickbook::actions& actions_);

        quickbook::actions& actions;
        bool no_eols;
        phrase_grammar common;
        
        qi::rule<iterator> start_, blocks, block_markup;
        qi::rule<iterator, quickbook::begin_section()> begin_section;
        qi::rule<iterator, quickbook::end_section()> end_section;
        qi::rule<iterator, quickbook::heading()> heading;
        qi::symbols<char, int> heading_symbol;
        qi::rule<iterator, quickbook::formatted()> paragraph_block, blockquote, preformatted;
        qi::symbols<char, quickbook::markup> paragraph_blocks;
        qi::rule<iterator, quickbook::def_macro()> def_macro;
        qi::rule<iterator, quickbook::table()> table;
        qi::rule<iterator, quickbook::table_row()> table_row;
        qi::rule<iterator, quickbook::table_cell()> table_cell;
        qi::rule<iterator, quickbook::formatted()> table_cell_body;        
        qi::rule<iterator, quickbook::variablelist()> variablelist;
        qi::rule<iterator, quickbook::varlistentry()> varlistentry;
        qi::rule<iterator, quickbook::formatted()>
            varlistterm, varlistterm_body,
            varlistitem, varlistitem_body;
        qi::rule<iterator, quickbook::xinclude()> xinclude;
        qi::rule<iterator, quickbook::include()> include;
        qi::rule<iterator, quickbook::import()> import;
        qi::rule<iterator, quickbook::define_template()> define_template;
        qi::rule<iterator, std::string()> template_body;
        qi::rule<iterator> template_body_recurse;
        qi::rule<iterator, quickbook::code()> code;
        qi::rule<iterator> code_line;
        qi::rule<iterator, quickbook::list()> list;
        qi::rule<iterator, quickbook::list_item()> list_item;
        qi::rule<iterator, std::string()> list_item_content;
                qi::rule<iterator, quickbook::hr()> hr;
        qi::rule<iterator, quickbook::paragraph()> paragraph;
        qi::rule<iterator, std::string()> paragraph_content;
        qi::rule<iterator> paragraph_end;
        qi::symbols<> paragraph_end_markups;
        qi::rule<iterator, quickbook::title()> title_phrase;
        qi::rule<iterator, std::string()> inside_paragraph;
        qi::rule<iterator, quickbook::formatted()> inside_paragraph2;
        qi::rule<iterator, std::string()> phrase_attr;
        qi::rule<iterator> phrase_end;
        qi::rule<iterator> comment, dummy_block;
        qi::rule<iterator, boost::optional<std::string>()> element_id_1_5;
        qi::rule<iterator, boost::optional<std::string>()> element_id;
        qi::rule<iterator, std::string()> element_id_part;
        qi::rule<iterator, std::string()> macro_identifier;
        qi::rule<iterator, std::string()> template_id;
        qi::rule<iterator, std::string()> identifier;
        qi::rule<iterator, std::string()> punctuation_identifier;
        qi::rule<iterator> hard_space, space, blank, eol;
        qi::rule<iterator, file_position()> position;
        qi::rule<iterator> error;
    };

    block_grammar::block_grammar(quickbook::actions& actions_)
        : block_grammar::base_type(start, "block")
        , rules_pimpl(new rules(actions_))
        , start(rules_pimpl->start_) {}

    block_grammar::~block_grammar() {}

    block_grammar::rules::rules(quickbook::actions& actions_)
        : actions(actions_), no_eols(true), common(actions, no_eols)
    {
        start_ =
            blocks >> blank
            ;

        blocks =
           +(   block_markup
            |   code                            [actions.process]
            |   list                            [actions.process]
            |   hr                              [actions.process]
            |   comment >> *eol
            |   paragraph                       [actions.process]
            |   eol
            )
            ;

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
            >>  qi::attr("dummy")
            ;

        heading = heading_symbol >> hard_space >> title_phrase;

        heading_symbol.add
            ("h1", 1)
            ("h2", 2)
            ("h3", 3)
            ("h4", 4)
            ("h5", 5)
            ("h6", 6)
            ("heading", -1);

        paragraph_block =
            paragraph_blocks >> hard_space >> inside_paragraph
            ;

        paragraph_blocks.add
            ("blurb", markup(blurb_pre, blurb_post))
            ("warning", markup(warning_pre, warning_post))
            ("caution", markup(caution_pre, caution_post))
            ("important", markup(important_pre, important_post))
            ("note", markup(note_pre, note_post))
            ("tip", markup(tip_pre, tip_post))
            ;

        blockquote =
                ':'
            >>  blank
            >>  qi::attr(markup(blockquote_pre, blockquote_post))
            >>  inside_paragraph
            ;

        preformatted %=
                "pre"
            >>  hard_space                      [ph::ref(no_eols) = false]
            >>  -eol
            >>  qi::attr(markup(preformatted_pre, preformatted_post))
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

        table =
                "table"
            >>  (&(*qi::blank >> qi::eol) | hard_space)
            >>  element_id_1_5
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
                qi::attr(markup(start_cell_, end_cell_))
            >>  inside_paragraph
            ;

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
                qi::attr(markup(start_varlistterm_, end_varlistterm_))
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
                qi::attr(markup(start_varlistitem_, end_varlistitem_))
            >>  inside_paragraph
            ;

        // TODO: Why do these use phrase_end? It doesn't make any sense.
        xinclude =
                "xinclude"
            >>  hard_space
            >>  *(qi::char_ - phrase_end)
            >>  qi::attr("dummy")
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
            >>  qi::attr("dummy")
            ;

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
            >>  position
            >>  template_body
            ;

        template_body =
            qi::raw[template_body_recurse]
            ;

        template_body_recurse =
                *(  ('[' >> template_body_recurse >> ']')
                |   (qi::char_ - ']')
                )
            >>  space
            >>  &qi::lit(']')
            ;

        // Blocks indicated by text layout (indentation, leading characters etc.)

        code =
                position
            >>  qi::raw[
                    code_line
                >>  *(*eol >> code_line)
                ]
            >>  +eol
            >>  qi::attr(true)
            ;

        code_line =
                qi::char_(" \t")
            >>  *(qi::char_ - eol)
            >>  eol
            ;

        list =
                &qi::char_("*#")
            >>  +list_item
            ;
        
        list_item =
                position
            >>  *qi::blank
            >>  qi::char_("*#")
            >>  qi::omit[*qi::blank]
            >>  list_item_content
            ;

        list_item_content =
            qi::eps[actions.phrase_push] >>
           *(   common
            |   (qi::char_ -
                    (   qi::eol >> *qi::blank >> &(qi::char_('*') | '#')
                    |   (eol >> eol)
                    )
                )                               [actions.plain_char]
            )
            >> +eol
            >> qi::eps[actions.phrase_pop]
            ;

        hr =
            qi::omit[
                "----"
            >>  *(qi::char_ - eol)
            >>  +eol
            ] >> qi::attr(quickbook::hr())
            ;

        paragraph = paragraph_content >> qi::attr("dummy");

        paragraph_content =
                qi::eps                         [actions.phrase_push]
            >> *(   common
                |   (qi::char_ -                // Make sure we don't go past
                        paragraph_end           // a single block.
                    )                           [actions.plain_char]
                )
            >>  qi::eps                         [actions.phrase_pop]
            >> (&qi::lit('[') | +eol)
            ;

        paragraph_end =
            '[' >> space >> paragraph_end_markups >> hard_space | eol >> eol
            ;

        paragraph_end_markups =
            "section", "endsect", "h1", "h2", "h3", "h4", "h5", "h6",
            "blurb", ":", "pre", "def", "table", "include", "xinclude",
            "variablelist", "import", "template", "warning", "caution",
            "important", "note", "tip", ":"
            ;

        // Block contents

        // Used when the title is used both to generate the title text and
        // possibly to generate an id (based on the raw source).
        title_phrase =
            qi::raw[
                phrase_attr                     [ph::at_c<1>(qi::_val) = qi::_1]
            ]                                   [ph::at_c<0>(qi::_val) = as_string(qi::_1)]
            ;

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
                qi::attr(markup(paragraph_pre, paragraph_post))
            >>  phrase_attr;

        phrase_attr =
                qi::eps                         [actions.phrase_push]        
            >> *(   common
                |   comment
                |   (qi::char_ - phrase_end)    [actions.plain_char]
                )
            >>  qi::eps                         [actions.phrase_pop]
            ;

        // Make sure that we don't go past a single block, except when
        // preformatted.
        phrase_end =
            ']' | qi::eps(ph::ref(no_eols)) >> eol >> eol
            ;

        comment =
            "[/" >> *(dummy_block | (qi::char_ - ']')) >> ']'
            ;

        dummy_block =
            '[' >> *(dummy_block | (qi::char_ - ']')) >> ']'
            ;

        // Identifiers

        element_id_1_5 = (qi::eps(qbk_since(105u)) >> element_id) | qi::eps;

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

        macro_identifier =
            +(qi::char_ - (qi::space | ']'))
            ;

        template_id =
            identifier | punctuation_identifier
            ;

        identifier =
            (qi::alpha | '_') >> *(qi::alnum | '_')
            ;
        
        punctuation_identifier =
            qi::repeat(1)[qi::punct - qi::char_("[]")]
            ;

        // Used after an identifier that must not be immediately
        // followed by an alpha-numeric character or underscore.
        hard_space =
            !(qi::alnum | '_') >> space
            ;

        space =
            *(qi::space | comment)
            ;

        blank =
            *(qi::blank | comment)
            ;

        eol = blank >> qi::eol
            ;

        position =
            qi::raw[qi::eps] [get_position];

        error =
            qi::raw[qi::eps] [actions.error];
    }
}
