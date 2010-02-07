/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_PHRASE_HPP)
#define BOOST_SPIRIT_QUICKBOOK_PHRASE_HPP

#include <vector>
#include <string>
#include <map>
#include "fwd.hpp"
#include "parse_types.hpp"

namespace quickbook
{
    struct source_mode {
        source_mode() {}
        source_mode(std::string const& m) : mode(m) {}

        std::string mode;
    };
    
    struct call_template {
        file_position position;
        bool escape;
        template_symbol const* symbol;
        std::vector<std::string> args;
    };

    struct anchor {
        std::string id;
    };
    
    struct link {
        formatted_type type;
        std::string destination;
        std::string content;
    };
    
    struct simple_markup {
        char symbol;
        std::string raw_content;
    };
    
    struct cond_phrase {
        std::string macro_id;
        std::string content;
    };
    
    struct break_ {
        file_position position;
    };
    
    struct image {
        typedef std::multimap<std::string, std::string> attribute_map;
    
        file_position position;
        std::string image_filename;
        attribute_map attributes;
    };
}

#endif // BOOST_SPIRIT_QUICKBOOK_PHRASE_HPP

