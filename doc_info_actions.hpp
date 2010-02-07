/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(BOOST_SPIRIT_QUICKBOOK_DOC_INFO_ACTIONS_HPP)
#define BOOST_SPIRIT_QUICKBOOK_DOC_INFO_ACTIONS_HPP

#include "fwd.hpp"
#include "doc_info.hpp"

namespace quickbook
{
    doc_info process(quickbook::actions&, doc_info const&);
}

#endif
