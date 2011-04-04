/*=============================================================================
    Copyright (c) 2001-2003 Joel de Guzman
    Copyright (c) 2011 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#ifndef QUICKBOOK_SYMBOLS_IPP
#define QUICKBOOK_SYMBOLS_IPP

///////////////////////////////////////////////////////////////////////////////

#include <boost/spirit/home/classic/symbols.hpp>
#include <boost/spirit/home/classic/core/assert.hpp>
#include <boost/intrusive_ptr.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace quickbook
{

///////////////////////////////////////////////////////////////////////////////
//
//  tst class
//
//      This it the Ternary Search Tree from
//      <boost/spirit/home/classic/symbols/impl/tst.ipp> adapted to be cheap
//      to copy.
//
//      Ternary Search Tree implementation. The data structure is faster than
//      hashing for many typical search problems especially when the search
//      interface is iterator based. Searching for a string of length k in a
//      ternary search tree with n strings will require at most O(log n+k)
//      character comparisons. TSTs are many times faster than hash tables
//      for unsuccessful searches since mismatches are discovered earlier
//      after examining only a few characters. Hash tables always examine an
//      entire key when searching.
//
//      For details see http://www.cs.princeton.edu/~rs/strings/.
//
//      *** This is a low level class and is
//          not meant for public consumption ***
//
///////////////////////////////////////////////////////////////////////////////

    template <typename T, typename CharT>
    struct tst_node
    {
        tst_node(CharT value_)
        : reference_count(0)
        , value(value_)
        , data(0)
        {
        }
        
        tst_node(tst_node const& other)
        : reference_count(0)
        , value(other.value)
        , left(other.left)
        , middle(other.middle)
        , right(other.right)
        , data(0)
        {
            if (other.data)
                data = new T(*other.data);
        }
        
        tst_node& operator=(tst_node other)
        {
            swap(other);
            return *this;
        }

        ~tst_node()
        {
            delete data;
        }
        
        void swap(tst_node& x, tst_node& y)
        {
            boost::swap(x.reference_count, y.reference_count);
            boost::swap(x.value, y.value);
            boost::swap(x.left, y.left);
            boost::swap(x.middle, y.middle);
            boost::swap(x.right, y.right);
            boost::swap(x.data, y.data);
        }

        short reference_count;
        CharT value;
        boost::intrusive_ptr<tst_node> left;
        boost::intrusive_ptr<tst_node> middle;
        boost::intrusive_ptr<tst_node> right;
        T* data;
    };

    template <typename T, typename CharT>
    void intrusive_ptr_add_ref(tst_node<T, CharT>* ptr)
        { ++ptr->reference_count; }

    template <typename T, typename CharT>
    void intrusive_ptr_release(tst_node<T, CharT>* ptr)
        { if(--ptr->reference_count == 0) delete ptr; }


    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename CharT>
    class tst
    {
    public:

        struct search_info
        {
            T*          data;
            std::size_t length;
        };

        void swap(tst& other)
        {
            root.swap(other.root);
        }

        template <typename IteratorT>
        T* add(IteratorT first, IteratorT const& last, T const& data)
        {
            if (first == last)
                return 0;

            node_ptr* np = &root;
            CharT ch = *first;

            BOOST_SPIRIT_ASSERT((first == last || ch != 0)
                && "Won't add string containing null character");

            while(ch != CharT(0))
            {
                if (!*np)
                {
                    *np = new node_t(ch);
                }
                else if ((*np)->reference_count > 1)
                {
                    *np = new node_t(**np);
                }

                if (ch < (*np)->value)
                {
                    np = &(*np)->left;
                }
                else if (ch == (*np)->value)
                {
                    ++first;
                    ch = (first == last) ? CharT(0) : *first;
                    BOOST_SPIRIT_ASSERT((first == last || ch != 0)
                        && "Won't add string containing null character");
                    np = &(*np)->middle;
                }
                else
                {
                    np = &(*np)->right;
                }
            }

            if (*np && (*np)->value == CharT(0))
            {
                node_ptr new_node = new node_t(ch);
                new_node->left = (*np)->left;
                new_node->right = (*np)->right;
                *np = new_node;
            }
            else
            {
                node_ptr new_node = new node_t(ch);
                new_node->right = *np;
                *np = new_node;
            }

            (*np)->data = new T(data);
            return (*np)->data;
        }
        
        template <typename ScannerT>
        search_info find(ScannerT const& scan) const
        {
            search_info result = { 0, 0 };
            if (scan.at_end()) {
                return result;
            }

            typedef typename ScannerT::iterator_t iterator_t;
            node_ptr    np = root;
            CharT       ch = *scan;
            iterator_t  save = scan.first;
            iterator_t  latest = scan.first;
            std::size_t latest_len = 0;

            while (np)
            {

                if (ch < np->value) // => go left!
                {
                    if (np->value == 0)
                    {
                        result.data = np->data;
                        if (result.data)
                        {
                            latest = scan.first;
                            latest_len = result.length;
                        }
                    }

                    np = np->left;
                }
                else if (ch == np->value) // => go middle!
                {
                    // Matching the null character is not allowed.
                    if (np->value == 0)
                    {
                        result.data = np->data;
                        if (result.data)
                        {
                            latest = scan.first;
                            latest_len = result.length;
                        }
                        break;
                    }

                    ++scan;
                    ch = scan.at_end() ? CharT(0) : *scan;
                    np = np->middle;
                    ++result.length;
                }
                else // (ch > np->value) => go right!
                {
                    if (np->value == 0)
                    {
                        result.data = np->data;
                        if (result.data)
                        {
                            latest = scan.first;
                            latest_len = result.length;
                        }
                    }

                    np = np->right;
                }
            }

            if (result.data == 0)
            {
                scan.first = save;
            }
            else
            {
                scan.first = latest;
                result.length = latest_len;
            }
            return result;
        }

    private:

        typedef tst_node<T, CharT> node_t;
        typedef boost::intrusive_ptr<node_t> node_ptr;
        node_ptr root;
    };

    typedef boost::spirit::classic::symbols<
        std::string,
        char,
        quickbook::tst<std::string, char>
    > string_symbols;
} // namespace quickbook

#endif
