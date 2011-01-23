/*=============================================================================
    Copyright (c) 2009 Daniel James

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(BOOST_QUICKBOOK_DETAIL_INPUT_PATH_HPP)
#define BOOST_QUICKBOOK_DETAIL_INPUT_PATH_HPP

#include <boost/filesystem/v3/path.hpp>
#include <string>

namespace quickbook
{
    namespace fs = boost::filesystem;

    namespace detail
    {
        // Convert paths from the command line, or other native sources to
        // our internal path type. Mainly used to convert cygwin paths, but
        // might be useful elsewhere.
        fs::path native_to_path(fs::path::string_type const&);
    
        // Conversion of filenames to and from genertic utf-8 paths
        // (such as those used in quickbook and the generated boostbook)
        fs::path generic_to_path(std::string const&);
        std::string path_to_generic(fs::path const&);
    }
}

#endif
