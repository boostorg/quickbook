/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2005 Thomas Guest
    Copyright (c) 2013, 2017 Daniel James

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "path.hpp"
#include "glob.hpp"
#include "include_paths.hpp"
#include "state.hpp"
#include "utils.hpp"
#include <boost/foreach.hpp>
#include <boost/range/algorithm/replace.hpp>
#include <boost/filesystem/operations.hpp>
#include <cassert>

namespace quickbook
{
    // Convert the path to its canonical representation if it exists.
    // Or something close if it doesn't.
    fs::path normalize_path(fs::path const& path)
    {
        fs::path p = fs::absolute(path); // The base of the path.
        fs::path extra; // The non-existant part of the path.
        int parent_count = 0; // Number of active '..' sections

        // Invariant: path is equivalent to: p / ('..' * parent_count) / extra
        // i.e. if parent_count == 0: p/extra
        // if parent_count == 2: p/../../extra

        // Pop path sections from path until we find an existing
        // path, adjusting for any dot path sections.
        while (!fs::exists(fs::status(p))) {
            fs::path name = p.filename();
            p = p.parent_path();
            if (name == "..") {
                ++parent_count;
            }
            else if (name == ".") {
            }
            else if (parent_count) {
                --parent_count;
            }
            else {
                extra = name / extra;
            }
        }

        // If there are any left over ".." sections, then add them
        // on to the end of the real path, and trust Boost.Filesystem
        // to sort them out.
        while (parent_count) {
            p = p / "..";
            --parent_count;
        }

        // Cannoicalize the existing part of the path, and add 'extra' back to
        // the end.
        return fs::canonical(p) / extra;
    }

    // Not a general purpose normalization function, just
    // from paths from the root directory. It strips the excess
    // ".." parts from a path like: "x/../../y", leaving "y".
    std::vector<fs::path> remove_dots_from_path(fs::path const& path)
    {
        assert(!path.has_root_directory() && !path.has_root_name());

        std::vector<fs::path> parts;

        BOOST_FOREACH(fs::path const& part, path)
        {
            if (part.empty() || part == ".") {
            }
            else if (part == "..") {
                if (!parts.empty()) parts.pop_back();
            }
            else {
                parts.push_back(part);
            }
        }

        return parts;
    }

    // The relative path from base to path
    fs::path path_difference(fs::path const& base, fs::path const& path)
    {
        fs::path
            absolute_base = fs::absolute(base),
            absolute_path = fs::absolute(path);

        // Remove '.', '..' and empty parts from the remaining path
        std::vector<fs::path>
            base_parts = remove_dots_from_path(absolute_base.relative_path()),
            path_parts = remove_dots_from_path(absolute_path.relative_path());

        std::vector<fs::path>::iterator
            base_it = base_parts.begin(),
            base_end = base_parts.end(),
            path_it = path_parts.begin(),
            path_end = path_parts.end();

        // Build up the two paths in these variables, checking for the first
        // difference.
        fs::path
            base_tmp = absolute_base.root_path(),
            path_tmp = absolute_path.root_path();

        fs::path result;

        // If they have different roots then there's no relative path so
        // just build an absolute path.
        if (!fs::equivalent(base_tmp, path_tmp))
        {
            result = path_tmp;
        }
        else
        {
            // Find the point at which the paths differ
            for(; base_it != base_end && path_it != path_end; ++base_it, ++path_it)
            {
                base_tmp /= *base_it;
                path_tmp /= *path_it;
                if (*base_it != *path_it) {
                    if (!fs::exists(base_tmp) || !fs::exists(path_tmp) ||
                        !fs::equivalent(base_tmp, path_tmp)) {
                        break;
                    }
                }
            }

            // Build a relative path to that point
            for(; base_it != base_end; ++base_it) result /= "..";
        }

        // Build the rest of our path
        for(; path_it != path_end; ++path_it) result /= *path_it;

        return result;
    }

