/*=============================================================================
    Copyright (c) 2015 Daniel James

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/detail/lightweight_test.hpp>
#include <boost/predef.h>
#include "path.hpp"

int main() {
    using boost::filesystem::path;
    using quickbook::file_path_to_url;

    BOOST_TEST_EQ("a/b", file_path_to_url(path("a/b")));
    BOOST_TEST_EQ("../a/b", file_path_to_url(path("../a/b")));
    BOOST_TEST_EQ("A%20B%2bC%2520", file_path_to_url(path("A B+C%20")));
    BOOST_TEST_EQ("file:///a/b", file_path_to_url(path("/a/b")));

    // TODO: Windows specific tests.
#if BOOST_OS_WINDOWS
    BOOST_TEST_EQ("file:///a", file_path_to_url(path("\\a")));
    BOOST_TEST_EQ("file:///c:/", file_path_to_url(path("c:\\")));
    BOOST_TEST_EQ("file:///c:/foo/bar", file_path_to_url(path("c:\\foo\\bar")));
#endif

    return boost::report_errors();
}
