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
        actions(fs::path const& filein_, fs::path const& xinclude_base, string_stream& out_);

    private:
        boost::scoped_ptr<quickbook_grammar> grammar_;

    public:
    ///////////////////////////////////////////////////////////////////////////
    // State
    ///////////////////////////////////////////////////////////////////////////

        typedef std::vector<std::string> string_list;

        static int const max_template_depth = 100;
        
        enum process_flags {
            process_none = 0,
            process_macros = 1,
            process_templates = 2,
            process_output = 4,
            process_normal = 7
        };

    // global state
        std::string             doc_title_qbk;
        fs::path                xinclude_base;
        template_stack          templates;
        int                     error_count;
        string_list             anchors;
        bool                    no_eols;
        bool                    warned_about_breaks;
        int                     context;

    // state saved for files and templates.
        std::string             doc_type;
        process_flags           process_state;
        string_symbols          macro;
        std::string             source_mode;
        std::string             doc_id;
        fs::path                filename;
        fs::path                filename_relative;  // for the __FILENAME__ macro.
                                                    // (relative to the original file
                                                    //  or include path).

    // state saved for templates.
        int                     template_depth;
        int                     section_level;
        int                     min_section_level;
        std::string             section_id;
        std::string             qualified_section_id;

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
        scoped_parser<scoped_context_impl>
                                scoped_context;
        scoped_parser<activate_processing_impl>
                                scoped_activate_processing;

        element_action          element;
        error_action            error;

        code_action             code;
        code_action             code_block;
        inline_code_action      inline_code;
        paragraph_action        paragraph;
        space                   space_char;
        plain_char_action       plain_char;
        raw_char_action         raw_char;
        escape_unicode_action   escape_unicode;

        simple_phrase_action    simple_markup;

        break_action            break_;
        do_macro_action         do_macro;

        element_id_warning_action element_id_warning;
    };

    // State savers

    struct file_state
    {
        enum scope_flags {
            scope_none = 0,
            scope_macros = 1,
            scope_templates = 2,
            scope_all = 3
        };
    
        explicit file_state(actions&, scope_flags);
        ~file_state();
        
        quickbook::actions& a;
        scope_flags scope;
        unsigned qbk_version;
        std::string doc_type;
        std::string doc_id;
        fs::path filename;
        fs::path filename_relative;
        std::string source_mode;
        actions::process_flags process_state;
        string_symbols macro;
    private:
        file_state(file_state const&);
        file_state& operator=(file_state const&);
    };

    struct template_state : file_state
    {
        explicit template_state(actions&);
        ~template_state();

        int template_depth;
        int section_level;
        int min_section_level;
        std::string section_id;
        std::string qualified_section_id;
    };
}

#endif // BOOST_SPIRIT_ACTIONS_CLASS_HPP

