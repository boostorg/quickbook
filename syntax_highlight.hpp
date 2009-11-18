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

namespace quickbook
{
    using namespace boost::spirit;

    template <class Iterator, class EscapeActions>
    struct parse_escaped_impl
    {
        parse_escaped_impl(EscapeActions& escape_actions)
            : actions(escape_actions) {}

        void operator()(boost::iterator_range<Iterator> escaped, unused_type, unused_type) const {
            bool unused;
            phrase_grammar<Iterator, EscapeActions> common(actions, unused);   
            Iterator first = escaped.begin(), last = escaped.end();
            while(first != last) {
                if(!qi::parse(first, last, common)) {
                    actions.plain_char(*first, 0, 0);
                    ++first;
                }
            }
        }
        
        EscapeActions& actions;
    };

    // Grammar for C++ highlighting
    template <
        typename Process
      , typename Space
      , typename Macro
      , typename DoMacro
      , typename PreEscape
      , typename PostEscape
      , typename EscapeActions
      , typename Unexpected
      , typename Out
      , typename Iterator>
    struct cpp_highlight
    : public qi::grammar<Iterator>
    {
        cpp_highlight(Out& out, Macro const& macro_symbols, DoMacro do_macro, EscapeActions& escape_actions)
        : cpp_highlight::base_type(program), out(out), macro_symbols(macro_symbols), do_macro(do_macro), escape_actions(escape_actions)
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
                &(macro_symbols                         // must not be followed by
                    >> (qi::eps - (qi::alpha | '_')))   // alpha or underscore
                >> macro_symbols            [do_macro]
                ;

            escape =
                qi::string("``")            [PreEscape(escape_actions, save)]
                >>
                (
                    (
                        (
                            // TODO: Is this right?
                            qi::raw[+(qi::char_ - "``") >> &qi::lit("``")]
                                [parse_escaped]
                        )
                        >>  qi::string("``")
                    )
                    |
                    (
                        qi::raw[qi::eps]    [escape_actions.error]
                        >> *qi::char_
                    )
                )                           [PostEscape(out, escape_actions, save)]
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

        qi::rule<Iterator>
                        program, macro, preprocessor, comment, special, string_, 
                        char_, number, identifier, keyword, escape,
                        string_char;

        Out& out;
        Macro const& macro_symbols;
        DoMacro do_macro;
        EscapeActions& escape_actions;

        qi::symbols<> keyword_;
        parse_escaped_impl<Iterator, EscapeActions> parse_escaped;
        std::string save;
    };

    // Grammar for Python highlighting
    // See also: The Python Reference Manual
    // http://docs.python.org/ref/ref.html
    template <
        typename Process
      , typename Space
      , typename Macro
      , typename DoMacro
      , typename PreEscape
      , typename PostEscape
      , typename EscapeActions
      , typename Unexpected
      , typename Out
      , typename Iterator>
    struct python_highlight
    : public qi::grammar<Iterator>
    {
        python_highlight(Out& out, Macro const& macro_symbols, DoMacro do_macro, EscapeActions& escape_actions)
        : python_highlight::base_type(program), out(out), macro_symbols(macro_symbols), do_macro(do_macro), escape_actions(escape_actions)
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
                &(macro_symbols                         // must not be followed by
                    >> (qi::eps - (qi::alpha | '_')))   // alpha or underscore
                >> macro_symbols            [do_macro]
                ;

            escape =
                qi::string("``")            [PreEscape(escape_actions, save)]
                >>
                (
                    (
                        (
                            qi::raw[+(qi::char_ - "``") >> &qi::lit("``")]
                                [parse_escaped]
                        )
                        >>  qi::string("``")
                    )
                    |
                    (
                        qi::raw[qi::eps]    [escape_actions.error]
                    >>  *qi::char_
                    )
                )                           [PostEscape(out, escape_actions, save)]
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

        qi::rule<Iterator>
                        program, macro, comment, special, string_, string_prefix, 
                        short_string, long_string, number, identifier, keyword, 
                        escape, string_char;

        Out& out;
        Macro const& macro_symbols;
        DoMacro do_macro;
        EscapeActions& escape_actions;

        qi::symbols<> keyword_;
        parse_escaped_impl<Iterator, EscapeActions> parse_escaped;
        std::string save;
    };

    // Grammar for plain text (no actual highlighting)
    template <
        typename CharProcess
      , typename Macro
      , typename DoMacro
      , typename PreEscape
      , typename PostEscape
      , typename EscapeActions
      , typename Out
      , typename Iterator>
    struct teletype_highlight
    : public qi::grammar<Iterator>
    {
        teletype_highlight(Out& out, Macro const& macro_symbols, DoMacro do_macro, EscapeActions& escape_actions)
        : teletype_highlight::base_type(program), out(out), macro_symbols(macro_symbols), do_macro(do_macro), escape_actions(escape_actions)
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
                &(macro_symbols                         // must not be followed by
                    >> (qi::eps - (qi::alpha | '_')))   // alpha or underscore
                >> macro_symbols            [do_macro]
                ;

            escape =
                qi::lit("``")                           [PreEscape(escape_actions, save)]
                >>
                (
                    (
                        (
                            qi::raw[+(qi::char_ - "``") >> &qi::lit("``")]
                                [parse_escaped]
                        )
                        >>  qi::string("``")
                    )
                    |
                    (
                        qi::raw[qi::eps]                [escape_actions.error]
                        >> *qi::char_
                    )
                )                                       [PostEscape(out, escape_actions, save)]
                ;
        }

        qi::rule<Iterator> program, macro, escape;

        Out& out;
        Macro const& macro_symbols;
        DoMacro do_macro;
        EscapeActions& escape_actions;

        parse_escaped_impl<Iterator, EscapeActions> parse_escaped;
        std::string save;
    };

}

#endif // BOOST_SPIRIT_QUICKBOOK_SYNTAX_HIGHLIGHT_HPP
