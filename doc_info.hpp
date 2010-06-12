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
#include <boost/variant/variant.hpp>
#include "fwd.hpp"
#include "strings.hpp"

namespace quickbook
{
    struct doc_info
    {
        typedef std::vector<unsigned int> copyright_years;
        typedef std::pair<copyright_years, std::string> copyright_entry;
        typedef std::vector<copyright_entry> copyright_list;
        typedef std::vector<raw_string> category_list;
        typedef std::pair<std::string, std::string> author;
        typedef std::vector<author> author_list;
        typedef boost::variant<raw_string, std::string> variant_string;
        enum variant_string_enum { raw_string_type, string_type };

        std::string             doc_type;
        raw_string              doc_title;
        raw_string              doc_version;
        raw_string              doc_id;
        raw_string              doc_dirname;
        copyright_list          doc_copyrights;
        variant_string          doc_purpose;
        category_list           doc_categories;
        author_list             doc_authors;
        variant_string          doc_license;
        raw_string              doc_last_revision;
        bool                    ignore;
    };
    
    struct doc_info_post
    {
        doc_info_post(doc_info& info) : info(info) {}
        doc_info& info;
    };
}

#endif // BOOST_SPIRIT_QUICKBOOK_GRAMMARS_HPP
