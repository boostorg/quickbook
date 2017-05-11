
/*=============================================================================
    Copyright (c) 2017 Daniel James

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "utils.hpp"
#include <boost/detail/lightweight_test.hpp>

#include <iostream>

void linkify_test() {
    using quickbook::detail::linkify;

    BOOST_TEST(linkify("abc", "link") == "<link linkend=\"link\">abc</link>");
    BOOST_TEST(linkify("<link linkend=\"something\">abc</link>", "link") ==
        "<link linkend=\"something\">abc</link>");
    BOOST_TEST(linkify("abc <link linkend=\"something\">def</link>", "link") ==
        "abc <link linkend=\"something\">def</link>");
    BOOST_TEST(linkify("<link linkend=\"something\">abc</link> def", "link") ==
        "<link linkend=\"something\">abc</link> def");
}

int main() {
    linkify_test();
    return boost::report_errors();
}
