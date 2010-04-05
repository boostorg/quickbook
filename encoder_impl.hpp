/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "fwd.hpp"
#include "encoder.hpp"
#include "phrase.hpp"
#include "state.hpp"
#include <vector>
#include <stack>

namespace quickbook
{
    struct boostbook_encoder : encoder {
        virtual void operator()(quickbook::state&, doc_info const&);
        virtual void operator()(quickbook::state&, doc_info_post const&);
    
        // Note: char is a plain quickbook character, string is an encoded
        // boostbook string. Oops.
        virtual void operator()(quickbook::state&, char);
        virtual void operator()(quickbook::state&, unicode_char const&);
        virtual void operator()(quickbook::state&, std::string const&);
        virtual void operator()(quickbook::state&, anchor const&);
        virtual void operator()(quickbook::state&, link const&);
        virtual void operator()(quickbook::state&, formatted const&);
        virtual void operator()(quickbook::state&, break_ const&);
        virtual void operator()(quickbook::state&, image2 const&);
    
        virtual void operator()(quickbook::state&, hr);
        virtual void operator()(quickbook::state&, begin_section2 const&);
        virtual void operator()(quickbook::state&, end_section2 const&);
        virtual void operator()(quickbook::state&, heading2 const&);
        virtual void operator()(quickbook::state&, variablelist const&);
        virtual void operator()(quickbook::state&, table2 const&);
        virtual void operator()(quickbook::state&, xinclude2 const&);
        virtual void operator()(quickbook::state&, list2 const&);
        virtual void operator()(quickbook::state&, callout_link const&);
        virtual void operator()(quickbook::state&, callout_list const&);
    
        virtual void operator()(quickbook::state&, code_token const&);
    
        virtual std::string encode(raw_string const&);
        virtual std::string encode(std::string const&);
        virtual std::string encode(char);
        virtual std::string encode(char const*);
    };

    struct html_encoder : encoder {
        html_encoder();
        ~html_encoder();

        virtual void operator()(quickbook::state&, doc_info const&);
        virtual void operator()(quickbook::state&, doc_info_post const&);
    
        // Note: char is a plain quickbook character, string is an encoded
        // html string. Oops.
        virtual void operator()(quickbook::state&, char);
        virtual void operator()(quickbook::state&, unicode_char const&);
        virtual void operator()(quickbook::state&, std::string const&);
        virtual void operator()(quickbook::state&, anchor const&);
        virtual void operator()(quickbook::state&, link const&);
        virtual void operator()(quickbook::state&, formatted const&);
        virtual void operator()(quickbook::state&, break_ const&);
        virtual void operator()(quickbook::state&, image2 const&);
    
        virtual void operator()(quickbook::state&, hr);
        virtual void operator()(quickbook::state&, begin_section2 const&);
        virtual void operator()(quickbook::state&, end_section2 const&);
        virtual void operator()(quickbook::state&, heading2 const&);
        virtual void operator()(quickbook::state&, variablelist const&);
        virtual void operator()(quickbook::state&, table2 const&);
        virtual void operator()(quickbook::state&, xinclude2 const&);
        virtual void operator()(quickbook::state&, list2 const&);
        virtual void operator()(quickbook::state&, callout_link const&);
        virtual void operator()(quickbook::state&, callout_list const&);
    
        virtual void operator()(quickbook::state&, code_token const&);
    
        virtual std::string encode(raw_string const&);
        virtual std::string encode(std::string const&);
        virtual std::string encode(char);
        virtual std::string encode(char const*);
    private:
        void push_footnotes(quickbook::state&);
        void pop_footnotes(quickbook::state&);

        int footnote_id;
        struct footnote {
            footnote(int id, std::string const& content)
                : id(id), content(content) {}
        
            int id;
            std::string content;
        };
        typedef std::vector<footnote> footnotes;
        std::stack<footnotes> footnote_stack;
    };

    struct empty_visitor {
        typedef bool result_type;
    
        template <typename T>
        bool operator()(T const& x) const {
            return x.empty();
        }
    };
    
    struct encode_raw_visitor {
        typedef std::string result_type;

        encoder& encoder_;
        encode_raw_visitor(encoder& e) : encoder_(e) {}
        
        std::string operator()(raw_string const& x) const {
            return encoder_.encode(x);
        }

        std::string operator()(std::string const& x) const {
            return x;
        }
    };
}