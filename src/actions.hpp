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

#include <string>
#include <vector>
#include <boost/spirit/include/phoenix1_functions.hpp>
#include <boost/spirit/include/classic_symbols_fwd.hpp>
#include "fwd.hpp"
#include "template_stack.hpp"
#include "utils.hpp"
#include "values.hpp"
#include "scoped.hpp"
#include "symbols.hpp"

namespace quickbook
{
    namespace cl = boost::spirit::classic;

    extern unsigned qbk_version_n; // qbk_major_version * 100 + qbk_minor_version

    struct quickbook_range {
        template <typename Arg>
        struct result
        {
            typedef bool type;
        };
        
        quickbook_range(unsigned min_, unsigned max_)
            : min_(min_), max_(max_) {}
        
        bool operator()() const {
            return qbk_version_n >= min_ && qbk_version_n < max_;
        }

        unsigned min_, max_;
    };
    
    inline quickbook_range qbk_since(unsigned min_) {
        return quickbook_range(min_, 999);
    }
    
    inline quickbook_range qbk_before(unsigned max_) {
        return quickbook_range(0, max_);
    }

    // Throws load_error
    int load_snippets(fs::path const& file, std::vector<template_symbol>& storage,
        std::string const& extension, value::tag_type load_type);

    std::string syntax_highlight(
        parse_iterator first, parse_iterator last,
        actions& escape_actions,
        std::string const& source_mode);        

    struct error_message_action
    {
        // Prints an error message to std::cerr

        error_message_action(quickbook::actions& actions, std::string const& m)
            : actions(actions)
            , message(m)
        {}

        void operator()(parse_iterator, parse_iterator) const;

        quickbook::actions& actions;
        std::string message;
    };

    struct error_action
    {
        // Prints an error message to std::cerr

        error_action(quickbook::actions& actions)
        : actions(actions) {}

        void operator()(parse_iterator first, parse_iterator last) const;

        error_message_action operator()(std::string const& message)
        {
            return error_message_action(actions, message);
        }

        quickbook::actions& actions;
    };

    struct element_action
    {
        element_action(quickbook::actions& actions)
            : actions(actions) {}

        void operator()(parse_iterator, parse_iterator) const;

        quickbook::actions& actions;
    };

    struct paragraph_action
    {
        //  implicit paragraphs
        //  doesn't output the paragraph if it's only whitespace.

        paragraph_action(
            quickbook::actions& actions)
        : actions(actions) {}

        void operator()() const;
        void operator()(parse_iterator, parse_iterator) const { (*this)(); }

        quickbook::actions& actions;
    };

    struct phrase_end_action
    {
        phrase_end_action(quickbook::actions& actions) :
            actions(actions) {}

        void operator()() const;
        void operator()(parse_iterator, parse_iterator) const { (*this)(); }

        quickbook::actions& actions;
    };

    struct simple_phrase_action
    {
        //  Handles simple text formats

        simple_phrase_action(
            collector& out
          , quickbook::actions& actions)
        : out(out)
        , actions(actions) {}

        void operator()(char) const;

        collector& out;
        quickbook::actions& actions;
    };

    struct cond_phrase_push : scoped_action_base
    {
        cond_phrase_push(quickbook::actions& x)
            : actions(x) {}

        bool start();
        void cleanup();

        quickbook::actions& actions;
        bool saved_conditional;
        std::vector<std::string> anchors;
    };

    extern char const* quickbook_get_date;
    extern char const* quickbook_get_time;

    struct do_macro_action
    {
        // Handles macro substitutions

        do_macro_action(collector& phrase, quickbook::actions& actions)
            : phrase(phrase)
            , actions(actions) {}

        void operator()(std::string const& str) const;
        collector& phrase;
        quickbook::actions& actions;
    };

    struct space_action
    {
        // Prints a space

        space_action(collector& out)
            : out(out) {}

        void operator()(char ch) const;
        void operator()(parse_iterator first, parse_iterator last) const;

        collector& out;
    };

    struct plain_char_action
    {
        // Prints a single plain char.
        // Converts '<' to "&lt;"... etc See utils.hpp

        plain_char_action(collector& phrase, quickbook::actions& actions)
        : phrase(phrase)
        , actions(actions) {}

        void operator()(char ch) const;
        void operator()(parse_iterator first, parse_iterator last) const;

        collector& phrase;
        quickbook::actions& actions;
    };
    
    struct escape_unicode_action
    {
        escape_unicode_action(collector& phrase, quickbook::actions& actions)
        : phrase(phrase)
        , actions(actions) {}
        void operator()(parse_iterator first, parse_iterator last) const;

        collector& phrase;
        quickbook::actions& actions;
    };

    struct code_action
    {
        // Does the actual syntax highlighing of code

        code_action(
            collector& out
          , collector& phrase
          , quickbook::actions& actions)
        : out(out)
        , phrase(phrase)
        , actions(actions)
        {
        }

        void operator()(parse_iterator first, parse_iterator last) const;

        collector& out;
        collector& phrase;
        quickbook::actions& actions;
    };

    struct inline_code_action
    {
        // Does the actual syntax highlighing of code inlined in text

        inline_code_action(
            collector& out
          , quickbook::actions& actions)
        : out(out)
        , actions(actions)
        {}

        void operator()(parse_iterator first, parse_iterator last) const;

        collector& out;
        quickbook::actions& actions;
    };

    struct break_action
    {
        break_action(collector& phrase, quickbook::actions& actions)
        : phrase(phrase), actions(actions) {}

        void operator()(parse_iterator f, parse_iterator) const;

        collector& phrase;
        quickbook::actions& actions;
    };

   struct element_id_warning_action
   {
        element_id_warning_action(quickbook::actions& actions_)
            : actions(actions_) {}

        void operator()(parse_iterator first, parse_iterator last) const;

        quickbook::actions& actions;
   };

    enum docinfo_types
    {
        docinfo_ignore = 0,
        docinfo_main = 1,
        docinfo_nested = 2
    };

    void pre(collector& out, quickbook::actions& actions, value include_doc_id, docinfo_types);
    void post(collector& out, quickbook::actions& actions, docinfo_types);

    struct phrase_to_docinfo_action_impl
    {
        template <typename Arg1, typename Arg2, typename Arg3 = void>
        struct result { typedef void type; };
    
        phrase_to_docinfo_action_impl(quickbook::actions& actions)
            : actions(actions) {}

        void operator()(parse_iterator first, parse_iterator last) const;
        void operator()(parse_iterator first, parse_iterator last, value::tag_type) const;

        quickbook::actions& actions;
    };
    
    typedef phoenix::function<phrase_to_docinfo_action_impl> phrase_to_docinfo_action;

    struct to_value_action
    {
        to_value_action(quickbook::actions& actions)
            : actions(actions) {}

        void operator()(parse_iterator first, parse_iterator last) const;

        quickbook::actions& actions;
    };

    struct scoped_output_push : scoped_action_base
    {
        scoped_output_push(quickbook::actions& actions)
            : actions(actions) {}

        bool start();
        void cleanup();

        quickbook::actions& actions;
        std::vector<std::string> saved_anchors;
    };

    struct set_no_eols_scoped : scoped_action_base
    {
        set_no_eols_scoped(quickbook::actions& actions)
            : actions(actions) {}

        bool start();
        void cleanup();

        quickbook::actions& actions;
        bool saved_no_eols;
    };
}

#endif // BOOST_SPIRIT_QUICKBOOK_ACTIONS_HPP
