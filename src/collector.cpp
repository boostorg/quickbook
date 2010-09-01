/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/assert.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include "collector.hpp"

namespace quickbook
{
    typedef boost::iostreams::filtering_ostream filtering_ostream;

    string_stream::string_stream()
        : buffer_ptr(new std::string())
        , stream_ptr(new filtering_ostream(
            boost::iostreams::back_inserter(*buffer_ptr.get())))
    {}

    string_stream::string_stream(string_stream const& other)
        : buffer_ptr(other.buffer_ptr)
        , stream_ptr(other.stream_ptr)
    {}
    
    string_stream&
    string_stream::operator=(string_stream const& other)
    {
        buffer_ptr = other.buffer_ptr;
        stream_ptr = other.stream_ptr;
        return *this;
    }

    collector::collector()
        : main(default_)
        , top(default_)
    {
    }

    collector::collector(string_stream& out)
        : main(out) 
        , top(out) 
    {
    }
    
    collector::~collector()
    {
        BOOST_ASSERT(streams.empty()); // assert there are no more pushes than pops!!!
    }
    
    void 
    collector::push()
    {
        streams.push(string_stream());
        top = boost::ref(streams.top());
    }
    
    void 
    collector::pop()
    {
        BOOST_ASSERT(!streams.empty());
        streams.pop();

        if (streams.empty())
            top = boost::ref(main);
        else
            top = boost::ref(streams.top());
    }
}
