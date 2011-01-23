/*=============================================================================
    Copyright (c) 2009 Daniel James

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(BOOST_QUICKBOOK_DETAIL_INPUT_PATH_HPP)
#define BOOST_QUICKBOOK_DETAIL_INPUT_PATH_HPP

#include <boost/filesystem/v3/path.hpp>
#include <vector>
#include <boost/any.hpp>
#include <string>

namespace quickbook
{
    namespace fs = boost::filesystem;

namespace detail
{
    // Use this class with Boost.Program Options to convert paths to the format
    // the Boost.Filesystem expects. This is needed on cygwin to convert cygwin
    // paths to windows paths (or vice versa, depending on how filesystem is set
    // up).
    //
    // Note that we don't want to convert paths in quickbook files, as they
    // should be platform independent, and aren't necessarily relative to the
    // current directory.

    class input_path {
        std::string path_;
    public:
        explicit input_path(char const* c) : path_(c) {}
        explicit input_path(std::string const& c) : path_(c) {}
        operator boost::filesystem::path() const { return boost::filesystem::path(path_); }

        friend void validate(boost::any&, const std::vector<std::string>&,
            input_path*, int);
    };
}}

#endif
