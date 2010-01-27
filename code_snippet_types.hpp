/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2005 Thomas Guest
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_CODE_SNIPPET_TYPES_HPP)
#define BOOST_SPIRIT_QUICKBOOK_CODE_SNIPPET_TYPES_HPP

#include <vector>
#include <string>
#include "./detail/actions.hpp"

namespace quickbook
{
    struct code_snippet_actions
    {
        code_snippet_actions(std::vector<template_symbol>& storage,
                                 std::string const& doc_id,
                                 char const* source_type)
            : storage(storage)
            , doc_id(doc_id)
            , source_type(source_type)
        {}

        void pass_thru(char);
        void escaped_comment(std::string const&);
        void compile(boost::iterator_range<iterator>);
        void callout(std::string const&, char const* role);
        void inline_callout(std::string const&);
        void line_callout(std::string const&);

        std::string code;
        std::string snippet;
        std::string id;
        std::vector<std::string> callouts;
        std::vector<template_symbol>& storage;
        std::string const doc_id;
        char const* const source_type;
    };
}

#endif