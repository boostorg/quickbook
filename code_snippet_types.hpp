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
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/support_unused.hpp>
#include "fwd.hpp"
// TODO: Convert template_symbol into a struct so it can be forward declared.
#include "template_stack.hpp"

namespace quickbook
{
    using boost::spirit::unused_type;

    struct code_snippet
    {
        file_position position;
        std::string identifier;
    };
    
    struct callout
    {
        std::string content;
        char const* role;
    };
    
    struct escaped_comment
    {
        std::string content;
        char const* dummy;
    };
}

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::code_snippet,
    (quickbook::file_position, position)
    (std::string, identifier)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::callout,
    (std::string, content)
    (char const*, role)
)

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::escaped_comment,
    (std::string, content)
    (char const*, dummy)
)

namespace quickbook
{
    using boost::spirit::unused_type;

    struct code_snippet_actions
    {
        code_snippet_actions(std::vector<template_symbol>& storage,
                                 std::string const& doc_id,
                                 char const* source_type)
            : process(*this)
            , output(*this)
            , storage(storage)
            , doc_id(doc_id)
            , source_type(source_type)
        {}

        struct process_action
        {
            explicit process_action(code_snippet_actions& a)
                : actions(a) {}

            void operator()(char x, unused_type, unused_type) const;
            void operator()(callout const& x, unused_type, unused_type) const;
            void operator()(escaped_comment const& x, unused_type, unused_type) const;

            code_snippet_actions& actions;
        };

        struct output_action
        {
            explicit output_action(code_snippet_actions& a)
                : actions(a) {}
        
            void operator()(code_snippet const& x, unused_type, unused_type) const;

            code_snippet_actions& actions;
        };

        process_action process;
        output_action output;
        std::string code;
        std::string snippet;
        std::vector<std::string> callouts;
        std::vector<template_symbol>& storage;
        std::string const doc_id;
        char const* const source_type;
    };
}

#endif