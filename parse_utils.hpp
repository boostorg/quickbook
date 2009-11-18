/*=============================================================================
    Copyright (c) 2009 Daniel James
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(BOOST_SPIRIT_QUICKBOOK_AS_STRING_HPP)
#define BOOST_SPIRIT_QUICKBOOK_AS_STRING_HPP

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <string>

namespace quickbook
{
    struct as_string_impl
    {
        template <typename Arg>
        struct result { typedef std::string type; };
        
        template <typename Arg>
        std::string operator()(Arg const& arg1) const
        {
            return std::string(arg1.begin(), arg1.end());
        }
    };
    
    namespace ph = boost::phoenix;

    namespace {
        ph::function<as_string_impl> as_string;
    }
}

#endif
