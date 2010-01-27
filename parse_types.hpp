/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_PARSE_TYPES_HPP)
#define BOOST_SPIRIT_QUICKBOOK_PARSE_TYPES_HPP

#include <string>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace quickbook
{
    // TODO: Add to a forward header somewhere.
    class actions;
    struct macro;
    typedef boost::spirit::classic::file_position file_position;

    struct markup {
        markup()
            : pre(""), post("") {}
        markup(char const* pre, char const* post)
            : pre(pre), post(post) {}

        char const* pre;
        char const* post;
    };

    struct formatted {
        markup type;
        std::string content;
    };

    void process(quickbook::actions&, formatted const&);
}

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::formatted,
    (quickbook::markup, type)
    (std::string, content)
)

#endif