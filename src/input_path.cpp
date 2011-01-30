/*=============================================================================
    Copyright (c) 2009 Daniel James

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/program_options.hpp>
#include <iostream>
#include "input_path.hpp"
#include "utils.hpp"

#if defined(_WIN32)
#include <boost/scoped_ptr.hpp>
#include <windows.h>
#endif

#if (defined(__cygwin__) || defined(__CYGWIN__))
#include <boost/scoped_array.hpp>
#include <boost/program_options/errors.hpp>
#include <sys/cygwin.h>
#endif

namespace quickbook {
    extern bool ms_errors;
}

namespace quickbook {
namespace detail {
#if defined(_WIN32)
    namespace {
        std::string to_utf8(std::wstring const& x)
        {
            int buffer_count = WideCharToMultiByte(CP_UTF8, 0, x.c_str(), -1, 0, 0, 0, 0); 
        
            if (!buffer_count)
                throw conversion_error("Error converting wide string to utf-8.");
    
            boost::scoped_ptr<char> buffer(new char[buffer_count]);
    
            if (!WideCharToMultiByte(CP_UTF8, 0, x.c_str(), -1, buffer.get(), buffer_count, 0, 0))
                throw conversion_error("Error converting wide string to utf-8.");
            
            return std::string(buffer.get());
        }

        std::wstring from_utf8(std::string const& x)
        {
            int buffer_count = MultiByteToWideChar(CP_UTF8, 0, x.c_str(), -1, 0, 0); 
        
            if (!buffer_count)
                throw conversion_error("Error converting utf-8 to wide string.");
    
            boost::scoped_ptr<wchar_t> buffer(new wchar_t[buffer_count]);
    
            if (!MultiByteToWideChar(CP_UTF8, 0, x.c_str(), -1, buffer.get(), buffer_count))
                throw conversion_error("Error converting utf-8 to wide string.");
            
            return native_string(buffer.get());
        }
    }
#endif

    std::string native_to_utf8(native_string const& x)
    {
#if QUICKBOOK_WIDE_NATIVE
        return to_utf8(x);
#else
        return x;
#endif
    }

    fs::path generic_to_path(std::string const& x)
    {
#if defined(_WIN32)
        return fs::path(from_utf8(x));
#else
        return fs::path(x);
#endif
    }

    std::string path_to_generic(fs::path const& x)
    {
#if defined(_WIN32)
        return to_utf8(x.generic_wstring());
#else
        return x.generic_string();
#endif
    }

    fs::path native_to_path(native_string const& path)
    {
#if !(defined(__cygwin__) || defined(__CYGWIN__))
        return fs::path(path);
#else
        cygwin_conv_path_t flags = CCP_POSIX_TO_WIN_W | CCP_RELATIVE;

        ssize_t size = cygwin_conv_path(flags, path.c_str(), NULL, 0);
        
        if (size < 0)
            throw conversion_error("Error converting cygwin path to windows.");

        boost::scoped_array<char> result(new char[size]);

        if(cygwin_conv_path(flags, path.c_str(), result.get(), size))
            throw conversion_error("Error converting cygwin path to windows.");

        return fs::path(result.get());
#endif
    }

    std::ostream& outerr()
    {
        return std::clog << "Error: ";
    }

    std::ostream& outerr(fs::path const& file, int line)
    {
        if (line >= 0)
        {
            if (ms_errors)
                return std::clog << file.string() << "(" << line << "): error: ";
            else
                return std::clog << file.string() << ":" << line << ": error: ";
        }
        else
        {
            return std::clog << file.string() << ": error: ";
        }
    }

    std::ostream& outwarn(fs::path const& file, int line)
    {
        if (line >= 0)
        {
            if (ms_errors)
                return std::clog << file.string() << "(" << line << "): warning: ";
            else
                return std::clog << file.string() << ":" << line << ": warning: ";
        }
        else
        {
            return std::clog << file.string() << ": warning: ";
        }
    }
}}
