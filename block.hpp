/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_BLOCK_HPP)
#define BOOST_SPIRIT_QUICKBOOK_BLOCK_HPP

#include <string>
#include <vector>
#include <boost/optional.hpp>
#include "fwd.hpp"
#include "parse_types.hpp"

namespace quickbook
{
    struct hr
    {
    };

    struct paragraph
    {
        std::string content;
        char const* dummy;
    };

    struct list_item
    {
        file_position position;
        std::string indent;
        char mark;
        std::string content;
    };

    typedef std::vector<list_item> list;

    struct title
    {
        std::string raw_markup;
        std::string content;
    };

    struct begin_section
    {
        boost::optional<std::string> id;
        title content;
    };
    
    struct end_section
    {
        quickbook::file_position position;
        char const* dummy;
    };

    struct heading
    {
        int level;
        title content;
    };

    struct define_template
    {
        std::string id;
        std::vector<std::string> params;
        quickbook::file_position position;
        std::string body;
    };

    struct def_macro
    {
        std::string macro_identifier;
        std::string content;
    };

    typedef std::vector<quickbook::formatted> varlistentry;

    struct variablelist
    {
        std::string title;
        std::vector<varlistentry> entries;
    };

    typedef quickbook::formatted table_cell;
    typedef std::vector<table_cell> table_row;
    
    struct table
    {
        boost::optional<std::string> id;
        std::string title;
        std::vector<table_row> rows;
    };

    struct xinclude
    {
        std::string path;
        char const* dummy;
    };

    struct import
    {
        std::string path;
        char const* dummy;
    };
    
    struct include
    {
        boost::optional<std::string> id;
        std::string path;
    };
    
    void process(quickbook::actions&, hr);
    void process(quickbook::actions&, paragraph const&);
    void process(quickbook::actions&, list const&);
    void process(quickbook::actions&, begin_section const&);
    void process(quickbook::actions&, end_section const&);
    void process(quickbook::actions&, heading const&);
    void process(quickbook::actions&, def_macro const&);
    void process(quickbook::actions&, define_template const&);
    void process(quickbook::actions&, variablelist const&);
    void process(quickbook::actions&, table const&);
    void process(quickbook::actions&, xinclude const&);
    void process(quickbook::actions&, import const&);
    void process(quickbook::actions&, include const&);
}

#endif