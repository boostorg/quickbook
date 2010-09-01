/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_DOC_INFO_HPP)
#define BOOST_SPIRIT_QUICKBOOK_DOC_INFO_HPP

#include <vector>
#include <string>
#include <utility>
#include "fwd.hpp"
#include "strings.hpp"

namespace quickbook
{
    struct docinfo_string {
        std::string raw;
        std::string encoded;

        docinfo_string() : raw(), encoded() {}

        void swap(docinfo_string& x) {
            raw.swap(x.raw);
            encoded.swap(x.encoded);
        }

        void clear() {
            raw.clear();
            encoded.clear();
        }

        bool empty() const {
            return raw.empty();
        }

        std::string const& get(unsigned version) const;
    };

    struct doc_info
    {
        typedef std::vector<unsigned int> copyright_years;
        struct copyright_entry {
            copyright_years years;
            docinfo_string holder;
        };
        typedef std::vector<copyright_entry> copyright_list;
        typedef std::vector<docinfo_string> category_list;
        struct author {
            docinfo_string firstname;
            docinfo_string surname;
        };
        typedef std::vector<author> author_list;

        std::string             doc_type;
        docinfo_string          doc_title;
        docinfo_string          doc_version;
        docinfo_string          doc_id;
        docinfo_string          doc_dirname;
        copyright_list          doc_copyrights;
        docinfo_string          doc_purpose;
        category_list           doc_categories;
        author_list             doc_authors;
        docinfo_string          doc_license;
        docinfo_string          doc_last_revision;
        bool                    ignore;
    };
    
    struct doc_info_post
    {
        doc_info_post(doc_info& info) : info(info) {}
        doc_info& info;
    };
    
    struct version
    {
        version() : major(-1), minor(-1) {}
    
        int major;
        int minor;
        file_position position;
    };
}

#endif // BOOST_SPIRIT_QUICKBOOK_GRAMMARS_HPP
