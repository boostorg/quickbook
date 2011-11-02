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
#include <cassert>

namespace quickbook {

    namespace fs = boost::filesystem;

    struct load_error : std::runtime_error
    {
        explicit load_error(std::string const& arg)
            : std::runtime_error(arg) {}
    };

    struct file
    {
        fs::path const path;
        std::string const source;
    private:
        unsigned qbk_version;
    public:

        file(fs::path const& path, std::string const& source,
                unsigned qbk_version) :
            path(path), source(source), qbk_version(qbk_version)
        {}

        unsigned version() const {
            assert(qbk_version);
            return qbk_version;
        }

        void version(unsigned v) {
            // Check that either version hasn't been set, or it was
            // previously set to the same version (because the same
            // file has been loaded twice).
            assert(!qbk_version || qbk_version == v);
            qbk_version = v;
        }
    };

    // If version isn't supplied then it must be set later.
    file* load(fs::path const& filename,
        unsigned qbk_version = 0);
}

#endif // BOOST_QUICKBOOK_FILES_HPP
