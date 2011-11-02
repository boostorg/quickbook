/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2011 Daniel James

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(BOOST_QUICKBOOK_FILES_HPP)
#define BOOST_QUICKBOOK_FILES_HPP

#include <string>
#include <boost/filesystem/v3/path.hpp>
#include <stdexcept>

namespace quickbook {

    namespace fs = boost::filesystem;

    struct load_error : std::runtime_error
    {
        explicit load_error(std::string const& arg)
            : std::runtime_error(arg) {}
    };

    struct file
    {
        fs::path path;
        std::string source;
    };

    file const* load(fs::path const& filename);
}

#endif // BOOST_QUICKBOOK_FILES_HPP
