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

    // state
        fs::path                filename;
        fs::path                outdir;
        macro_symbols           macro;
        int                     section_level;
        std::string             section_id;
        std::string             qualified_section_id;
        std::string             source_mode;

        typedef boost::tuple<
            fs::path
          , fs::path
          , macro_symbols
          , int
          , std::string
          , std::string
          , std::string>
        state_tuple;

        std::stack<state_tuple> state_stack;

    // temporary or global state
        string_list             template_info;
        int                     template_depth;
        template_stack          templates;
        int                     error_count;

    // push/pop the states and the streams
        void push();
        void pop();

    ///////////////////////////////////////////////////////////////////////////
    // actions
    ///////////////////////////////////////////////////////////////////////////
        output_action           output;
        process_action          process;
        phrase_push_action      phrase_push;
        phrase_pop_action       phrase_pop;
        error_action            error;
        phrase_to_string_action extract_doc_license;
        phrase_to_string_action extract_doc_purpose;

        syntax_highlight        syntax_p;
        code_action             code;
        phrase_action           paragraph;
        plain_char_action       plain_char;
        raw_char_action         raw_char;

        template_body_action    template_body;
        char const*             url_pre;
        char const*             url_post;
        char const*             link_pre;
        char const*             link_post;

        element_id_warning_action element_id_warning;
        xinclude_action         xinclude;
        include_action          include;
        import_action           import;
    };
}

#endif // BOOST_SPIRIT_ACTIONS_CLASS_HPP

