/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "grammar_impl.hpp"
#include "utils.hpp"
#include "actions.hpp"
#include "state.hpp"
#include <boost/spirit/include/qi_nonterminal.hpp>

namespace quickbook
{
    struct error_handler
    {
        error_handler(quickbook::actions& a)
            : actions_(a) {}
    
        void operator()(
            boost::fusion::vector<
                iterator&,
                iterator const&,
                iterator const&,
                boost::spirit::info const&> args,
            boost::spirit::unused_type,
            qi::error_handler_result& r) const
        {   
            iterator& first = boost::fusion::at_c<0>(args);
            iterator end = boost::fusion::at_c<1>(args);
            iterator pos = boost::fusion::at_c<2>(args);
            boost::spirit::info const& error = boost::fusion::at_c<3>(args);

            detail::outerr(pos.get_position().file, pos.get_position().line)
                << "Syntax error near column "
                << pos.get_position().column
                << ". Expecting "
                << error
                << "."
                << std::endl;
            
            ++actions_.state_.error_count;
            
            // Pretend we've successfully parsed the whole content since we've
            // already complained about the parse error and don't want to
            // complain again.
            r = qi::accept;
            first = end;
        }
        
        quickbook::actions& actions_;
    };

   quickbook_grammar::quickbook_grammar(quickbook::actions& a)
        : impl_(new impl(a))
        , command_line_macro(impl_->command_line_macro, "command_line_macro")
        , phrase(impl_->common, "phrase")
        , simple_phrase(impl_->simple_phrase, "simple_phrase")
        , block(impl_->block_start, "block")
        , doc_info(impl_->doc_info_details, "doc_info")
    {
        qi::on_error(impl_->block_start, error_handler(a));
    }
    
    quickbook_grammar::~quickbook_grammar()
    {
    }

    quickbook_grammar::impl::impl(quickbook::actions& a)
        : actions(a)
        , no_eols(true)
        , store_()
    {
        init_phrase();
        init_phrase_markup();
        init_phrase_image();
        init_block();
        init_block_markup();
        init_block_section();
        init_block_table();
        init_template();
        init_code();
        init_doc_info();
    }
}
