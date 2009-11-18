/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_DOC_INFO_HPP)
#define BOOST_SPIRIT_QUICKBOOK_DOC_INFO_HPP

#include "./grammars.hpp"
#include "./detail/quickbook.hpp"
#include "./parse_utils.hpp"
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_uint.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace quickbook
{
    using namespace boost::spirit;
    namespace ph = boost::phoenix;

    template <typename Iterator, typename Actions>
    doc_info_grammar<Iterator, Actions>::doc_info_grammar(Actions& actions)
            : doc_info_grammar::base_type(doc_info), actions(actions)
            , unused(false), common(actions, unused)
    {
        typedef qi::uint_parser<int, 10, 1, 2>  uint2_t;

        doc_types =
            "book", "article", "library", "chapter", "part"
          , "appendix", "preface", "qandadiv", "qandaset"
          , "reference", "set"
        ;
        
        doc_info =
            space
            >> '[' >> space
            >> qi::raw[doc_types]           [ph::ref(actions.doc_type) = as_string(qi::_1)]
            >> hard_space
            >>  (  *(qi::char_ -
                    (qi::char_('[') | ']' | qi::eol)
                    )
                )                           [ph::ref(actions.doc_title) = as_string(qi::_1)]
            >>  -(
                    space >> '[' >>
                        quickbook_version
                    >> space >> ']'
                )
            >>
                *(
                    space >> '[' >>
                    (
                      doc_version
                    | doc_id
                    | doc_dirname
                    | doc_copyright         [ph::push_back(ph::ref(actions.doc_copyrights), ph::ref(copyright))]
                    | doc_purpose           [actions.extract_doc_purpose]
                    | doc_category
                    | doc_authors
                    | doc_license           [actions.extract_doc_license]
                    | doc_last_revision
                    | doc_source_mode
                    )
                    >> space >> ']' >> +qi::eol
                )
            >> space >> ']' >> +qi::eol
            ;

        quickbook_version =
                "quickbook" >> hard_space
            >>  (   qi::uint_               [ph::ref(qbk_major_version) = qi::_1]
                    >> '.' 
                    >>  uint2_t()           [ph::ref(qbk_minor_version) = qi::_1]
                )
            ;

        doc_version =
                "version" >> hard_space
            >> qi::raw[*(qi::char_ - ']')]  [ph::ref(actions.doc_version) = as_string(qi::_1)]
            ;

        doc_id =
                "id" >> hard_space
            >> qi::raw[*(qi::char_ - ']')]  [ph::ref(actions.doc_id) = as_string(qi::_1)]
            ;

        doc_dirname =
                "dirname" >> hard_space
            >> qi::raw[*(qi::char_ - ']')]  [ph::ref(actions.doc_dirname) = as_string(qi::_1)]
            ;

        doc_copyright =
                "copyright" >> hard_space   [ph::clear(ph::ref(copyright.first))]
            >> +( qi::repeat(4)[qi::digit]  [ph::push_back(ph::ref(copyright.first), as_string(qi::_1))]
                  >> space
                )
            >> space
            >> (*(qi::char_ - ']'))         [ph::ref(copyright.second) = as_string(qi::_1)]
            ;

        doc_purpose =
                "purpose" >> hard_space
            >> qi::raw[phrase]              [ph::ref(actions.doc_purpose_1_1) = as_string(qi::_1)]
            ;

        doc_category =
                "category" >> hard_space
            >> (*(qi::char_ - ']'))         [ph::ref(actions.doc_category) = as_string(qi::_1)]
            ;

        doc_author =
                space
            >>  '[' >> space
            >>  (*(qi::char_ - ','))        [ph::ref(name.second) = as_string(qi::_1)] // surname
            >>  ',' >> space
            >>  (*(qi::char_ - ']'))        [ph::ref(name.first) = as_string(qi::_1)] // firstname
            >>  ']'
            ;

        doc_authors =
                "authors" >> hard_space
            >> doc_author                   [ph::push_back(ph::ref(actions.doc_authors), ph::ref(name))]
            >> *(   ','
                    >>  doc_author          [ph::push_back(ph::ref(actions.doc_authors), ph::ref(name))]
                )
            ;

        doc_license =
                "license" >> hard_space
            >> qi::raw[phrase]              [ph::ref(actions.doc_license_1_1) = as_string(qi::_1)]
            ;

        doc_last_revision =
                "last-revision" >> hard_space
            >> (*(qi::char_ - ']'))         [ph::ref(actions.doc_last_revision) = as_string(qi::_1)]
            ;

        doc_source_mode =
                "source-mode" >> hard_space
            >>  (
                   qi::string("c++") 
                |  qi::string("python")
                |  qi::string("teletype")
                )                           [ph::ref(actions.source_mode) = qi::_1]
            ;

        comment =
            "[/" >> *(qi::char_ - ']') >> ']'
            ;

        space =
            *(qi::space | comment)
            ;

        hard_space =
            (qi::eps - (qi::alnum | '_')) >> space  // must not be preceded by
            ;                                       // alpha-numeric or underscore

        phrase =
           *(   common
            |   comment
            |   (qi::char_ - ']')           [actions.plain_char]
            )
            ;
    }
}

#endif // BOOST_SPIRIT_QUICKBOOK_DOC_INFO_HPP

