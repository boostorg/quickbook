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
#include <boost/spirit/include/support_attributes.hpp>
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
    
    struct get_position_impl
    {
        template <typename Range, typename Context>
        void operator()(Range const& it, Context& c, bool& x) const {
            boost::spirit::_val(it, c, x) = it.begin().get_position();
        }
    };

    namespace ph = boost::phoenix;

    namespace {
        get_position_impl get_position;
        ph::function<as_string_impl> as_string;
    }
}

#endif
