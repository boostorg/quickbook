/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_GRAMMARS_HPP)
#define BOOST_SPIRIT_QUICKBOOK_GRAMMARS_HPP

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/scoped_ptr.hpp>
#include "./detail/actions_class.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;

    struct phrase_grammar : qi::grammar<iterator>
    {
        phrase_grammar(quickbook::actions& actions, bool& no_eols);
        ~phrase_grammar();

        struct rules;
        boost::scoped_ptr<rules> rules_pimpl;
        qi::rule<iterator> start;
    private:
        phrase_grammar(phrase_grammar const&);
        phrase_grammar& operator=(phrase_grammar const&);
    };

    struct simple_phrase_grammar : qi::grammar<iterator>
    {
        simple_phrase_grammar(quickbook::actions& actions);
        ~simple_phrase_grammar();

        struct rules;
        boost::scoped_ptr<rules> rules_pimpl;
        qi::rule<iterator> start;
    private:
        simple_phrase_grammar(simple_phrase_grammar const&);
        simple_phrase_grammar& operator=(simple_phrase_grammar const&);
    };

    struct block_grammar : qi::grammar<iterator>
    {
        block_grammar(quickbook::actions& actions);
        ~block_grammar();

        struct rules;
        boost::scoped_ptr<rules> rules_pimpl;
        qi::rule<iterator> start;
    private:
        block_grammar(block_grammar const&);
        block_grammar& operator=(block_grammar const&);
    };

    struct doc_info
    {
        typedef std::vector<std::string> string_list;
        typedef std::vector<std::pair<std::string, std::string> > author_list;
        typedef std::vector<std::pair<string_list, std::string> > copyright_list;

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
    };

    struct doc_info_grammar : qi::grammar<iterator, doc_info()>
    {
        doc_info_grammar(quickbook::actions& actions);
        ~doc_info_grammar();

        struct rules;
        boost::scoped_ptr<rules> rules_pimpl;
        qi::rule<iterator, doc_info()> start;
    private:
        doc_info_grammar(doc_info_grammar const&);
        doc_info_grammar& operator=(doc_info_grammar const&);
    };

    // TODO: Duplicate declaration:
    
    struct code_snippet_actions;

    struct python_code_snippet_grammar
        : qi::grammar<iterator>
    {
        typedef code_snippet_actions actions_type;
    
        python_code_snippet_grammar(actions_type& actions);
        ~python_code_snippet_grammar();

        struct rules;
        boost::scoped_ptr<rules> rules_pimpl;
        qi::rule<iterator> start;
    private:
        python_code_snippet_grammar(python_code_snippet_grammar const&);
        python_code_snippet_grammar& operator=(python_code_snippet_grammar const&);
    };

    struct cpp_code_snippet_grammar
        : qi::grammar<iterator>
    {
        typedef code_snippet_actions actions_type;
    
        cpp_code_snippet_grammar(actions_type& actions);
        ~cpp_code_snippet_grammar();

        struct rules;
        boost::scoped_ptr<rules> rules_pimpl;
        qi::rule<iterator> start;
    private:
        cpp_code_snippet_grammar(cpp_code_snippet_grammar const&);
        cpp_code_snippet_grammar& operator=(cpp_code_snippet_grammar const&);
    };
}

#endif // BOOST_SPIRIT_QUICKBOOK_GRAMMARS_HPP
