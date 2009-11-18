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
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_actor.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_symbols.hpp>

namespace quickbook
{
    using namespace boost::spirit;

    template <typename Actions>
    template <typename Scanner>
    doc_info_grammar<Actions>::
        definition<Scanner>::definition(doc_info_grammar const& self)
        : unused(false), common(self.actions, unused)
    {
        Actions& actions = self.actions;

        doc_types =
            "book", "article", "library", "chapter", "part"
          , "appendix", "preface", "qandadiv", "qandaset"
          , "reference", "set"
        ;
        
        doc_info =
            space
            >> '[' >> space
            >> (doc_types >> classic::eps_p)
                                            [classic::assign_a(actions.doc_type)]
            >> hard_space
            >>  (  *(classic::anychar_p -
                    (classic::ch_p('[') | ']' | classic::eol_p)
                    )
                )                           [classic::assign_a(actions.doc_title)]
            >>  !(
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
                    | doc_copyright         [classic::push_back_a(actions.doc_copyrights, copyright)]
                    | doc_purpose           [actions.extract_doc_purpose]
                    | doc_category
                    | doc_authors
                    | doc_license           [actions.extract_doc_license]
                    | doc_last_revision
                    | doc_source_mode
                    )
                    >> space >> ']' >> +classic::eol_p
                )
            >> space >> ']' >> +classic::eol_p
            ;

        quickbook_version =
                "quickbook" >> hard_space
            >>  (   classic::uint_p         [classic::assign_a(qbk_major_version)]
                    >> '.' 
                    >>  uint2_t()           [classic::assign_a(qbk_minor_version)]
                )
            ;

        doc_version =
                "version" >> hard_space
            >> (*(classic::anychar_p - ']'))
                                            [classic::assign_a(actions.doc_version)]
            ;

        doc_id =
                "id" >> hard_space
            >> (*(classic::anychar_p - ']'))
                                            [classic::assign_a(actions.doc_id)]
            ;

        doc_dirname =
                "dirname" >> hard_space
            >> (*(classic::anychar_p - ']'))
                                            [classic::assign_a(actions.doc_dirname)]
            ;

        doc_copyright =
                "copyright" >> hard_space   [classic::clear_a(copyright.first)]
            >> +( classic::repeat_p(4)[classic::digit_p]
                                            [classic::push_back_a(copyright.first)]
                  >> space
                )
            >> space
            >> (*(classic::anychar_p - ']'))
                                            [classic::assign_a(copyright.second)]
            ;

        doc_purpose =
                "purpose" >> hard_space
            >> phrase                       [classic::assign_a(actions.doc_purpose_1_1)]
            ;

        doc_category =
                "category" >> hard_space
            >> (*(classic::anychar_p - ']'))
                                            [classic::assign_a(actions.doc_category)]
            ;

        doc_author =
                space
            >>  '[' >> space
            >>  (*(classic::anychar_p - ','))
                                            [classic::assign_a(name.second)] // surname
            >>  ',' >> space
            >>  (*(classic::anychar_p - ']'))
                                            [classic::assign_a(name.first)] // firstname
            >>  ']'
            ;

        doc_authors =
                "authors" >> hard_space
            >> doc_author                   [classic::push_back_a(actions.doc_authors, name)]
            >> *(   ','
                    >>  doc_author          [classic::push_back_a(actions.doc_authors, name)]
                )
            ;

        doc_license =
                "license" >> hard_space
            >> phrase                       [classic::assign_a(actions.doc_license_1_1)]
            ;

        doc_last_revision =
                "last-revision" >> hard_space
            >> (*(classic::anychar_p - ']'))[classic::assign_a(actions.doc_last_revision)]
            ;

        doc_source_mode =
                "source-mode" >> hard_space
            >>  (
                   classic::str_p("c++") 
                |  "python"
                |  "teletype"
                )                           [classic::assign_a(actions.source_mode)]
            ;

        comment =
            "[/" >> *(classic::anychar_p - ']') >> ']'
            ;

        space =
            *(classic::space_p | comment)
            ;

        hard_space =
            (classic::eps_p - (classic::alnum_p | '_')) >> space
                                                // must not be preceded by
            ;                                   // alpha-numeric or underscore

        phrase =
           *(   common
            |   comment
            |   (classic::anychar_p - ']')  [actions.plain_char]
            )
            ;
    }
}

#endif // BOOST_SPIRIT_QUICKBOOK_DOC_INFO_HPP

