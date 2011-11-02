/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include "files.hpp"
#include <boost/filesystem/v3/fstream.hpp>
#include <boost/unordered_map.hpp>
#include <fstream>

namespace quickbook
{
    // Read the first few bytes in a file to see it starts with a byte order
    // mark. If it doesn't, then write the characters we've already read in.
    // Although, given how UTF-8 works, if we've read anything in, the files
    // probably broken.

    template <typename InputIterator, typename OutputIterator>
    bool check_bom(InputIterator& begin, InputIterator end,
            OutputIterator out, char const* chars, int length)
    {
        char const* ptr = chars;

        while(begin != end && *begin == *ptr) {
            ++begin;
            ++ptr;
            --length;
            if(length == 0) return true;
        }

        // Failed to match, so write the skipped characters to storage:
        while(chars != ptr) *out++ = *chars++;

        return false;
    }

    template <typename InputIterator, typename OutputIterator>
    std::string read_bom(InputIterator& begin, InputIterator end,
            OutputIterator out)
    {
        if(begin == end) return "";

        const char* utf8 = "\xef\xbb\xbf" ;
        const char* utf32be = "\0\0\xfe\xff";
        const char* utf32le = "\xff\xfe\0\0";

        unsigned char c = *begin;
        switch(c)
        {
        case 0xEF: { // UTF-8
            return check_bom(begin, end, out, utf8, 3) ? "UTF-8" : "";
        }
        case 0xFF: // UTF-16/UTF-32 little endian
            return !check_bom(begin, end, out, utf32le, 2) ? "" :
                check_bom(begin, end, out, utf32le + 2, 2) ? "UTF-32" : "UTF-16";
        case 0: // UTF-32 big endian
            return check_bom(begin, end, out, utf32be, 4) ? "UTF-32" : "";
        case 0xFE: // UTF-16 big endian
            return check_bom(begin, end, out, utf32be + 2, 2) ? "UTF-16" : "";
        default:
            return "";
        }
    }

    // Copy a string, converting mac and windows style newlines to unix
    // newlines.

    template <typename InputIterator, typename OutputIterator>
    void normalize(InputIterator begin, InputIterator end,
            OutputIterator out)
    {
        std::string encoding = read_bom(begin, end, out);

        if(encoding != "UTF-8" && encoding != "")
        throw load_error(encoding +
            " is not supported. Please use UTF-8.");

        while(begin != end) {
            if(*begin == '\r') {
                *out++ = '\n';
                ++begin;
                if(begin != end && *begin == '\n') ++begin;
            }
            else {
                *out++ = *begin++;
            }
        }
    }

    namespace
    {
        boost::unordered_map<fs::path, file> files;
    }

    file const* load(fs::path const& filename)
    {
        boost::unordered_map<fs::path, file>::iterator pos;
        bool inserted;

        boost::tie(pos, inserted) = files.emplace(filename, file());

        if (inserted)
        {
            pos->second.path = filename;

            fs::ifstream in(filename, std::ios_base::in);

            if (!in)
                throw load_error("Could not open input file.");

            // Turn off white space skipping on the stream
            in.unsetf(std::ios::skipws);

            normalize(
                std::istream_iterator<char>(in),
                std::istream_iterator<char>(),
                std::back_inserter(pos->second.source));
        }

        return &pos->second;
    }
}
