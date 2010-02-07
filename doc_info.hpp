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

namespace quickbook
{
    struct doc_info
    {
        typedef std::vector<std::string> copyright_years;
        typedef std::pair<copyright_years, std::string> copyright_entry;
        typedef std::vector<copyright_entry> copyright_list;
        typedef std::pair<std::string, std::string> author;
        typedef std::vector<author> author_list;

        std::string             doc_type;
        std::string             doc_title;
        std::string             doc_version;
        std::string             doc_id;
        std::string             doc_dirname;
        copyright_list          doc_copyrights;
        std::string             doc_purpose;
        std::string             doc_category;
        author_list             doc_authors;
        std::string             doc_license;
        std::string             doc_last_revision;
        bool                    ignore;
    };
    
    struct doc_info_post
    {
        doc_info_post(doc_info& info) : info(info) {}
        doc_info& info;
    };
}

#endif // BOOST_SPIRIT_QUICKBOOK_GRAMMARS_HPP
