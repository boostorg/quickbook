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
        fs::path                xinclude_base;
        template_stack          templates;
        int                     error_count;
        string_list             anchors;
        bool                    warned_about_breaks;
        bool                    conditional;
        id_manager&             ids;
        value_builder           callouts;           // callouts are global as
        int                     callout_depth;      // they don't nest.

    // state saved for files and templates.
        bool                    imported;
        string_symbols          macro;
        std::string             source_mode;
        value                   source_mode_next;
        file_ptr                current_file;
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

        void start_list(char mark);
        void end_list(char mark);
        void start_list_item();
        void end_list_item();

        void start_callouts();
        std::string add_callout(value);
        std::string end_callouts();
    };
}

#endif // BOOST_SPIRIT_ACTIONS_CLASS_HPP

