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

#include <map>
#include <string>
#include <vector>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include "fwd.hpp"

#ifdef BOOST_MSVC
// disable copy/assignment could not be generated, unreferenced formal params
#pragma warning (push)
#pragma warning(disable : 4511 4512 4100)
#endif

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    using boost::spirit::unused_type;

    extern unsigned qbk_major_version;
    extern unsigned qbk_minor_version;
    extern unsigned qbk_version_n; // qbk_major_version * 100 + qbk_minor_version

    struct quickbook_since_impl {
        template <typename Arg1>
        struct result { typedef bool type; };
        
        bool operator()(unsigned min_) const {
            return qbk_version_n >= min_;
        }
    };

    struct quickbook_before_impl {
        template <typename Arg1>
        struct result { typedef bool type; };
        
        bool operator()(unsigned max_) const {
            return qbk_version_n < max_;
        }
    };

    namespace {
        boost::phoenix::function<quickbook_since_impl> qbk_since;
        boost::phoenix::function<quickbook_before_impl> qbk_before;
    }

    // TODO: Define this elsewhere?

    struct macro {
        macro() {}
        explicit macro(char const* x) : raw_markup(x) {};
        explicit macro(std::string const& x) : raw_markup(x) {};

        std::string raw_markup;
    };

    typedef qi::symbols<char, macro> macro_symbols;

    typedef boost::iterator_range<iterator> iterator_range;

    struct error_action
    {
        // Prints an error message to std::cerr

        error_action(
            int& error_count)
        : error_count(error_count) {}

        void operator()(iterator_range, unused_type, unused_type) const;

        int& error_count;
    };

    extern char const* quickbook_get_date;
    extern char const* quickbook_get_time;

    struct element_id_warning_action
    {
        void operator()(iterator_range, unused_type, unused_type) const;
    };

    struct phrase_push_action
    {
        phrase_push_action(collector& phrase)
            : phrase(phrase) {}

        void operator()(unused_type, unused_type, unused_type) const {
            return (*this)();
        }

        void operator()() const;
        
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
        void operator()(T const& x) const;
        
        quickbook::actions& actions;
    };

    ///////////////////////////////////////////////////////////////////////////
    // actions
    ///////////////////////////////////////////////////////////////////////////

    struct actions
    {
        actions(state&);
    
        state&                      state_;
        template_stack&             templates;
        macro_symbols&              macro;

        process_action              process;
        phrase_push_action          phrase_push;
        phrase_pop_action           phrase_pop;
        error_action                error;
        element_id_warning_action   element_id_warning;
    };
}

#ifdef BOOST_MSVC
#pragma warning (pop)
#endif

#endif // BOOST_SPIRIT_QUICKBOOK_ACTIONS_HPP

