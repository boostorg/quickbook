/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2005 Thomas Guest
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/filesystem/v3/operations.hpp>
#include "actions.hpp"
#include "state.hpp"
#include "quickbook.hpp"
#include "block.hpp"

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
        , block(out_)
        , phrase()
        , encoder(encoder)

    // state
        , filename(fs::absolute(fs::path(filein_)))
        , outdir(outdir_)
        , macro_change_depth(0)
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
            filename.native();

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
                macro_change_depth
              , section_level
              , min_section_level
              , section_id
              , qualified_section_id
              , source_mode
            )
        );

        phrase.push();
        block.push();
        templates.push();
    }
    
    // Pushing and popping the macro symbol table is pretty expensive, so
    // instead implement a sort of 'stack on write'. Call this whenever a
    // change is made to the macro table, and it'll stack the current macros
    // if necessary. Would probably be better to implement macros in a less
    // expensive manner.
    void state::copy_macros_for_write()
    {
        if(macro_change_depth != state_stack.size())
        {
            macro_stack.push(macro_symbols_wrap(macro));
            macro_change_depth = state_stack.size();
        }
    }

    void state::pop()
    {
        if(macro_change_depth == state_stack.size())
        {
            macro = macro_stack.top().get();
            macro_stack.pop();
        }

        boost::tie(
            macro_change_depth
          , section_level
          , min_section_level
          , section_id
          , qualified_section_id
          , source_mode
        ) = state_stack.top();
        state_stack.pop();

        phrase.pop();
        block.pop();
        templates.pop();
    }
    
    void state::paragraph_output()
    {
        std::string paragraph;
        phrase.swap(paragraph);

        // TODO: Use spirit to do this?

        std::string::const_iterator
            pos = paragraph.begin(),
            end = paragraph.end();

        while(pos != paragraph.end() && (
            *pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r'))
        {
            ++pos;
        }

        if(pos != end) {
            actions a(*this);
            
            quickbook::paragraph p;
            p.content = paragraph;
            
            a.process(p);
        }
    }
}
