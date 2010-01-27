/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2005 Thomas Guest
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include "./actions_class.hpp"
#include "./markups.hpp"

#if (defined(BOOST_MSVC) && (BOOST_MSVC <= 1310))
#pragma warning(disable:4355)
#endif

namespace quickbook
{
    actions::actions(char const* filein_, fs::path const& outdir_, string_stream& out_)
    // header info
        : doc_type()
        , doc_title()
        , doc_version()
        , doc_id()
        , doc_dirname()
        , doc_copyrights()
        , doc_purpose()
        , doc_category()
        , doc_authors()
        , doc_license()
        , doc_last_revision()
        , include_doc_id()
        , doc_license_1_1()
        , doc_purpose_1_1()

    // main output stream
        , out(out_)

    // auxilliary streams
        , phrase()

    // state
        , filename(fs::complete(fs::path(filein_, fs::native)))
        , outdir(outdir_)
        , macro()
        , section_level(0)
        , section_id()
        , qualified_section_id()
        , source_mode("c++")

    // temporary or global state
        , template_info()
        , template_depth(0)
        , templates()
        , error_count(0)

    // actions
        , output(*this)
        , process(*this)
        , phrase_push(phrase)
        , phrase_pop(phrase)
        , error(error_count)
        , extract_doc_license(doc_license, phrase)
        , extract_doc_purpose(doc_purpose, phrase)

        , syntax_p(source_mode, *this)
        , code(out, phrase, syntax_p)
        , plain_char(phrase)
        , raw_char(phrase)

        , xinclude(out, *this)
        , include(*this)
        , import(out, *this)
    {
        // turn off __FILENAME__ macro on debug mode = true
        std::string filename_str = debug_mode ?
            std::string("NO_FILENAME_MACRO_GENERATED_IN_DEBUG_MODE") :
            filename.native_file_string();

        // add the predefined macros
        macro.add
            ("__DATE__", quickbook::macro(quickbook_get_date))
            ("__TIME__", quickbook::macro(quickbook_get_time))
            ("__FILENAME__", quickbook::macro(filename_str))
        ;
    }

    void actions::push()
    {
        state_stack.push(
            boost::make_tuple(
                filename
              , outdir
              , macro
              , section_level
              , section_id
              , qualified_section_id
              , source_mode
            )
        );

        out.push();
        phrase.push();
        templates.push();
    }

    void actions::pop()
    {
        boost::tie(
            filename
          , outdir
          , macro
          , section_level
          , section_id
          , qualified_section_id
          , source_mode
        ) = state_stack.top();
        state_stack.pop();

        out.pop();
        phrase.pop();
        templates.pop();
    }
}
