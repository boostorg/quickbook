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

namespace quickbook
{
    quickbook_grammar::quickbook_grammar(quickbook::actions& a)
        : impl_(new impl(a))
        , command_line_macro(impl_->command_line_macro, "command_line_macro")
        , phrase(impl_->common, "phrase")
        , simple_phrase(impl_->simple_phrase, "simple_phrase")
        , block(impl_->block_start, "block")
        , doc_info(impl_->doc_info_details, "doc_info")
    {
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
        init_block_template();
        init_doc_info();
    }
}
