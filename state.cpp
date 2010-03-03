/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2005 Thomas Guest
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/filesystem/operations.hpp>
#include "actions.hpp"
#include "state.hpp"
#include "quickbook.hpp"

#if (defined(BOOST_MSVC) && (BOOST_MSVC <= 1310))
#pragma warning(disable:4355)
#endif

namespace quickbook
{
    namespace fs = boost::filesystem;

    state::state(char const* filein_, fs::path const& outdir_, string_stream& out_,
        encoder_ptr const& encoder)
    // header info
        : doc_id()
        , doc_title()

    // main output stream
        , phrase(out_)
        , encoder(encoder)

    // state
        , filename(fs::complete(fs::path(filein_, fs::native)))
        , outdir(outdir_)
        , macro()
        , section_level(0)
        , min_section_level(0)
        , section_id()
        , qualified_section_id()
        , source_mode("c++")

    // temporary or global state
        , template_depth(0)
        , templates()
        , error_count(0)
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

    void state::push()
    {
        state_stack.push(
            boost::make_tuple(
                macro
              , section_level
              , min_section_level
              , section_id
              , qualified_section_id
              , source_mode
            )
        );

        phrase.push();
        templates.push();
    }

    void state::pop()
    {
        boost::tie(
            macro
          , section_level
          , min_section_level
          , section_id
          , qualified_section_id
          , source_mode
        ) = state_stack.top();
        state_stack.pop();

        phrase.pop();
        templates.pop();
    }
}
