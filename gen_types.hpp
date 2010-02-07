/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(BOOST_SPIRIT_QUICKBOOK_GEN_TYPES_HPP)
#define BOOST_SPIRIT_QUICKBOOK_GEN_TYPES_HPP

#include <string>
#include <vector>
#include <map>
#include <boost/optional.hpp>
#include "block.hpp"

namespace quickbook
{
    struct image2 {
        typedef std::map<std::string, std::string> attribute_map;
        
        image2(attribute_map const& attributes)
            : attributes(attributes) {}

        attribute_map attributes;
    };
    
    struct begin_section2 {
        std::string id;
        std::string content;
        std::string linkend;
    };

    struct end_section2 {
    };

    struct heading2
    {
        int level;
        std::string id;
        std::string content;
        std::string linkend;
    };

    struct table2
    {
        boost::optional<std::string> id;
        boost::optional<std::string> title;
        int cols;
        boost::optional<table_row> head;
        std::vector<table_row> rows;
    };

    struct xinclude2
    {
        std::string path;
    };

    struct list_item2;

    struct list2
    {
        char mark;
        std::vector<list_item2> items;
    };

    struct list_item2
    {
        std::string content;
        list2 sublist;
    };
}

#endif
