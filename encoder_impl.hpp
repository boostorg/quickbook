#include "fwd.hpp"
#include "encoder.hpp"
#include "phrase.hpp"
#include "state.hpp"

namespace quickbook
{
    struct boostbook_encoder : encoder {
        virtual void operator()(quickbook::state&, doc_info const&);
        virtual void operator()(quickbook::state&, doc_info_post const&);
    
        // Note: char is a plain quickbook character, string is an encoded
        // boostbook string. Oops.
        virtual void operator()(quickbook::state&, char);
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
    
        virtual std::string encode(std::string const&);
        virtual std::string encode(char);
        virtual std::string encode(char const*);
    };

    struct html_encoder : encoder {
        virtual void operator()(quickbook::state&, doc_info const&);
        virtual void operator()(quickbook::state&, doc_info_post const&);
    
        // Note: char is a plain quickbook character, string is an encoded
        // html string. Oops.
        virtual void operator()(quickbook::state&, char);
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
    
        virtual std::string encode(std::string const&);
        virtual std::string encode(char);
        virtual std::string encode(char const*);
    };
}