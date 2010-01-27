/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_QUICKBOOK_SYNTAX_HIGHLIGHT_HPP)
#define BOOST_SPIRIT_QUICKBOOK_SYNTAX_HIGHLIGHT_HPP

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_directive.hpp>
#include "./grammars.hpp"
#include "./phrase.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    using boost::spirit::unused_type;

    struct parse_escaped_impl
    {
        parse_escaped_impl(quickbook::actions& escape_actions)
            : actions(escape_actions) {}

        void operator()(boost::iterator_range<iterator> escaped, unused_type, unused_type) const {
            bool unused;
            phrase_grammar common(actions, unused);   
            iterator first = escaped.begin(), last = escaped.end();
            while(first != last) {
                if(!qi::parse(first, last, common)) {
                    actions.plain_char(*first, 0, 0);
                    ++first;
                }
            }
        }
        
        quickbook::actions& actions;
    };

    // Grammar for C++ highlighting
    template <
        typename Process
      , typename Space
      , typename Unexpected
      , typename Out>
    struct cpp_highlight
    : public qi::grammar<iterator>
    {
        cpp_highlight(Out& out, quickbook::actions& escape_actions)
        : cpp_highlight::base_type(program), out(out), escape_actions(escape_actions)
        , parse_escaped(escape_actions)
        {
            program
                =
                *(  qi::raw[+qi::space]     [Space(out)]
                |   macro
                |   escape
                |   qi::raw[preprocessor]   [Process("preprocessor", out)]
                |   qi::raw[comment]        [Process("comment", out)]
                |   qi::raw[keyword]        [Process("keyword", out)]
                |   qi::raw[identifier]     [Process("identifier", out)]
                |   qi::raw[special]        [Process("special", out)]
                |   qi::raw[string_]        [Process("string", out)]
                |   qi::raw[char_]          [Process("char", out)]
                |   qi::raw[number]         [Process("number", out)]
                |   qi::raw[qi::char_]      [Unexpected(out)]
                )
                ;

            macro =
                (   escape_actions.macro                // must not be followed by
                >>  &(qi::eps - (qi::alpha | '_'))      // alpha or underscore
                )                                       [escape_actions.process]
                ;

            escape =
                "``" >> (
                    (qi::raw[+(qi::char_ - "``")] >> "``")
                                                        [parse_escaped]
                    | qi::raw[*qi::char_]               [escape_actions.error]
                )
                ;

            preprocessor
                =   '#' >> *qi::space >> ((qi::alpha | '_') >> *(qi::alnum | '_'))
                ;

            comment
                =   qi::lit("//") >> *(qi::char_ - qi::eol) >> -qi::eol
                |   qi::lit("/*") >> *(qi::char_ - "*/") >> -qi::lit("*/")
                ;

            keyword
                =   keyword_ >> (qi::eps - (qi::alnum | '_'))
                ;   // make sure we recognize whole words only

            keyword_
                =   "and_eq", "and", "asm", "auto", "bitand", "bitor",
                    "bool", "break", "case", "catch", "char", "class",
                    "compl", "const_cast", "const", "continue", "default",
                    "delete", "do", "double", "dynamic_cast",  "else",
                    "enum", "explicit", "export", "extern", "false",
                    "float", "for", "friend", "goto", "if", "inline",
                    "int", "long", "mutable", "namespace", "new", "not_eq",
                    "not", "operator", "or_eq", "or", "private",
                    "protected", "public", "register", "reinterpret_cast",
                    "return", "short", "signed", "sizeof", "static",
                    "static_cast", "struct", "switch", "template", "this",
                    "throw", "true", "try", "typedef", "typeid",
                    "typename", "union", "unsigned", "using", "virtual",
                    "void", "volatile", "wchar_t", "while", "xor_eq", "xor"
                ;

            special
                =   +qi::char_("~!%^&*()+={[}]:;,<.>?/|\\-")
                ;

            string_char = ('\\' >> qi::char_) | (qi::char_ - '\\');

            string_
                =   -qi::no_case['l'] >> '"' >> *(string_char - '"') >> -qi::lit('"');
                ;

            char_
                =   -qi::no_case['l'] >> '\'' >> *(string_char - '\'') >> -qi::lit('\'');
                ;

            number
                =   (
                        qi::no_case["0x"] >> qi::hex
                    |   '0' >> qi::oct
                    |   qi::long_double
                    )
                    >>  *qi::no_case[qi::char_("ldfu")]
                ;

            identifier
                =   (qi::alpha | '_') >> *(qi::alnum | '_')
                ;
        }

        qi::rule<iterator>
                        program, macro, preprocessor, comment, special, string_, 
                        char_, number, identifier, keyword, escape,
                        string_char;

        Out& out;
        quickbook::actions& escape_actions;

        qi::symbols<> keyword_;
        parse_escaped_impl parse_escaped;
        std::string save;
    };

    // Grammar for Python highlighting
    // See also: The Python Reference Manual
    // http://docs.python.org/ref/ref.html
    template <
        typename Process
      , typename Space
      , typename Unexpected
      , typename Out>
    struct python_highlight
    : public qi::grammar<iterator>
    {
        python_highlight(Out& out, quickbook::actions& escape_actions)
        : python_highlight::base_type(program), out(out), escape_actions(escape_actions)
        , parse_escaped(escape_actions)
        {
            program
                =
                *(  qi::raw[+qi::space]     [Space(out)]
                |   macro
                |   escape          
                |   qi::raw[comment]        [Process("comment", out)]
                |   qi::raw[keyword]        [Process("keyword", out)]
                |   qi::raw[identifier]     [Process("identifier", out)]
                |   qi::raw[special]        [Process("special", out)]
                |   qi::raw[string_]        [Process("string", out)]
                |   qi::raw[number]         [Process("number", out)]
                |   qi::raw[qi::char_]      [Unexpected(out)]
                )
                ;

            macro =
                (   escape_actions.macro                // must not be followed by
                >>  &(qi::eps - (qi::alpha | '_'))      // alpha or underscore
                )                                       [escape_actions.process]
                ;

            escape =
                "``" >> (
                    (qi::raw[+(qi::char_ - "``")] >> "``")
                                                        [parse_escaped]
                    | qi::raw[*qi::char_]               [escape_actions.error]
                )
                ;

            comment
                = qi::lit('#') >> *(qi::char_ - qi::eol) >> -qi::eol;
                ;

            keyword
                =   keyword_ >> (qi::eps - (qi::alnum | '_'))
                ;   // make sure we recognize whole words only

            keyword_
                =
                "and",       "del",       "for",       "is",        "raise",    
                "assert",    "elif",      "from",      "lambda",    "return",   
                "break",     "else",      "global",    "not",       "try",  
                "class",     "except",    "if",        "or",        "while",    
                "continue",  "exec",      "import",    "pass",      "yield",   
                "def",       "finally",   "in",        "print",

                // Technically "as" and "None" are not yet keywords (at Python
                // 2.4). They are destined to become keywords, and we treat them 
                // as such for syntax highlighting purposes.
                
                "as", "None"
                ;

            special
                =   +qi::char_("~!%^&*()+={[}]:;,<.>/|\\-")
                ;

            string_prefix
                =    qi::no_case[qi::string("u") >> - qi::string("r")]
                ;
            
            string_
                =   - string_prefix >> (long_string | short_string)
                ;

            string_char = ('\\' >> qi::char_) | (qi::char_ - '\\');
        
            short_string
                =   qi::lit('\'') >> *(string_char - '\'') >> -qi::lit('\'') |
                    qi::lit('"') >> *(string_char - '"') >> -qi::lit('"')
                ;
        
            long_string
                =   qi::lit("'''") >> *(string_char - "'''") >> -qi::lit("'''") |
                    qi::lit("\"\"\"") >> *(string_char - "\"\"\"") >> -qi::lit("\"\"\"")
                ;
            
            number
                =   (
                        qi::no_case["0x"] >> qi::hex
                    |   '0' >> qi::oct
                    |   qi::long_double
                    )
                    >>  *qi::no_case[qi::char_("lj")]
                ;

            identifier
                =   (qi::alpha | '_') >> *(qi::alnum | '_')
                ;
        }

        qi::rule<iterator>
                        program, macro, comment, special, string_, string_prefix, 
                        short_string, long_string, number, identifier, keyword, 
                        escape, string_char;

        Out& out;
        quickbook::actions& escape_actions;

        qi::symbols<> keyword_;
        parse_escaped_impl parse_escaped;
        std::string save;
    };

    // Grammar for plain text (no actual highlighting)
    template <
        typename CharProcess
      , typename Out>
    struct teletype_highlight
    : public qi::grammar<iterator>
    {
        teletype_highlight(Out& out, quickbook::actions& escape_actions)
        : teletype_highlight::base_type(program), out(out), escape_actions(escape_actions)
        , parse_escaped(escape_actions)
        {
            program
                =
                *(  macro
                |   escape          
                |   qi::char_                           [CharProcess(out)]
                )
                ;

            macro =
                (   escape_actions.macro                // must not be followed by
                >>  &(qi::eps - (qi::alpha | '_'))      // alpha or underscore
                )                                       [escape_actions.process]
                ;

            escape =
                "``" >> (
                    (qi::raw[+(qi::char_ - "``")] >> "``")
                                                        [parse_escaped]
                    | qi::raw[*qi::char_]               [escape_actions.error]
                )
                ;
        }

        qi::rule<iterator> program, macro, escape;

        Out& out;
        quickbook::actions& escape_actions;

        parse_escaped_impl parse_escaped;
        std::string save;
    };

}

#endif // BOOST_SPIRIT_QUICKBOOK_SYNTAX_HIGHLIGHT_HPP
