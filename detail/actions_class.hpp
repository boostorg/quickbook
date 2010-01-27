/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_ACTIONS_CLASS_HPP)
#define BOOST_SPIRIT_ACTIONS_CLASS_HPP

#include "./actions.hpp"
#include <boost/tuple/tuple.hpp>

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace fs = boost::filesystem;

    struct actions
    {
        actions(char const* filein_, fs::path const& outdir, string_stream& out_);

    ///////////////////////////////////////////////////////////////////////////
    // State
    ///////////////////////////////////////////////////////////////////////////

        typedef std::vector<std::string> string_list;
        typedef std::vector<std::pair<std::string, std::string> > author_list;
        typedef std::vector<std::pair<string_list, std::string> > copyright_list;
        typedef std::pair<char, int> mark_type;
        static int const max_template_depth = 100;

    // header info
        std::string             doc_type;
        std::string             doc_title;
        std::string             doc_version;
        std::string             doc_id;
        std::string             doc_dirname;
        copyright_list          doc_copyrights;
        std::string             doc_purpose;
        std::string             doc_category;
        author_list             doc_authors;
        std::string             doc_license;
        std::string             doc_last_revision;
        std::string             include_doc_id;
        std::string             doc_license_1_1;
        std::string             doc_purpose_1_1;

    // main output stream
        collector               out;

    // auxilliary streams
        collector               phrase;
        collector               temp_para;
        collector               list_buffer;

    // state
        fs::path                filename;
        fs::path                outdir;
        string_symbols          macro;
        int                     section_level;
        std::string             section_id;
        std::string             qualified_section_id;
        std::string             source_mode;

        typedef boost::tuple<
            fs::path
          , fs::path
          , string_symbols
          , int
          , std::string
          , std::string
          , std::string>
        state_tuple;

        std::stack<state_tuple> state_stack;

    // temporary or global state
        unsigned                table_span;
        std::string             table_header;
        std::string             macro_id;
        std::stack<mark_type>   list_marks;
        int                     list_indent;
        string_list             template_info;
        int                     template_depth;
        bool                    template_escape;
        template_stack          templates;
        int                     error_count;

    // push/pop the states and the streams
        void push();
        void pop();

    ///////////////////////////////////////////////////////////////////////////
    // actions
    ///////////////////////////////////////////////////////////////////////////
        process_action          process;
        phrase_push_action      phrase_push;
        phrase_pop_action       phrase_pop;
        error_action            error;
        phrase_to_string_action extract_doc_license;
        phrase_to_string_action extract_doc_purpose;

        syntax_highlight        syntax_p;
        code_action             code;
        code_action             code_block;
        inline_code_action      inline_code;
        phrase_action           paragraph;
        phrase_action           inside_paragraph;
        generic_header_action   h;
        header_action           h1, h2, h3, h4, h5, h6;
        markup_action           hr;
        phrase_action           blurb, blockquote, preformatted;
        phrase_action           warning, caution, important, note, tip;
        plain_char_action       plain_char;
        raw_char_action         raw_char;

        list_action             list;
        list_format_action      list_format;
        phrase_action           list_item;

        simple_phrase_action    simple_bold;
        simple_phrase_action    simple_italic;
        simple_phrase_action    simple_underline;
        simple_phrase_action    simple_teletype;
        simple_phrase_action    simple_strikethrough;

        variablelist_action     variablelist;
        markup_action           start_varlistentry;
        markup_action           end_varlistentry;
        markup_action           start_varlistterm;
        markup_action           end_varlistterm;
        start_varlistitem_action start_varlistitem;
        end_varlistitem_action  end_varlistitem;

        macro_identifier_action macro_identifier;
        macro_definition_action macro_definition;
        do_macro_action         do_macro;
        template_body_action    template_body;
        char const*             url_pre;
        char const*             url_post;
        char const*             link_pre;
        char const*             link_post;
        table_action            table;
        start_row_action        start_row;
        markup_action           end_row;
        start_col_action        start_cell;
        end_col_action          end_cell;

        begin_section_action    begin_section;
        end_section_action      end_section;
        element_id_warning_action element_id_warning;
        xinclude_action         xinclude;
        include_action          include;
        import_action           import;

        markup_action           escape_pre;
        markup_action           escape_post;
    };
}

#endif // BOOST_SPIRIT_ACTIONS_CLASS_HPP

