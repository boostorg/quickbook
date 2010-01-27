/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_CODE_HPP)
#define BOOST_SPIRIT_QUICKBOOK_CODE_HPP

namespace quickbook
{
    struct code {
        bool block;
        file_position position;
        std::string code;
    };
    
    void process(quickbook::actions&, code const&);
}

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::code,
    (quickbook::file_position, position)
    (std::string, code)
    (bool, block)
)

#endif