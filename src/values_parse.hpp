/*=============================================================================
    Copyright (c) 2010-2011 Daniel James

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(BOOST_SPIRIT_QUICKBOOK_VALUES_PARSE_HPP)
#define BOOST_SPIRIT_QUICKBOOK_VALUES_PARSE_HPP

#include "values.hpp"
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/phoenix1_functions.hpp>
#include <boost/spirit/include/phoenix1_primitives.hpp>

namespace quickbook {
    namespace cl = boost::spirit::classic;
    namespace ph = phoenix;

    struct value_builder_save
    {
        value_builder_save(value_builder& a) : a(a) { a.save(); }
        ~value_builder_save() { a.restore(); }
        value_builder& a;
    };

    template <typename ParserT>
    struct value_save_parser
        : public cl::unary< ParserT, cl::parser< value_save_parser<ParserT> > >
    {
        typedef value_save_parser<ParserT> self_t;
        typedef cl::unary< ParserT, cl::parser< value_save_parser<ParserT> > > base_t;

        template <typename ScannerT>
        struct result
        {
            typedef typename cl::parser_result<ParserT, ScannerT>::type type;
        };

        value_save_parser(value_builder& a, ParserT const &p)
            : base_t(p)
            , builder_(a)
        {}

        template <typename ScannerT>
        typename result<ScannerT>::type parse(ScannerT const &scan) const
        {
            typedef typename ScannerT::iterator_t iterator_t;
            iterator_t save = scan.first;

            value_builder_save saver(builder_);

            typename cl::parser_result<ParserT, ScannerT>::type result
                = this->subject().parse(scan);

            if (result) {
                return scan.create_match(result.length(), cl::nil_t(), save, scan.first);
            }
            else {
                return scan.no_match();
            }
        }
        
        value_builder& builder_;
    };

    struct value_save_gen
    {
        explicit value_save_gen(value_builder& b)
            : builder_(b) {}

        template<typename ParserT>
        value_save_parser<typename cl::as_parser<ParserT>::type>
        operator[](ParserT const& p) const
        {
            typedef typename cl::as_parser<ParserT> as_parser_t;
            typedef typename as_parser_t::type parser_t;
            
            return value_save_parser<parser_t>
                (builder_, as_parser_t::convert(p));
        }
        
        value_builder& builder_;
    };

    struct value_builder_list
    {
        value_builder_list(value_builder& a, value::tag_type t)
            : a(a), finished(false) { a.start_list(t); }
        void finish() { a.finish_list(); finished = true; }
        ~value_builder_list() { if(!finished) a.clear_list(); }
        value_builder& a;
        bool finished;
    };

    template <typename TagActor, typename ParserT>
    struct value_scoped_list_parser
        : public cl::unary< ParserT, cl::parser< value_scoped_list_parser<TagActor, ParserT> > >
    {
        typedef value_scoped_list_parser<TagActor, ParserT> self_t;
        typedef cl::unary< ParserT, cl::parser<self_t> > base_t;

        template <typename ScannerT>
        struct result
        {
            typedef typename cl::parser_result<ParserT, ScannerT>::type type;
        };

        value_scoped_list_parser(value_builder& a, TagActor k, ParserT const &p)
            : base_t(p)
            , builder_(a)
            , tag_(k)
        {}

        template <typename ScannerT>
        typename result<ScannerT>::type parse(ScannerT const &scan) const
        {
            typedef typename ScannerT::iterator_t iterator_t;
            iterator_t save = scan.first;
            value_builder_list list(builder_, tag_());
            typename cl::parser_result<ParserT, ScannerT>::type result
                = this->subject().parse(scan);

            if (result) {
                list.finish();
                return scan.create_match(result.length(), cl::nil_t(), save, scan.first);
            }
            else {
                return scan.no_match();
            }
        }
        
        value_builder& builder_;
        TagActor tag_;
    };

	template <typename TagActor>
	struct value_scoped_list_gen2
	{
        explicit value_scoped_list_gen2(value_builder& b, TagActor t)
            : b(b), tag_(t) {}

        template<typename ParserT>
        value_scoped_list_parser<TagActor, typename cl::as_parser<ParserT>::type>
        operator[](ParserT const& p) const
        {
            typedef typename cl::as_parser<ParserT> as_parser_t;
            typedef typename as_parser_t::type parser_t;
            
            return value_scoped_list_parser<TagActor, parser_t>
                (b, tag_, as_parser_t::convert(p));
        }
        
        value_builder& b;
        TagActor tag_;		
	};
	
    struct value_scoped_list_gen
    {
        template <typename Arg1> struct result { typedef void type; };
    	typedef void result_type;
    
        explicit value_scoped_list_gen(value_builder& b)
            : b(b) {}

        template <typename T>
        value_scoped_list_gen2<typename ph::as_actor<T>::type>
        	operator()(T const& tag) const
        {
            return value_scoped_list_gen2<typename ph::as_actor<T>::type>(
            		b, ph::as_actor<T>::convert(tag));
        }

        value_scoped_list_gen2<ph::as_actor<value::tag_type>::type>
        	operator()(value::tag_type tag = value::default_tag) const
        {
            return value_scoped_list_gen2<ph::as_actor<value::tag_type>::type>(
            		b, ph::as_actor<value::tag_type>::convert(tag));
        }

        value_builder& b;
    };

    struct value_entry
    {
        template <typename Arg1, typename Arg2 = void, typename Arg3 = void, typename Arg4 = void>
        struct result {
            typedef void type;
        };

        value_entry(value_builder& b)
            : b(b) {}

        template <typename Iterator>
        void operator()(Iterator begin, Iterator end,
                value::tag_type tag = value::default_tag) const
        {
            b.insert(qbk_value(begin, end, tag));
        }

        template <typename Iterator>
        void operator()(Iterator begin, Iterator,
                std::string const& v,
                value::tag_type tag = value::default_tag) const
        {
            b.insert(qbk_value(v, begin.get_position(), tag));
        }

        value_builder& b;
    };

    struct value_reset
    {
        typedef void result_type;
    
        value_reset(value_builder& b)
            : b(b) {}

        void operator()() const {
            b.reset();
        }

        value_builder& b;
    };
    
    struct value_sort
    {
        typedef void result_type;
    
        value_sort(value_builder& b)
            : b(b) {}

        void operator()() const {
            b.sort_list();
        }

        value_builder& b;
    };

    struct value_parser
    {
        value_parser()
            : builder()
            , save(builder)
            , list(builder)
            , entry(builder)
            , reset(builder)
            , sort(builder)
            {}
    
        value get() { return builder.get(); }

        value_builder builder;
        value_save_gen save;
        value_scoped_list_gen list;
        ph::function<value_entry> entry;
        ph::function<value_reset> reset;
        ph::function<value_sort> sort;
    };
}

#endif
