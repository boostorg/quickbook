/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_ACTIONS_HPP)
#define BOOST_SPIRIT_QUICKBOOK_ACTIONS_HPP

#include <time.h>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <boost/spirit/include/classic_iterator.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include "./collector.hpp"
#include "./template_stack.hpp"
#include "./utils.hpp"

#ifdef BOOST_MSVC
// disable copy/assignment could not be generated, unreferenced formal params
#pragma warning (push)
#pragma warning(disable : 4511 4512 4100)
#endif

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace fs = boost::filesystem;
    using boost::spirit::unused_type;

    // TODO: This is defined in two places.
    typedef qi::symbols<char, std::string> string_symbols;    

    typedef boost::spirit::classic::position_iterator<
        std::string::const_iterator> iterator;
    typedef boost::spirit::classic::file_position file_position;
    typedef boost::iterator_range<iterator> iterator_range;
    typedef qi::symbols<char, std::string> string_symbols;
    typedef std::map<std::string, std::string> attribute_map;

    struct actions;
    extern tm* current_time; // the current time
    extern tm* current_gm_time; // the current UTC time
    extern bool debug_mode;
    extern std::vector<std::string> include_path;

    // forward declarations
    struct actions;
    int parse(char const* filein_, actions& actor, bool ignore_docinfo = false);

    struct error_action
    {
        // Prints an error message to std::cerr

        error_action(
            int& error_count)
        : error_count(error_count) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        int& error_count;
    };

    struct phrase_action
    {
        //  blurb, blockquote, preformatted, list_item,
        //  unordered_list, ordered_list

        phrase_action(
            collector& out,
            collector& phrase,
            std::string const& pre,
            std::string const& post)
        : out(out)
        , phrase(phrase)
        , pre(pre)
        , post(post) {}

        void operator()(unused_type, unused_type, unused_type) const;

        collector& out;
        collector& phrase;
        std::string pre;
        std::string post;
    };

    struct header_action
    {
        //  Handles paragraph, h1, h2, h3, h4, h5, h6,

        header_action(
            collector& out,
            collector& phrase,
            std::string const& library_id,
            std::string const& section_id,
            std::string const& qualified_section_id,
            std::string const& pre,
            std::string const& post)
        : out(out)
        , phrase(phrase)
        , library_id(library_id)
        , section_id(section_id)
        , qualified_section_id(qualified_section_id)
        , pre(pre)
        , post(post) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
        collector& phrase;
        std::string const& library_id;
        std::string const& section_id;
        std::string const& qualified_section_id;
        std::string pre;
        std::string post;
    };

    struct generic_header_action
    {
        //  Handles h

        generic_header_action(
            collector& out,
            collector& phrase,
            std::string const& library_id,
            std::string const& section_id,
            std::string const& qualified_section_id,
            int const& section_level)
        : out(out)
        , phrase(phrase)
        , library_id(library_id)
        , section_id(section_id)
        , qualified_section_id(qualified_section_id)
        , section_level(section_level) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
        collector& phrase;
        std::string const& library_id;
        std::string const& section_id;
        std::string const& qualified_section_id;
        int const& section_level;
    };

    struct simple_phrase_action
    {
        //  Handles simple text formats

        simple_phrase_action(
            collector& out
          , std::string const& pre
          , std::string const& post
          , string_symbols const& macro)
        : out(out)
        , pre(pre)
        , post(post)
        , macro(macro) {}

        void operator()(iterator_range const&, unused_type, unused_type) const;

        collector& out;
        std::string pre;
        std::string post;
        string_symbols const& macro;
    };

    struct cond_phrase_action_pre
    {
        //  Handles conditional phrases

        cond_phrase_action_pre(
            collector& out
          , std::vector<bool>& conditions
          , string_symbols const& macro)
        : out(out)
        , conditions(conditions)
        , macro(macro) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
        std::vector<bool>& conditions;
        string_symbols const& macro;
    };

    struct cond_phrase_action_post
    {
        //  Handles conditional phrases

        cond_phrase_action_post(
            collector& out
          , std::vector<bool>& conditions
          , string_symbols const& macro)
        : out(out)
        , conditions(conditions)
        , macro(macro) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
        std::vector<bool>& conditions;
        string_symbols const& macro;
    };

    struct list_action
    {
        //  Handles lists

        typedef std::pair<char, int> mark_type;
        list_action(
            collector& out
          , collector& list_buffer
          , int& list_indent
          , std::stack<mark_type>& list_marks)
        : out(out)
        , list_buffer(list_buffer)
        , list_indent(list_indent)
        , list_marks(list_marks) {}

        void operator()(unused_type, unused_type, unused_type) const;

        collector& out;
        collector& list_buffer;
        int& list_indent;
        std::stack<mark_type>& list_marks;
    };

    struct list_format_action
    {
        //  Handles list formatting and hierarchy

        typedef std::pair<char, int> mark_type;
        list_format_action(
            collector& out
          , int& list_indent
          , std::stack<mark_type>& list_marks
          , int& error_count)
        : out(out)
        , list_indent(list_indent)
        , list_marks(list_marks)
        , error_count(error_count) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
        int& list_indent;
        std::stack<mark_type>& list_marks;
        int& error_count;
    };

    struct span
    {
        // Decorates c++ code fragments

        span(char const* name, collector& out)
        : name(name), out(out) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        char const* name;
        collector& out;
    };

    struct unexpected_char
    {
        // Handles unexpected chars in c++ syntax

        unexpected_char(collector& out)
        : out(out) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
    };

    struct anchor_action
    {
        // Handles anchors

        anchor_action(collector& out)
            : out(out) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
    };

    extern char const* quickbook_get_date;
    extern char const* quickbook_get_time;

    struct do_macro_action
    {
        // Handles macro substitutions
        
        do_macro_action(collector& phrase)
            : phrase(phrase) {}

        void operator()(std::string const& str, unused_type, unused_type) const;
        collector& phrase;
    };

    struct space
    {
        // Prints a space

        space(collector& out)
            : out(out) {}

        void operator()(iterator_range, unused_type, unused_type) const;
        void operator()(char ch, unused_type, unused_type) const;

        collector& out;
    };

    struct pre_escape_back
    {
        // Escapes back from code to quickbook (Pre)

        pre_escape_back(actions& escape_actions, std::string& save)
            : escape_actions(escape_actions), save(save) {}

        void operator()(unused_type, unused_type, unused_type) const;

        actions& escape_actions;
        std::string& save;
    };

    struct post_escape_back
    {
        // Escapes back from code to quickbook (Post)

        post_escape_back(collector& out, actions& escape_actions, std::string& save)
            : out(out), escape_actions(escape_actions), save(save) {}

        void operator()(unused_type, unused_type, unused_type) const;

        collector& out;
        actions& escape_actions;
        std::string& save;
    };

    struct raw_char_action
    {
        // Prints a single raw (unprocessed) char.
        // Allows '<', '>'... etc.

        raw_char_action(collector& phrase)
        : phrase(phrase) {}

        void operator()(char ch, unused_type, unused_type) const;
        void operator()(iterator_range, unused_type, unused_type) const;

        collector& phrase;
    };

    struct plain_char_action
    {
        // Prints a single plain char.
        // Converts '<' to "&lt;"... etc See utils.hpp

        plain_char_action(collector& phrase)
        : phrase(phrase) {}

        void operator()(char ch, unused_type, unused_type) const;
        void operator()(iterator_range, unused_type, unused_type) const;

        collector& phrase;
    };
    
    struct image_action
    {
        template <typename Arg1, typename Arg2, typename Arg3 = void>
        struct result {typedef void type; };
    
        // Handles inline images

        image_action(collector& phrase, int& error_count)
        : phrase(phrase)
        , error_count(error_count) {}

        void operator()(iterator, std::string,
            std::multimap<std::string, std::string>
                = std::multimap<std::string, std::string>()) const;

        collector& phrase;
        int& error_count;
    };

    struct markup_action
    {
        template <typename T = void> struct result { typedef void type; };

        // A generic markup action

        markup_action(collector& phrase, std::string const& str)
        : phrase(phrase), str(str) {}

        void operator()() const
        {
            phrase << str;
        }

        void operator()(unused_type) const
        {
            phrase << str;
        }

        void operator()(unused_type, unused_type, unused_type) const
        {
            phrase << str;
        }

        collector& phrase;
        std::string str;
    };

    struct syntax_highlight
    {
        syntax_highlight(
            std::string const& source_mode
          , string_symbols const& macro
          , actions& escape_actions)
        : source_mode(source_mode)
        , macro(macro)
        , escape_actions(escape_actions)
        {
        }

        std::string operator()(iterator begin, iterator end) const;

        std::string const& source_mode;
        string_symbols const& macro;
        actions& escape_actions;
    };

    struct code_action
    {
        // Does the actual syntax highlighing of code

        code_action(
            collector& out
          , collector& phrase
          , syntax_highlight& syntax_p)
        : out(out)
        , phrase(phrase)
        , syntax_p(syntax_p)
        {
        }

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
        collector& phrase;
        syntax_highlight& syntax_p;
    };

    struct inline_code_action
    {
        // Does the actual syntax highlighing of code inlined in text

        inline_code_action(
            collector& out
          , syntax_highlight& syntax_p)
        : out(out)
        , syntax_p(syntax_p)
        {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
        syntax_highlight& syntax_p;
    };

    struct start_varlistitem_action
    {
        start_varlistitem_action(collector& phrase)
        : phrase(phrase) {}

        void operator()(unused_type, unused_type, unused_type) const;

        collector& phrase;
    };

    struct end_varlistitem_action
    {
        end_varlistitem_action(collector& phrase, collector& temp_para)
        : phrase(phrase), temp_para(temp_para) {}

        void operator()(unused_type, unused_type, unused_type) const;

        collector& phrase;
        collector& temp_para;
    };

    struct break_action
    {
        // Handles line-breaks (DEPRECATED!!!)

        break_action(collector& phrase)
        : phrase(phrase) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& phrase;
    };

    struct macro_identifier_action
    {
        // Handles macro identifiers

        macro_identifier_action(quickbook::actions& actions)
        : actions(actions) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        quickbook::actions& actions;
    };

    struct macro_definition_action
    {
        // Handles macro definitions

        macro_definition_action(quickbook::actions& actions)
        : actions(actions) {}

        void operator()(unused_type, unused_type, unused_type) const;

        quickbook::actions& actions;
    };

    struct template_body_action
    {
        // Handles template definitions

        template_body_action(quickbook::actions& actions)
        : actions(actions) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        quickbook::actions& actions;
    };

    struct do_template_action
    {
        template <typename Arg1, typename Arg2, typename Arg3, typename Arg4>
        struct result { typedef void type; };
        
        // Handles template substitutions

        do_template_action(quickbook::actions& actions)
        : actions(actions) {}

        void operator()(iterator, bool, template_symbol const&, std::vector<std::string>) const;

        quickbook::actions& actions;
    };

    struct variablelist_action
    {
        // Handles variable lists

        variablelist_action(quickbook::actions& actions)
        : actions(actions) {}

        template <typename T>
        struct result { typedef void type; };

        void operator()(std::string const&) const;

        quickbook::actions& actions;
    };

    struct table_action
    {
        // Handles tables

        table_action(quickbook::actions& actions)
        : actions(actions) {}

        template <typename Arg1, typename Arg2>
        struct result { typedef void type; };

        void operator()(boost::optional<std::string> const&, std::string const&) const;

        quickbook::actions& actions;
    };

    struct start_row_action
    {
        // Handles table rows

        start_row_action(collector& phrase, unsigned& span, std::string& header)
            : phrase(phrase), span(span), header(header) {}

        void operator()(unused_type, unused_type, unused_type) const;

        collector& phrase;
        unsigned& span;
        std::string& header;
    };

    struct start_col_action
    {
        // Handles table columns

        start_col_action(collector& phrase, unsigned& span)
        : phrase(phrase), span(span) {}

        void operator()(unused_type, unused_type, unused_type) const;

        collector& phrase;
        unsigned& span;
    };

    struct end_col_action
    {
        end_col_action(collector& phrase, collector& temp_para)
        : phrase(phrase), temp_para(temp_para) {}

        void operator()(unused_type, unused_type, unused_type) const;

        collector& phrase;
        collector& temp_para;
    };

    struct begin_section_action
    {    
        // Handles begin page

        begin_section_action(
            collector& out
          , collector& phrase
          , std::string& library_id
          , std::string& section_id
          , int& section_level
          , std::string& qualified_section_id)
        : out(out)
        , phrase(phrase)
        , library_id(library_id)
        , section_id(section_id)
        , section_level(section_level)
        , qualified_section_id(qualified_section_id) {}

        template <typename A1, typename A2>
        struct result { typedef void type; };

        void operator()(boost::optional<std::string> const&, iterator_range) const;

        collector& out;
        collector& phrase;
        std::string& library_id;
        std::string& section_id;
        int& section_level;
        std::string& qualified_section_id;
    };

    struct end_section_action
    {
        end_section_action(
            collector& out
          , int& section_level
          , std::string& qualified_section_id
          , int& error_count)
        : out(out)
        , section_level(section_level)
        , qualified_section_id(qualified_section_id)
        , error_count(error_count) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
        int& section_level;
        std::string& qualified_section_id;
        int& error_count;
   };
   
   struct element_id_warning_action
   {
       void operator()(iterator_range, unused_type, unused_type) const;
   };

    struct xinclude_action
    {
        // Handles XML includes
        xinclude_action(collector& out_, quickbook::actions& actions_)
            : out(out_), actions(actions_) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
        quickbook::actions& actions;
    };

    struct include_action
    {
        // Handles QBK includes

        include_action(quickbook::actions& actions_)
            : actions(actions_) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        quickbook::actions& actions;
    };

    struct import_action
    {
        // Handles import of source code files (e.g. *.cpp *.py)
        import_action(collector& out_, quickbook::actions& actions_)
            : out(out_), actions(actions_) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        collector& out;
        quickbook::actions& actions;
    };

    struct xml_author
    {
        // Handles xml author

        xml_author(collector& out)
        : out(out) {}

        void operator()(std::pair<std::string, std::string> const& author) const;

        collector& out;
    };

    struct xml_year
    {
        // Handles xml year

        xml_year(collector& out)
            : out(out) {}

        void operator()(std::string const &year) const;

        collector& out;
    };

    struct xml_copyright
    {
        // Handles xml copyright

        xml_copyright(collector& out)
            : out(out) {}

        void operator()(std::pair<std::vector<std::string>, std::string> const &copyright) const;

        collector& out;
    };

    void pre(collector& out, quickbook::actions& actions, bool ignore_docinfo = false);
    void post(collector& out, quickbook::actions& actions, bool ignore_docinfo = false);
    
    struct phrase_push_action
    {
        phrase_push_action(collector& phrase)
            : phrase(phrase) {}

        void operator()(unused_type, unused_type, unused_type) const;
        
        collector& phrase;
    };

    struct phrase_pop_action
    {
        phrase_pop_action(collector& phrase)
            : phrase(phrase) {}

        template <typename Context>
        void operator()(unused_type x1, Context& c, unused_type x2) const
        {
            boost::spirit::_val(x1, c, x2) = (*this)();
        }
        
        std::string operator()() const;
        
        collector& phrase;
    };

    struct phrase_to_string_action
    {
        phrase_to_string_action(std::string& out, collector& phrase)
            : out(out) , phrase(phrase) {}

        void operator()(unused_type, unused_type, unused_type) const;

        std::string& out;
        collector& phrase;
    };

    struct code_snippet_actions
    {
        code_snippet_actions(std::vector<template_symbol>& storage,
                                 std::string const& doc_id,
                                 char const* source_type)
            : storage(storage)
            , doc_id(doc_id)
            , source_type(source_type)
        {}

        void pass_thru(char);
        void escaped_comment(std::string const&);
        void compile(boost::iterator_range<iterator>);
        void callout(std::string const&, char const* role);
        void inline_callout(std::string const&);
        void line_callout(std::string const&);

        std::string code;
        std::string snippet;
        std::string id;
        std::vector<std::string> callouts;
        std::vector<template_symbol>& storage;
        std::string const doc_id;
        char const* const source_type;
    };
    
    struct process_action
    {
        process_action(quickbook::actions& actions)
            : actions(actions) {}
        
        typedef void result_type;

        template <typename Arg1, typename Arg2 = void, typename Arg3 = void>
        struct result { typedef void type; };

        template <typename Attrib, typename Context>
        void operator()(Attrib& a, Context& c, bool& pass) const {
            (*this)(a);
        }
        
        template <typename T>
        void operator()(boost::optional<T> const& x) const {
            if(x) (*this)(*x);
        }

        template <BOOST_VARIANT_ENUM_PARAMS(typename T)>
        void operator()(boost::variant<BOOST_VARIANT_ENUM_PARAMS(T)> const& x) const {
            return boost::apply_visitor(*this, x);
        }

        void operator()(unused_type) const {
        }

        template <typename T>
        void operator()(T const& x) const {
            process(actions, x);
        }
        
        quickbook::actions& actions;
    };
}

#ifdef BOOST_MSVC
#pragma warning (pop)
#endif

#endif // BOOST_SPIRIT_QUICKBOOK_ACTIONS_HPP

