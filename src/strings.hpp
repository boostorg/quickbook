/*=============================================================================
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_STRINGS_HPP)
#define BOOST_SPIRIT_QUICKBOOK_STRINGS_HPP

#include "fwd.hpp"

namespace quickbook
{
    struct raw_string {
        raw_string() {}
        raw_string(raw_source const& x) : value(x.begin(), x.end()) {}
        raw_string& operator=(raw_source const& x) {
            value.assign(x.begin(), x.end());
            return *this;
        }
        raw_string& operator=(std::string const& x) {
            value = x;
            return *this;
        }
        raw_string& operator=(char const* x) {
            value = x;
            return *this;
        }
        std::string::const_iterator begin() const { return value.begin(); }
        std::string::const_iterator end() const { return value.end(); }
        bool empty() const { return value.empty(); }
        void clear() { value.clear(); }
    
        std::string value;
    };
}

#endif
