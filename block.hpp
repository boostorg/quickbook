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
#include "strings.hpp"

namespace quickbook
{
    struct block_formatted {
        formatted_type type;
        std::string content;
    };

    struct hr
    {
    };

    struct paragraph
    {
        std::string content;
    };
    
    struct block_separator
    {
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
        raw_source raw;
        std::string content;
    };

    struct begin_section
    {
        boost::optional<raw_string> id;
        title content;
    };
    
    struct end_section
    {
        quickbook::file_position position;
    };

    struct heading
    {
        int level;
        title content;
    };

    struct def_macro
    {
        std::string macro_identifier;
        std::string content;
    };

    typedef std::vector<quickbook::block_formatted> varlistentry;

    struct variablelist
    {
        std::string title;
        std::vector<varlistentry> entries;
    };

    typedef quickbook::block_formatted table_cell;
    typedef std::vector<table_cell> table_row;
    
    struct table
    {
        boost::optional<raw_string> id;
        std::string title;
        std::vector<table_row> rows;
    };

    struct xinclude
    {
        std::string path;
    };

    struct import
    {
        std::string path;
    };
    
    struct include
    {
        boost::optional<raw_string> id;
        std::string path;
    };
}

#endif
