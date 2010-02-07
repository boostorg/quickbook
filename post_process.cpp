/*=============================================================================
    Copyright (c) 2005 2006 Joel de Guzman
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include "post_process.hpp"
#include "utils.hpp"
#include "parse_utils.hpp"
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_lexeme.hpp>
#include <boost/spirit/include/qi_char_.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <set>
#include <stack>
#include <cctype>

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;
    typedef std::string::const_iterator iter_type;

    struct printer
    {
        printer(std::string& out, int& current_indent, int linewidth)
            : prev(0), out(out), current_indent(current_indent) , column(0)
            , in_string(false), linewidth(linewidth) {}

        void indent()
        {
            BOOST_ASSERT(current_indent >= 0); // this should not happen!
            for (int i = 0; i < current_indent; ++i)
                out += ' ';
            column = current_indent;
        }

        void break_line()
        {
            out.erase(out.find_last_not_of(' ')+1); // trim trailing spaces
            out += '\n';
            indent();
        }

        bool line_is_empty() const
        {
            for (iter_type i = out.end()-(column-current_indent); i != out.end(); ++i)
            {
                if (*i != ' ')
                    return false;
            }
            return true;
        }

        void align_indent()
        {
            // make sure we are at the proper indent position
            if (column != current_indent)
            {
                if (column > current_indent)
                {
                    if (line_is_empty())
                    {
                        // trim just enough trailing spaces down to current_indent position
                        out.erase(out.end()-(column-current_indent), out.end());
                        column = current_indent;
                    }
                    else
                    {
                        // nope, line is not empty. do a hard CR
                        break_line();
                    }
                }
                else
                {
                    // will this happen? (i.e. column <= current_indent)
                    while (column != current_indent)
                    {
                        out += ' ';
                        ++column;
                    }
                }
            }
        }

        void print(char ch)
        {
            // Print a char. Attempt to break the line if we are exceeding
            // the target linewidth. The linewidth is not an absolute limit.
            // There are many cases where a line will exceed the linewidth
            // and there is no way to properly break the line. Preformatted
            // code that exceeds the linewidth are examples. We cannot break
            // preformatted code. We shall not attempt to be very strict with
            // line breaking. What's more important is to have a reproducable
            // output (i.e. processing two logically equivalent xml files
            // results in two lexically equivalent xml files). *** pretty
            // formatting is a secondary goal ***

            // Strings will occur only in tag attributes. Normal content
            // will have &quot; instead. We shall deal only with tag
            // attributes here.
            if (ch == '"')
                in_string = !in_string; // don't break strings!

            if (!in_string && std::isspace(static_cast<unsigned char>(ch)))
            {
                // we can break spaces if they are not inside strings
                if (!std::isspace(static_cast<unsigned char>(prev)))
                {
                    if (column >= linewidth)
                    {
                        break_line();
                        if (column == 0 && ch == ' ')
                        {
                            ++column;
                            out += ' ';
                        }
                    }
                    else
                    {
                        ++column;
                        out += ' ';
                    }
                }
            }
            else
            {
                // we can break tag boundaries and stuff after
                // delimiters if they are not inside strings
                // and *only-if* the preceding char is a space
                if (!in_string
                    && column >= linewidth
                    && (ch == '<' && std::isspace(static_cast<unsigned char>(prev))))
                    break_line();
                out += ch;
                ++column;
            }

            prev = ch;
        }

        void
        print(iter_type f, iter_type l)
        {
            for (iter_type i = f; i != l; ++i)
                 print(*i);
        }

        void
        print(std::string const& x)
        {
            print(x.begin(), x.end());
        }

        void
        print_tag(std::string const& str, bool is_flow_tag)
        {
            if (is_flow_tag)
            {
                print(str);
            }
            else
            {
                // This is not a flow tag, so, we're going to do a
                // carriage return anyway. Let us remove extra right
                // spaces.
                BOOST_ASSERT(!str.empty()); // this should not happen
                iter_type i = str.end();
                while (i != str.begin() && std::isspace(static_cast<unsigned char>(*(i-1))))
                    --i;
                print(str.begin(), i);
            }
        }

        char prev;
        std::string& out;
        int& current_indent;
        int column;
        bool in_string;
        int linewidth;
    };

    char const* block_tags_[] =
    {
          "author"
        , "blockquote"
        , "bridgehead"
        , "callout"
        , "calloutlist"
        , "caution"
        , "copyright"
        , "entry"
        , "footnote"
        , "important"
        , "informaltable"
        , "itemizedlist"
        , "legalnotice"
        , "listitem"
        , "note"
        , "orderedlist"
        , "para"
        , "row"
        , "section"
        , "table"
        , "tbody"
        , "textobject"
        , "tgroup"
        , "thead"
        , "tip"
        , "variablelist"
        , "varlistentry"
        , "warning"
        , "xml"
        , "xi:include"
    };

    char const* doc_types_[] =
    {
          "book"
        , "article"
        , "library"
        , "chapter"
        , "part"
        , "appendix"
        , "preface"
        , "qandadiv"
        , "qandaset"
        , "reference"
        , "set"
    };

    struct tidy_compiler
    {
        tidy_compiler(std::string& out, int linewidth)
            : out(out), current_indent(0), printer_(out, current_indent, linewidth)
        {
            static int const n_block_tags = sizeof(block_tags_)/sizeof(char const*);
            for (int i = 0; i != n_block_tags; ++i)
            {
                block_tags.insert(block_tags_[i]);
            }

            static int const n_doc_types = sizeof(doc_types_)/sizeof(char const*);
            for (int i = 0; i != n_doc_types; ++i)
            {
                block_tags.insert(doc_types_[i]);
                block_tags.insert(doc_types_[i] + std::string("info"));
                block_tags.insert(doc_types_[i] + std::string("purpose"));
            }
        }

        bool is_flow_tag(std::string const& tag)
        {
            return block_tags.find(tag) == block_tags.end();
        }

        std::set<std::string> block_tags;
        std::stack<std::string> tags;
        std::string& out;
        int current_indent;
        printer printer_;
        std::string current_tag;
    };

    template <typename Iterator>
    struct tidy_grammar : qi::grammar<Iterator>
    {
        typedef boost::iterator_range<Iterator> iterator_range;
    
        tidy_grammar(tidy_compiler& state, int indent)
            : tidy_grammar::base_type(tidy)
            , state(state), indent(indent)
        {
            tag =
                qi::lexeme[qi::raw[
                    +(qi::alpha | qi::char_("_:"))
                ]]
               [ph::bind(&tidy_grammar::do_tag, this, qi::_1)];

            code = qi::raw[
                    "<programlisting>"
                >>  *(qi::char_ - "</programlisting>")
                >>  "</programlisting>"
                ];

            // What's the business of lexeme_d['>' >> *space]; ?
            // It is there to preserve the space after the tag that is
            // otherwise consumed by the space skipper.

            escape =
                (    "<!--quickbook-escape-prefix-->"
                >>  qi::raw[*(qi::char_ - "<!--quickbook-escape-postfix-->")]
                >>  qi::lexeme[
                        "<!--quickbook-escape-postfix-->"
                    >>  qi::raw[*qi::space]
                    ]
                )   [ph::bind(&tidy_grammar::do_escape, this, qi::_1, qi::_2)]
                ;

            start_tag = qi::raw['<' >> tag >> *(qi::char_ - '>') >> qi::lexeme['>' >> *qi::space]];
            start_end_tag = qi::raw[
                    '<' >> tag >> *(qi::char_ - ("/>" | qi::lit('>'))) >> qi::lexeme["/>" >> *qi::space]
                |   "<?" >> tag >> *(qi::char_ - '?') >> qi::lexeme["?>" >> *qi::space]
                |   "<!--" >> *(qi::char_ - "-->") >> qi::lexeme["-->" >> *qi::space]
                |   "<!" >> tag >> *(qi::char_ - '>') >> qi::lexeme['>' >> *qi::space]
                ];
            content = qi::lexeme[ +(qi::char_ - '<') ];
            end_tag = qi::raw["</" >> +(qi::char_ - '>') >> qi::lexeme['>' >> *qi::space]];

            markup =
                    escape
                |   code            [ph::bind(&tidy_grammar::do_code, this, qi::_1)]
                |   start_end_tag   [ph::bind(&tidy_grammar::do_start_end_tag, this, qi::_1)]
                |   start_tag       [ph::bind(&tidy_grammar::do_start_tag, this, qi::_1)]
                |   end_tag         [ph::bind(&tidy_grammar::do_end_tag, this, qi::_1)]
                |   content         [ph::bind(&tidy_grammar::do_content, this, qi::_1)]
                ;

            tidy = +markup;
        }

        void do_escape(iterator_range x, iterator_range post) const
        {
            // Trim spaces from contents and append
            Iterator f = x.begin(), l = x.end();
            while (f != l && std::isspace(*f))
                ++f;
            while (f != l && std::isspace(*(l - 1)))
                --l;
            state.out.append(f, l);

            // Append spaces trailing the closing tag.
            state.out.append(post.begin(), post.end());
        }

        void do_code(std::string const& x) const
        {
            state.out += '\n';
            // print the string taking care of line
            // ending CR/LF platform issues
            for (iter_type i = x.begin(), l = x.end(); i != l; ++i)
            {
                if (*i == '\n')
                {
                    state.out += '\n';
                    ++i;
                    if (i != l && *i != '\r')
                        state.out += *i;
                }
                else if (*i == '\r')
                {
                    state.out += '\n';
                    ++i;
                    if (i != l && *i != '\n')
                        state.out += *i;
                }
                else
                {
                    state.out += *i;
                }
            }
            state.out += '\n';
            state.printer_.indent();
        }

        void do_tag(iterator_range x) const
        {
            state.current_tag = std::string(x.begin(), x.end());
        }

        void do_start_end_tag(std::string const& x) const
        {
            bool is_flow_tag = state.is_flow_tag(state.current_tag);
            if (!is_flow_tag)
                state.printer_.align_indent();
            state.printer_.print_tag(x, is_flow_tag);
            if (!is_flow_tag)
                state.printer_.break_line();
        }

        void do_start_tag(std::string const& x) const
        {
            state.tags.push(state.current_tag);
            bool is_flow_tag = state.is_flow_tag(state.current_tag);
            if (!is_flow_tag)
                state.printer_.align_indent();
            state.printer_.print_tag(x, is_flow_tag);
            if (!is_flow_tag)
            {
                state.current_indent += indent;
                state.printer_.break_line();
            }
        }

        void do_content(std::string const& x) const
        {
            state.printer_.print(x);
        }

        void do_end_tag(std::string const& x) const
        {
            bool is_flow_tag = state.is_flow_tag(state.tags.top());
            if (!is_flow_tag)
            {
                state.current_indent -= indent;
                state.printer_.align_indent();
            }
            state.printer_.print_tag(x, is_flow_tag);
            if (!is_flow_tag)
                state.printer_.break_line();
            state.tags.pop();
        }

        tidy_compiler& state;
        int indent;

        qi::rule<Iterator>  tidy, tag,
                            markup, escape;
        qi::rule<Iterator, std::string()>
                            start_tag, start_end_tag,
                            content, end_tag, code;
    };

    int post_process(
        std::string const& in
      , std::ostream& out
      , int indent
      , int linewidth)
    {
        if (indent == -1)
            indent = 2;         // set default to 2
        if (linewidth == -1)
            linewidth = 80;     // set default to 80

        try
        {
            std::string tidy;
            tidy_compiler state(tidy, linewidth);
            tidy_grammar<iter_type> g(state, indent);
            iter_type first = in.begin(), last = in.end();
            bool r = parse(first, last, g, qi::space);
            if (r && first == last)
            {
                out << tidy;
                return 0;
            }
            else
            {
                // fallback!
                ::quickbook::detail::outerr("")
                    << "Warning: Post Processing Failed."
                    << std::endl;
                out << in;
                return 1;
            }
        }

        catch(...)
        {
            // fallback!
            ::quickbook::detail::outerr("")
                << "Post Processing Failed."
                << std::endl;
            out << in;
            return 1;
        }
    }
}

