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
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include "doc_info.hpp"
#include "grammar_impl.hpp"
#include "actions.hpp"
#include "state.hpp"
#include "parse_utils.hpp"
#include "misc_rules.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;
    
    void set_quickbook_version(boost::fusion::vector<unsigned, unsigned> version)
    {
        qbk_major_version = boost::fusion::at_c<0>(version);
        qbk_minor_version = boost::fusion::at_c<1>(version);
        qbk_version_n = (qbk_major_version * 100) + qbk_minor_version;
    }
    
    void default_quickbook_version()
    {
        qbk_major_version = 1;
        qbk_minor_version = 1;
        qbk_version_n = 101;

        // TODO:
        //detail::outwarn(actions.filename.native_file_string(),1)
        //    << "Warning: Quickbook version undefined. "
        //       "Version 1.1 is assumed" << std::endl;
    }

    void quickbook_grammar::impl::init_doc_info()
    {
        qi::symbols<char>& doc_types = store_.create();
        qi::rule<iterator>& quickbook_version = store_.create();
        qi::rule<iterator, std::string()>& phrase = store_.create();
        qi::rule<iterator, raw_source()>& doc_version = store_.create();
        qi::rule<iterator, raw_source()>& doc_id = store_.create();
        qi::rule<iterator, raw_source()>& doc_dirname = store_.create();
        qi::rule<iterator, raw_source()>& doc_category = store_.create();
        qi::rule<iterator, raw_source()>& doc_last_revision = store_.create();
        qi::rule<iterator, std::string()>& doc_source_mode = store_.create(); // TODO: raw_source
        qi::rule<iterator, doc_info::variant_string()>& doc_purpose = store_.create();
        qi::rule<iterator, doc_info::variant_string()>& doc_license = store_.create();
        qi::rule<iterator, doc_info::copyright_entry()>& doc_copyright = store_.create();
        qi::rule<iterator, doc_info::author_list()>& doc_authors = store_.create();
        qi::rule<iterator, doc_info::author()>& doc_author = store_.create();
        qi::rule<iterator, quickbook::raw_string()>& raw_phrase = store_.create();

        typedef qi::uint_parser<int, 10, 1, 2>  uint2_t;

        doc_types =
            "book", "article", "library", "chapter", "part"
          , "appendix", "preface", "qandadiv", "qandaset"
          , "reference", "set"
        ;
        
        doc_info_details =
            space
            >>  '[' >> space
            >>  qi::raw[doc_types]          [member_assign(&doc_info::doc_type)]
            >>  hard_space
            >>  qi::raw[
                    *(~qi::char_("[]") - qi::eol)
                ]                           [member_assign(&doc_info::doc_title)]
            >>  quickbook_version
            >>
                *(
                    space >> '[' >>
                    (
                      doc_version           [member_assign(&doc_info::doc_version)]
                    | doc_id                [member_assign(&doc_info::doc_id)]
                    | doc_dirname           [member_assign(&doc_info::doc_dirname)]
                    | doc_copyright         [member_push(&doc_info::doc_copyrights)]
                    | doc_purpose           [member_assign(&doc_info::doc_purpose)]
                    | doc_category          [member_push(&doc_info::doc_categories)]
                    | doc_authors           [member_assign(&doc_info::doc_authors)]
                    | doc_license           [member_assign(&doc_info::doc_license)]
                    | doc_last_revision     [member_assign(&doc_info::doc_last_revision)]
                      // This has to be set in actions so that source code in phrases use the
                      // correct encoding.
                    | doc_source_mode       [ph::ref(actions.state_.source_mode) = qi::_1]
                    )
                    >> space >> ']' >> +qi::eol
                )
            >> space >> ']' >> +qi::eol
            ;

        quickbook_version =
            (   space
            >>  '['
            >>  "quickbook"
            >>  hard_space
            >>  qi::uint_
            >>  '.' 
            >>  uint2_t()
            >>  space >> ']'
            )                               [set_quickbook_version]
            | qi::eps                       [default_quickbook_version]
            ;

        doc_version = "version" >> hard_space >> qi::raw[*~qi::char_(']')];
        doc_id      = "id"      >> hard_space >> qi::raw[*~qi::char_(']')];
        doc_dirname = "dirname" >> hard_space >> qi::raw[*~qi::char_(']')];
        doc_category="category" >> hard_space >> qi::raw[*~qi::char_(']')];
        doc_last_revision = "last-revision" >> hard_space >> qi::raw[*~qi::char_(']')];

        doc_copyright =
                "copyright"
            >>  hard_space
            >>  (+(qi::uint_ >> space))     [member_assign(&doc_info::copyright_entry::years)]
            >>  qi::raw[*~qi::char_(']')]
                                            [member_assign(&doc_info::copyright_entry::holder)]
            ;

        doc_purpose =
                "purpose" >> hard_space
            >>  (
                    qi::eps(qbk_before(103)) >> raw_phrase |
                    qi::eps(qbk_since(103)) >> phrase
                )
            ;

        doc_author =
                space
            >>  '['
            >>  space
            >>  (*~qi::char_(','))          [member_assign(&doc_info::author::surname)]
            >>  ',' >> space
            >>  (*~qi::char_(']'))          [member_assign(&doc_info::author::firstname)]
            >>  ']'
            ;

        doc_authors = "authors" >> hard_space >> (doc_author % ',') ;

        doc_license =
                "license" >> hard_space
            >>  (
                    qi::eps(qbk_before(103)) >> raw_phrase |
                    qi::eps(qbk_since(103)) >> phrase
                )
            ;

        doc_source_mode =
                "source-mode" >> hard_space
            >>  (
                   qi::string("c++") 
                |  qi::string("python")
                |  qi::string("teletype")
                )
            ;

        raw_phrase =
                qi::raw[phrase]             [qi::_val = qi::_1]
            ;

        phrase =
                qi::eps                     [actions.phrase_push]
            >>  *(   common
                |   comment
                |   (~qi::char_(']'))       [actions.process]
                )
            >>  qi::eps                     [actions.phrase_pop]
            ;
    }
}
