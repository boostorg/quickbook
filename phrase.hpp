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

#include <string>
#include <map>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include "detail/template_stack.hpp"

namespace quickbook
{
    // TODO: Add to a forward header somewhere.
    class actions;
    typedef boost::spirit::classic::file_position file_position;

    struct source_mode {
        source_mode() {}
        source_mode(std::string const& m) : mode(m) {}

        std::string mode;
    };

    struct markup {
        markup()
            : pre(""), post("") {}
        markup(char const* pre, char const* post)
            : pre(pre), post(post) {}

        char const* pre;
        char const* post;
    };
    
    struct template_ {
        file_position position;
        bool escape;
        template_symbol symbol;
        std::vector<std::string> params;
    };

    struct anchor {
        char const* dummy;
        std::string id;
    };
    
    struct link {
        markup type;
        std::string destination;
        std::string content;
    };
    
    struct formatted {
        markup type;
        std::string content;
    };
    
    struct simple_markup {
        char symbol;
        std::string raw_content;
    };
    
    struct code {
        bool block;
        file_position position;
        std::string code;
    };
    
    struct cond_phrase {
        std::string macro_id;
        std::string content;
    };
    
    struct break_ {
        const char* dummy;
        file_position position;
    };
    
    struct image {
        typedef std::multimap<std::string, std::string> attribute_map;
    
        file_position position;
        std::string image_filename;
        attribute_map attributes;
    };

    void process(quickbook::actions&, source_mode const&);
    void process(quickbook::actions&, template_ const&);
    void process(quickbook::actions&, anchor const&);
    void process(quickbook::actions&, link const&);
    void process(quickbook::actions&, formatted const&);
    void process(quickbook::actions&, simple_markup const&);
    void process(quickbook::actions&, code const&);
    void process(quickbook::actions&, cond_phrase const&);
    void process(quickbook::actions&, break_ const&);
    void process(quickbook::actions&, image const&);
}

#endif // BOOST_SPIRIT_QUICKBOOK_PHRASE_HPP

