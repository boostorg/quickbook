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

#include <boost/scoped_ptr.hpp>
#include "actions.hpp"
#include "parsers.hpp"
#include "values_parse.hpp"
#include "collector.hpp"

namespace quickbook
{
    namespace cl = boost::spirit::classic;
    namespace fs = boost::filesystem;

    struct actions
    {
        actions(fs::path const& filein_, fs::path const& xinclude_base, string_stream& out_,
                id_manager&);

    private:
        boost::scoped_ptr<quickbook_grammar> grammar_;

    public:
    ///////////////////////////////////////////////////////////////////////////
    // State
    ///////////////////////////////////////////////////////////////////////////

        typedef std::vector<std::string> string_list;

        static int const max_template_depth = 100;
        
    // global state
        std::string             doc_title_qbk;
        fs::path                xinclude_base;
        template_stack          templates;
        int                     error_count;
        string_list             anchors;
        bool                    no_eols;
        bool                    warned_about_breaks;
        bool                    conditional;
        id_manager&             ids;
        file*                   current_file_tmp;   // Temporary non-const pointer to new
                                                    // current_file so that the
                                                    // version can be written to.

    // state saved for files and templates.
        bool                    imported;
        std::string             doc_type;
        string_symbols          macro;
        std::string             source_mode;
        file const*             current_file;
        fs::path                filename_relative;  // for the __FILENAME__ macro.
                                                    // (relative to the original file
                                                    //  or include path).

    // state saved for templates.
        int                     template_depth;
        int                     min_section_level;

    // output state - scoped by templates and grammar
        collector               out;            // main output stream
        collector               phrase;         // phrase output stream
        value_parser            values;         // parsed values

        quickbook_grammar& grammar() const;

    ///////////////////////////////////////////////////////////////////////////
    // actions
    ///////////////////////////////////////////////////////////////////////////

        to_value_action         to_value;
        phrase_to_docinfo_action docinfo_value;
        
        scoped_parser<cond_phrase_push>
                                scoped_cond_phrase;
        scoped_parser<scoped_output_push>
                                scoped_output;
        scoped_parser<set_no_eols_scoped>
                                scoped_no_eols;

        element_action          element;
        error_action            error;

        code_action             code;
        code_action             code_block;
        inline_code_action      inline_code;
        paragraph_action        paragraph;
        phrase_end_action       phrase_end;
        space_action            space_char;
        plain_char_action       plain_char;
        escape_unicode_action   escape_unicode;

        simple_phrase_action    simple_markup;

        break_action            break_;
        do_macro_action         do_macro;

        element_id_warning_action element_id_warning;
    };
}

#endif // BOOST_SPIRIT_ACTIONS_CLASS_HPP

