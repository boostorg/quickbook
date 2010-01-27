/*=============================================================================
    Copyright (c) 2009 Daniel James
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(BOOST_SPIRIT_QUICKBOOK_QUICKBOOK_HPP)
#define BOOST_SPIRIT_QUICKBOOK_QUICKBOOK_HPP

#include <boost/spirit/include/phoenix_function.hpp>

namespace quickbook
{
    extern unsigned qbk_major_version;
    extern unsigned qbk_minor_version;
    extern unsigned qbk_version_n; // qbk_major_version * 100 + qbk_minor_version

    struct quickbook_since_impl {
        template <typename Arg1>
        struct result { typedef bool type; };
        
        bool operator()(unsigned min_) const {
            return qbk_version_n >= min_;
        }
    };

    struct quickbook_before_impl {
        template <typename Arg1>
        struct result { typedef bool type; };
        
        bool operator()(unsigned max_) const {
            return qbk_version_n < max_;
        }
    };

    namespace {
        boost::phoenix::function<quickbook_since_impl> qbk_since;
        boost::phoenix::function<quickbook_before_impl> qbk_before;
    }
}

#endif
