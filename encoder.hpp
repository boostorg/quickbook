/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(BOOST_SPIRIT_QUICKBOOK_OUTPUT_HPP)
#define BOOST_SPIRIT_QUICKBOOK_OUTPUT_HPP

#include <boost/shared_ptr.hpp>
#include "fwd.hpp"
#include "phrase.hpp"
#include "block.hpp"
#include "syntax_highlight.hpp"
#include "doc_info.hpp"
#include "gen_types.hpp"
#include "template.hpp"

namespace quickbook
{
    // Base class for encoders.
    //
    // The methods in this class possibly shouldn't be const, as it might be
    // useful to store some state eg. in case anything needs to rearranged in
    // order to create a valid document. Maybe this should act more like a
    // stream? It could have something like the collector interface, with
    // push and pop methods.
    
    struct encoder {
        void operator()(quickbook::state&, nothing) {}
    
        virtual void operator()(quickbook::state&, doc_info const&) = 0;
        virtual void operator()(quickbook::state&, doc_info_post const&) = 0;
    
        // Note: char is a plain quickbook character, string is an encoded
        // boostbook string. Oops.
        virtual void operator()(quickbook::state&, char) = 0;
        virtual void operator()(quickbook::state&, std::string const&) = 0;
        virtual void operator()(quickbook::state&, anchor const&) = 0;
        virtual void operator()(quickbook::state&, link const&) = 0;
        virtual void operator()(quickbook::state&, formatted const&) = 0;
        virtual void operator()(quickbook::state&, break_ const&) = 0;
        virtual void operator()(quickbook::state&, image2 const&) = 0;
    
        virtual void operator()(quickbook::state&, hr) = 0;
        virtual void operator()(quickbook::state&, begin_section2 const&) = 0;
        virtual void operator()(quickbook::state&, end_section2 const&) = 0;
        virtual void operator()(quickbook::state&, heading2 const&) = 0;
        virtual void operator()(quickbook::state&, variablelist const&) = 0;
        virtual void operator()(quickbook::state&, table2 const&) = 0;
        virtual void operator()(quickbook::state&, xinclude2 const&) = 0;
        virtual void operator()(quickbook::state&, list2 const&) = 0;
        virtual void operator()(quickbook::state&, callout_link const&) = 0;
        virtual void operator()(quickbook::state&, callout_list const&) = 0;
    
        virtual void operator()(quickbook::state&, code_token const&) = 0;
    
        virtual std::string encode(std::string const&) = 0;
        virtual std::string encode(char) = 0;
        virtual std::string encode(char const*) = 0;
    };

    struct encode_action
    {
        encode_action(quickbook::state& state,
            quickbook::encoder& encoder)
            : state(state), encoder(encoder) {}
      
        template <typename T>
        void operator()(T const& x) const {
            encoder(state, x);
        }

        quickbook::state& state;
        quickbook::encoder& encoder;
    };
}

#endif
