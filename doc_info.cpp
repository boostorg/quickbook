/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "doc_info.hpp"
#include "grammars.hpp"
#include "actions.hpp"
#include "state.hpp"
#include "parse_utils.hpp"
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/qi_attr_cast.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/reverse_view.hpp>

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;
    
    void set_quickbook_version(boost::optional<std::pair<unsigned, unsigned> > version)
    {
        if (version)
        {
            qbk_major_version = version->first;
            qbk_minor_version = version->second;
        }
        else
        {
            qbk_major_version = 1;
            qbk_minor_version = 1;

            // TODO:
            //detail::outwarn(actions.filename.native_file_string(),1)
            //    << "Warning: Quickbook version undefined. "
            //       "Version 1.1 is assumed" << std::endl;
        }

        qbk_version_n = (qbk_major_version * 100) + qbk_minor_version;
    }

    struct doc_info_grammar::rules
    {
        rules(quickbook::actions& actions);

        quickbook::actions& actions;
        bool unused;
        phrase_grammar common;
        qi::symbols<char> doc_types;
        qi::rule<iterator, doc_info()> doc_info_details;
        qi::rule<iterator> comment, space, hard_space;
        qi::rule<iterator, std::pair<unsigned, unsigned>()> quickbook_version;
        qi::rule<iterator, std::string()> phrase, doc_version, doc_id, doc_dirname, doc_category, doc_last_revision, doc_source_mode, doc_purpose, doc_license;
        qi::rule<iterator, std::pair<std::vector<unsigned int>, std::string>()> doc_copyright;
        qi::rule<iterator, std::vector<std::pair<std::string, std::string> >()> doc_authors;
        qi::rule<iterator, boost::fusion::reverse_view<
                std::pair<std::string, std::string> >()> doc_author;
    };

    doc_info_grammar::doc_info_grammar(quickbook::actions& actions)
            : doc_info_grammar::base_type(start)
            , rules_pimpl(new rules(actions))
            , start(rules_pimpl->doc_info_details) {}

    doc_info_grammar::~doc_info_grammar() {}


    doc_info_grammar::rules::rules(quickbook::actions& actions)
            : actions(actions), unused(false), common(actions, unused)
    {
        typedef qi::uint_parser<int, 10, 1, 2>  uint2_t;

        doc_types =
            "book", "article", "library", "chapter", "part"
          , "appendix", "preface", "qandadiv", "qandaset"
          , "reference", "set"
        ;
        
        doc_info_details =
            space
            >> '[' >> space
            >> qi::raw[doc_types]           [member_assign(&doc_info::doc_type)]
            >> hard_space
            >>  (  *(qi::char_ -
                    (qi::char_('[') | ']' | qi::eol)
                    )
                )                           [member_assign(&doc_info::doc_title)]
            >>  quickbook_version           [set_quickbook_version]
            >>
                *(
                    space >> '[' >>
                    (
                      doc_version           [member_assign(&doc_info::doc_version)]
                    | doc_id                [member_assign(&doc_info::doc_id)]
                    | doc_dirname           [member_assign(&doc_info::doc_dirname)]
                    | doc_copyright         [ph::push_back(ph::bind(&doc_info::doc_copyrights, qi::_val), qi::_1)]
                    | doc_purpose           [member_assign(&doc_info::doc_purpose)]
                    | doc_category          [member_assign(&doc_info::doc_category)]
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

        quickbook_version = -(
                space >> '['
            >>  "quickbook"
            >>  hard_space
            >>  qi::uint_
            >>  '.' 
            >>  uint2_t()
            >>  space >> ']'
            );

        doc_version = "version" >> hard_space >> *(qi::char_ - ']');
        doc_id      = "id"      >> hard_space >> *(qi::char_ - ']');
        doc_dirname = "dirname" >> hard_space >> *(qi::char_ - ']');
        doc_category="category" >> hard_space >> *(qi::char_ - ']');
        doc_last_revision = "last-revision" >> hard_space >> *(qi::char_ - ']');

        doc_copyright =
                "copyright"
            >>  hard_space
            >>  +(qi::uint_ >> space)
            >>  qi::raw[(*(qi::char_ - ']'))]
            ;

        doc_purpose =
                "purpose" >> hard_space
            >>  (
                    qi::eps(qbk_before(103)) >> qi::raw[phrase] |
                    qi::eps(qbk_since(103)) >> phrase
                )
            ;

        doc_author =
                space
            >>  '['
            >> space
            >>  (*(qi::char_ - ','))
            >>  ',' >> space
            >>  (*(qi::char_ - ']'))
            >>  ']'
            ;

        doc_authors = "authors" >> hard_space >> (doc_author % ',') ;

        doc_license =
                "license" >> hard_space
            >>  (
                    qi::eps(qbk_before(103)) >> qi::raw[phrase] |
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

        comment =
            "[/" >> *(qi::char_ - ']') >> ']'
            ;

        space =
            *(qi::space | comment)
            ;

        hard_space =
            !(qi::alnum | '_') >> space     // must not be preceded by
            ;                               // alpha-numeric or underscore

        phrase =
                qi::eps                     [actions.phrase_push]
            >>  *(   common
                |   comment
                |   (qi::char_ - ']')       [actions.process]
                )
            >>  qi::eps                     [actions.phrase_pop]
            ;
    }
}
