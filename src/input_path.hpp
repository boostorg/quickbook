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
#include <stdexcept>

namespace quickbook
{
    namespace fs = boost::filesystem;

    namespace detail
    {
        struct conversion_error : std::runtime_error
        {
            conversion_error(char const* m) : std::runtime_error(m) {}
        };

        // 'generic':   Paths in quickbook source and the generated boostbook.
        //              Always UTF-8.
        // 'native':    Paths (or other parameters) from the command line and
        //              possibly other sources in the future. Wide strings on
        //              normal windows, UTF-8 for cygwin and other platforms
        //              (hopefully).
        // 'path':      Stored as a boost::filesystem::path. Since
        //              Boost.Filesystem doesn't support cygwin, this
        //              is always wide on windows. UTF-8 on other
        //              platforms (again, hopefully).
    
#if defined(_WIN32) && !(defined(__cygwin__) || defined(__CYGWIN__))
#define QUICKBOOK_WIDE_NATIVE 1
        typedef std::wstring native_string;
#else
#define QUICKBOOK_WIDE_NATIVE 0
        typedef std::string native_string;
#endif

        std::string native_to_utf8(native_string const&);
        fs::path native_to_path(native_string const& x);
    
        std::string path_to_generic(fs::path const&);
        fs::path generic_to_path(std::string const&);
    }
}

#endif
