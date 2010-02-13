/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_TEMPLATE_STACK_HPP)
#define BOOST_SPIRIT_QUICKBOOK_TEMPLATE_STACK_HPP

#include <string>
#include <deque>
#include <vector>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/home/qi/detail/assign_to.hpp>
#include "fwd.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;

    struct template_value
    {
        template_value() {}
        template_value(
            quickbook::file_position position,
            std::string const& content)
        :
            position(position),
            content(content)
        {}
    
        quickbook::file_position position;
        std::string content;
    };

    struct define_template
    {
        define_template() {}    

        define_template(
            std::string id,
            std::vector<std::string> params,
            template_value body
            )
        :
            id(id), params(params), body(body)
        {}

        std::string id;
        std::vector<std::string> params;
        template_value body;
    };

    struct call_template {
        file_position position;
        bool escape;
        template_symbol const* symbol;
        std::vector<template_value> args;
    };

    struct template_scope;
    struct template_symbol;

    struct template_stack
    {
        typedef std::deque<template_scope> deque;

        struct parser : boost::spirit::qi::primitive_parser<parser>
        {
            template <typename Ctx, typename Itr>
            struct attribute {
                typedef template_symbol const* type;
            };
        
            parser(template_stack& ts)
                : ts(ts) {}

            template <typename Context, typename Skipper, typename Attribute>
            bool parse(iterator& first, iterator const& last
              , Context& context, Skipper const& skipper, Attribute& attr) const
            {
                boost::spirit::qi::skip_over(first, last, skipper);
                template_symbol const* symbol = ts.prefix_find(first, last);
                if(symbol) boost::spirit::traits::assign_to(symbol, attr);
                return symbol;
            }

            template_stack& ts;
        };

        template_stack();
        ~template_stack();
        template_symbol const* prefix_find(iterator& first, iterator const& last) const;
        template_symbol const* find(std::string const& symbol) const;
        template_symbol const* find_top_scope(std::string const& symbol) const;
        template_scope const& top_scope() const;
        // Add the given template symbol to the current scope.
        // If a parent scope isn't supplied, uses the current scope.
        bool add(define_template const&, template_scope const* parent = 0);
        void push();
        void pop();

        // Set the current scope's parent.
        void set_parent_scope(template_scope const&);

        parser scope;

    private:

        friend struct parser;
        deque scopes;
        
        template_stack(template_stack const&);
        template_stack& operator=(template_stack const&);        
    };
}

#endif // BOOST_SPIRIT_QUICKBOOK_TEMPLATE_STACK_HPP

