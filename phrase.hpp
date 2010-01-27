/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_PHRASE_HPP)
#define BOOST_SPIRIT_QUICKBOOK_PHRASE_HPP

#include <string>

namespace quickbook
{
    // TODO: Add to a forward header somewhere.
    class actions;

    struct source_mode {
        source_mode() {}
        source_mode(std::string const& m) : mode(m) {}

        std::string mode;
    };

    struct link_type {
        link_type()
            : pre(""), post("") {}
        link_type(char const* pre, char const* post)
            : pre(pre), post(post) {}

        char const* pre;
        char const* post;
    };
    
    struct link {
        link_type type;
        std::string destination;
        std::string content;
    };

    void process(quickbook::actions& actions, source_mode const& s);
    void process(quickbook::actions& actions, link const& x);
}

#endif // BOOST_SPIRIT_QUICKBOOK_PHRASE_HPP

