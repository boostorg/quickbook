/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/qi_attr_cast.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include "doc_info.hpp"
#include "grammar_impl.hpp"
#include "actions.hpp"
#include "state.hpp"
#include "parse_utils.hpp"
#include "misc_rules.hpp"
#include "utils.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace repo = boost::spirit::repository;
    namespace ph = boost::phoenix;

    struct doc_info_grammar_local
    {
        qi::symbols<char> doc_types;
        qi::rule<iterator, version()> quickbook_version;
        qi::rule<iterator, docinfo_string()> doc_version;
        qi::rule<iterator, docinfo_string()> doc_id;
        qi::rule<iterator, docinfo_string()> doc_dirname;
        qi::rule<iterator, docinfo_string()> doc_category;
        qi::rule<iterator, docinfo_string()> doc_last_revision;
        qi::rule<iterator, std::string()> doc_source_mode; // TODO: raw_source
        qi::rule<iterator, docinfo_string()> doc_purpose;
        qi::rule<iterator, docinfo_string()> doc_license;
        qi::rule<iterator, doc_info::copyright_entry()> doc_copyright;
        qi::rule<iterator, doc_info::author_list()> doc_authors;
        qi::rule<iterator, doc_info::author()> doc_author;
        qi::rule<iterator, docinfo_string()> doc_info_phrase;
        qi::rule<iterator, docinfo_string()> doc_info_text;
        qi::rule<iterator, docinfo_string()> doc_info_text_comma;
        qi::rule<iterator, docinfo_string()> doc_info_title;
        qi::rule<iterator, std::string()> doc_info_phrase_impl;
        qi::rule<iterator, std::string()> doc_info_text_impl;
        qi::rule<iterator, std::string()> doc_info_text_comma_impl;
        qi::rule<iterator, std::string()> doc_info_title_impl;
    };

    void quickbook_grammar::impl::init_doc_info()
    {
        doc_info_grammar_local& local = store_.create();

        typedef qi::uint_parser<int, 10, 1, 2>  uint2_t;

        local.doc_types =
            "book", "article", "library", "chapter", "part"
          , "appendix", "preface", "qandadiv", "qandaset"
          , "reference", "set"
        ;

        doc_info_details =
            repo::confix(space >> '[' >> space, space >> ']' >> +qi::eol)
            [   qi::raw[local.doc_types]    [member_assign(&doc_info::doc_type)]
            >>  hard_space
            >>  local.doc_info_title        [member_assign(&doc_info::doc_title)]
            >>  local.quickbook_version     [actions.process]
            >>  *repo::confix(space >> '[', space >> ']' >> +qi::eol)
                [   local.doc_version       [member_assign(&doc_info::doc_version)]
                |   local.doc_id            [member_assign(&doc_info::doc_id)]
                |   local.doc_dirname       [member_assign(&doc_info::doc_dirname)]
                |   local.doc_copyright     [member_push(&doc_info::doc_copyrights)]
                |   local.doc_purpose       [member_assign(&doc_info::doc_purpose)]
                |   local.doc_category      [member_push(&doc_info::doc_categories)]
                |   local.doc_authors       [member_assign(&doc_info::doc_authors)]
                |   local.doc_license       [member_assign(&doc_info::doc_license)]
                |   local.doc_last_revision
                                            [member_assign(&doc_info::doc_last_revision)]
                    // This has to be set in actions so that source code in phrases use the
                    // correct encoding.
                |   local.doc_source_mode   [ph::ref(actions.state_.source_mode) = qi::_1]
                ]
            ]
            ;

        local.quickbook_version =
                position                    [member_assign(&version::position)]
            >>  -repo::confix(space >> '[', space >> ']')
                [   "quickbook"
                >>  hard_space
                >>  qi::uint_               [member_assign(&version::major)]
                >>  '.'
                >>  uint2_t()               [member_assign(&version::minor)]
                ]
            ;

        local.doc_version = "version" >> hard_space >> local.doc_info_text;
        local.doc_id      = "id"      >> hard_space >> local.doc_info_text;
        local.doc_dirname = "dirname" >> hard_space >> local.doc_info_text;
        local.doc_category="category" >> hard_space >> local.doc_info_text;
        local.doc_last_revision = "last-revision" >> hard_space >> local.doc_info_text;

        local.doc_copyright =
                "copyright"
            >>  hard_space
            >>  (+(qi::uint_ >> space))     [member_assign(&doc_info::copyright_entry::years)]
            >>  local.doc_info_text         [member_assign(&doc_info::copyright_entry::holder)]
            ;

        local.doc_purpose =
                "purpose"
            >>  hard_space
            >>  local.doc_info_phrase
            ;

        local.doc_author =
                '['
            >>  space
            >>  local.doc_info_text_comma   [member_assign(&doc_info::author::surname)]
            >>  ',' >> space
            >>  local.doc_info_text         [member_assign(&doc_info::author::firstname)]
            >>  ']'
            ;

        local.doc_authors
            =   "authors"
            >>  hard_space
            >>  (   (local.doc_author >> space)
                %   -(qi::char_(',') >> space)
                );

        local.doc_license =
                "license"
            >>  hard_space
            >>  local.doc_info_phrase
            ;

        local.doc_source_mode =
                "source-mode" >> hard_space
            >>  (
                   qi::string("c++")
                |  qi::string("python")
                |  qi::string("teletype")
                )
            ;

        local.doc_info_phrase =
            qi::raw[
                local.doc_info_phrase_impl  [member_assign(&docinfo_string::encoded)]
            ]                               [member_assign(&docinfo_string::raw)]
            ;

        local.doc_info_text =
            qi::raw[
                local.doc_info_text_impl    [member_assign(&docinfo_string::encoded)]
            ]                               [member_assign(&docinfo_string::raw)]
            ;

        local.doc_info_text_comma =
            qi::raw[
                local.doc_info_text_comma_impl
                                            [member_assign(&docinfo_string::encoded)]
            ]                               [member_assign(&docinfo_string::raw)]
            ;

        local.doc_info_title =
            qi::raw[
                local.doc_info_title_impl   [member_assign(&docinfo_string::encoded)]
            ]                               [member_assign(&docinfo_string::raw)]
            ;

        local.doc_info_phrase_impl =
                qi::eps                     [actions.phrase_push]
            >>  *(  common
                |   comment
                |   (~qi::char_(']'))       [actions.process]
                )
            >>  qi::eps                     [actions.phrase_pop]
            ;

        local.doc_info_text_impl =
                qi::eps                     [actions.phrase_push]
            >>  *(  escape
                |   (~qi::char_(']'))       [actions.process]
                )
            >>  qi::eps                     [actions.phrase_pop]
            ;

        local.doc_info_text_comma_impl =
                qi::eps                     [actions.phrase_push]
            >>  *(  escape
                |   (~qi::char_("],"))      [actions.process]
                )
            >>  qi::eps                     [actions.phrase_pop]
            ;

        local.doc_info_title_impl =
                qi::eps                     [actions.phrase_push]
            >>  *(  escape
                |   (~qi::char_("[]") - qi::eol)
                                            [actions.process]
                )
            >>  qi::eps                     [actions.phrase_pop]
            ;
    }
}
