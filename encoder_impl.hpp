#include "fwd.hpp"
#include "encoder.hpp"
#include "phrase.hpp"
#include "state.hpp"

namespace quickbook
{
    struct boostbook_encoder : encoder {
        virtual void operator()(quickbook::state&, doc_info const&) const;
        virtual void operator()(quickbook::state&, doc_info_post const&) const;
    
        // Note: char is a plain quickbook character, string is an encoded
        // boostbook string. Oops.
        virtual void operator()(quickbook::state&, char) const;
        virtual void operator()(quickbook::state&, std::string const&) const;
        virtual void operator()(quickbook::state&, anchor const&) const;
        virtual void operator()(quickbook::state&, link const&) const;
        virtual void operator()(quickbook::state&, formatted const&) const;
        virtual void operator()(quickbook::state&, break_ const&) const;
        virtual void operator()(quickbook::state&, image2 const&) const;
    
        virtual void operator()(quickbook::state&, hr) const;
        virtual void operator()(quickbook::state&, begin_section2 const&) const;
        virtual void operator()(quickbook::state&, end_section2 const&) const;
        virtual void operator()(quickbook::state&, heading2 const&) const;
        virtual void operator()(quickbook::state&, variablelist const&) const;
        virtual void operator()(quickbook::state&, table2 const&) const;
        virtual void operator()(quickbook::state&, xinclude2 const&) const;
        virtual void operator()(quickbook::state&, list2 const&) const;
        virtual void operator()(quickbook::state&, callout_link const&) const;
        virtual void operator()(quickbook::state&, callout_list const&) const;
    
        virtual void operator()(quickbook::state&, code_token const&) const;
    
        virtual std::string encode(std::string const&) const;
        virtual std::string encode(char) const;
        virtual std::string encode(char const*) const;
    };

    struct html_encoder : encoder {
        virtual void operator()(quickbook::state&, doc_info const&) const;
        virtual void operator()(quickbook::state&, doc_info_post const&) const;
    
        // Note: char is a plain quickbook character, string is an encoded
        // html string. Oops.
        virtual void operator()(quickbook::state&, char) const;
        virtual void operator()(quickbook::state&, std::string const&) const;
        virtual void operator()(quickbook::state&, anchor const&) const;
        virtual void operator()(quickbook::state&, link const&) const;
        virtual void operator()(quickbook::state&, formatted const&) const;
        virtual void operator()(quickbook::state&, break_ const&) const;
        virtual void operator()(quickbook::state&, image2 const&) const;
    
        virtual void operator()(quickbook::state&, hr) const;
        virtual void operator()(quickbook::state&, begin_section2 const&) const;
        virtual void operator()(quickbook::state&, end_section2 const&) const;
        virtual void operator()(quickbook::state&, heading2 const&) const;
        virtual void operator()(quickbook::state&, variablelist const&) const;
        virtual void operator()(quickbook::state&, table2 const&) const;
        virtual void operator()(quickbook::state&, xinclude2 const&) const;
        virtual void operator()(quickbook::state&, list2 const&) const;
        virtual void operator()(quickbook::state&, callout_link const&) const;
        virtual void operator()(quickbook::state&, callout_list const&) const;
    
        virtual void operator()(quickbook::state&, code_token const&) const;
    
        virtual std::string encode(std::string const&) const;
        virtual std::string encode(char) const;
        virtual std::string encode(char const*) const;
    };
}