    // Convert a Boost.Filesystem path to a URL.
    //
    // I'm really not sure about this, as the meaning of root_name and
    // root_directory are only clear for windows.
    //
    // Some info on file URLs at:
    // https://en.wikipedia.org/wiki/File_URI_scheme
    std::string file_path_to_url(fs::path const& x)
    {
        // TODO: Maybe some kind of error if this doesn't understand the path.
        // TODO: Might need a special cygwin implementation.
        // TODO: What if x.has_root_name() && !x.has_root_directory()?
        // TODO: What does Boost.Filesystem do for '//localhost/c:/path'?
        //       Is that event allowed by windows?

        if (x.has_root_name()) {
            std::string root_name = detail::path_to_generic(x.root_name());

            if (root_name.size() > 2 && root_name[0] == '/' && root_name[1] == '/') {
                // root_name is a network location.
                return "file:" + detail::escape_uri(detail::path_to_generic(x));
            }
            else if (root_name.size() >= 2 && root_name[root_name.size() - 1] == ':') {
                // root_name is a drive.
                return "file:///"
                    + detail::escape_uri(root_name.substr(0, root_name.size() - 1))
                    + ":/" // TODO: Or maybe "|/".
                    + detail::escape_uri(detail::path_to_generic(x.relative_path()));
            }
            else {
                // Not sure what root_name is.
                return detail::escape_uri(detail::path_to_generic(x));
            }
        }
        else if (x.has_root_directory()) {
            return "file://" + detail::escape_uri(detail::path_to_generic(x));
        }
        else {
            return detail::escape_uri(detail::path_to_generic(x));
        }
    }

    std::string dir_path_to_url(fs::path const& x)
    {
        return file_path_to_url(x) + "/";
    }

    namespace detail {
#if QUICKBOOK_WIDE_PATHS
        std::string command_line_to_utf8(command_line_string const& x)
        {
            return to_utf8(x);
        }
#else
        std::string command_line_to_utf8(command_line_string const& x)
        {
            return x;
        }
#endif

#if QUICKBOOK_WIDE_PATHS
        fs::path generic_to_path(quickbook::string_view x)
        {
            return fs::path(from_utf8(x));
        }

        std::string path_to_generic(fs::path const& x)
        {
            return to_utf8(x.generic_wstring());
        }
#else
        fs::path generic_to_path(quickbook::string_view x)
        {
            return fs::path(x.begin(), x.end());
        }

        std::string path_to_generic(fs::path const& x)
        {
            return x.generic_string();
        }
#endif

#if QUICKBOOK_CYGWIN_PATHS
        fs::path command_line_to_path(command_line_string const& path)
        {
            cygwin_conv_path_t flags = CCP_POSIX_TO_WIN_W | CCP_RELATIVE;

            ssize_t size = cygwin_conv_path(flags, path.c_str(), NULL, 0);

            if (size < 0)
                throw conversion_error("Error converting cygwin path to windows.");

            boost::scoped_array<char> result(new char[size]);
            void* ptr = result.get();

            if(cygwin_conv_path(flags, path.c_str(), ptr, size))
                throw conversion_error("Error converting cygwin path to windows.");

            return fs::path(static_cast<wchar_t*>(ptr));
        }

        stream_string path_to_stream(fs::path const& path)
        {
            cygwin_conv_path_t flags = CCP_WIN_W_TO_POSIX | CCP_RELATIVE;

            ssize_t size = cygwin_conv_path(flags, path.native().c_str(), NULL, 0);

            if (size < 0)
                throw conversion_error("Error converting windows path to cygwin.");

            boost::scoped_array<char> result(new char[size]);

            if(cygwin_conv_path(flags, path.native().c_str(), result.get(), size))
                throw conversion_error("Error converting windows path to cygwin.");

            return std::string(result.get());
        }
#else
        fs::path command_line_to_path(command_line_string const& path)
        {
            return fs::path(path);
        }

#if QUICKBOOK_WIDE_PATHS && !QUICKBOOK_WIDE_STREAMS
        stream_string path_to_stream(fs::path const& path)
        {
            return path.string();
        }
#else
        stream_string path_to_stream(fs::path const& path)
        {
            return path.native();
        }
#endif

#endif // QUICKBOOK_CYGWIN_PATHS
    }
}
