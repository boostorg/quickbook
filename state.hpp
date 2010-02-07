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

#include <stack>
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem/path.hpp>
#include "fwd.hpp"
#include "collector.hpp"
#include "template.hpp"
#include "actions.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace fs = boost::filesystem;

    struct state
    {
        state(char const* filein_, fs::path const& outdir, string_stream& out_);

    ///////////////////////////////////////////////////////////////////////////
    // State
    ///////////////////////////////////////////////////////////////////////////

        static int const max_template_depth = 100;

        std::string             doc_id;
        std::string             doc_title;

    // main output stream
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
        int                     template_depth;
        template_stack          templates;
        int                     error_count;

    // push/pop the states and the streams
        void push();
        void pop();
    };    
}

#endif // BOOST_SPIRIT_ACTIONS_CLASS_HPP

