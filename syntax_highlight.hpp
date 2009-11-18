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

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_chset.hpp>
#include <boost/spirit/include/classic_symbols.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include "./grammars.hpp"

namespace quickbook
{
    using namespace boost::spirit;

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
      , typename Out>
    struct cpp_highlight
    : public classic::grammar<cpp_highlight<Process, Space, Macro, DoMacro, PreEscape, PostEscape, EscapeActions, Unexpected, Out> >
    {
        cpp_highlight(Out& out, Macro const& macro, DoMacro do_macro, EscapeActions& escape_actions)
        : out(out), macro(macro), do_macro(do_macro), escape_actions(escape_actions) {}

        template <typename Scanner>
        struct definition
        {
            definition(cpp_highlight const& self)
                : common(self.escape_actions, unused)
                , unused(false)
            {
                program
                    =
                    *(  (+classic::space_p)      [Space(self.out)]
                    |   macro
                    |   escape
                    |   preprocessor    [Process("preprocessor", self.out)]
                    |   comment         [Process("comment", self.out)]
                    |   keyword         [Process("keyword", self.out)]
                    |   identifier      [Process("identifier", self.out)]
                    |   special         [Process("special", self.out)]
                    |   string_         [Process("string", self.out)]
                    |   char_           [Process("char", self.out)]
                    |   number          [Process("number", self.out)]
                    |   classic::repeat_p(1)[classic::anychar_p] [Unexpected(self.out)]
                    )
                    ;

                macro = 
                    classic::eps_p(self.macro                    // must not be followed by
                        >> (classic::eps_p - (classic::alpha_p | '_')))   // alpha or underscore
                    >> self.macro                       [self.do_macro]
                    ;

                qbk_phrase =
                   *(   common
                    |   (classic::anychar_p - classic::str_p("``"))   [self.escape_actions.plain_char]
                    )
                    ;

                escape =
                    classic::str_p("``")         [PreEscape(self.escape_actions, save)]
                    >>
                    (
                        (
                            (
                                (+(classic::anychar_p - "``") >> classic::eps_p("``"))
                                & qbk_phrase
                            )
                            >>  classic::str_p("``")
                        )
                        |
                        (
                            classic::eps_p       [self.escape_actions.error]
                            >> *classic::anychar_p
                        )
                    )                   [PostEscape(self.out, self.escape_actions, save)]
                    ;

                preprocessor
                    =   '#' >> *classic::space_p >> ((classic::alpha_p | '_') >> *(classic::alnum_p | '_'))
                    ;

                comment
                    =   classic::comment_p("//") | classic::comment_p("/*", "*/")
                    ;

                keyword
                    =   keyword_ >> (classic::eps_p - (classic::alnum_p | '_'))
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
                    =   +classic::chset_p("~!%^&*()+={[}]:;,<.>?/|\\-")
                    ;

                string_char = ('\\' >> classic::anychar_p) | (classic::anychar_p - '\\');

                string_
                    =   !classic::as_lower_d['l'] >> classic::confix_p('"', *string_char, '"')
                    ;

                char_
                    =   !classic::as_lower_d['l'] >> classic::confix_p('\'', *string_char, '\'')
                    ;

                number
                    =   (
                            classic::as_lower_d["0x"] >> classic::hex_p
                        |   '0' >> classic::oct_p
                        |   classic::real_p
                        )
                        >>  *classic::as_lower_d[classic::chset_p("ldfu")]
                    ;

                identifier
                    =   (classic::alpha_p | '_') >> *(classic::alnum_p | '_')
                    ;
            }

            classic::rule<Scanner>
                            program, macro, preprocessor, comment, special, string_, 
                            char_, number, identifier, keyword, qbk_phrase, escape,
                            string_char;

            classic::symbols<> keyword_;
            phrase_grammar<EscapeActions> common;
            std::string save;
            bool unused;

            classic::rule<Scanner> const&
            start() const { return program; }
        };

        Out& out;
        Macro const& macro;
        DoMacro do_macro;
        EscapeActions& escape_actions;
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
      , typename Out>
    struct python_highlight
    : public classic::grammar<python_highlight<Process, Space, Macro, DoMacro, PreEscape, PostEscape, EscapeActions, Unexpected, Out> >
    {
        python_highlight(Out& out, Macro const& macro, DoMacro do_macro, EscapeActions& escape_actions)
        : out(out), macro(macro), do_macro(do_macro), escape_actions(escape_actions) {}

        template <typename Scanner>
        struct definition
        {
            definition(python_highlight const& self)
                : common(self.escape_actions, unused)
                , unused(false)
            {
                program
                    =
                    *(  (+classic::space_p)      [Space(self.out)]
                    |   macro
                    |   escape          
                    |   comment         [Process("comment", self.out)]
                    |   keyword         [Process("keyword", self.out)]
                    |   identifier      [Process("identifier", self.out)]
                    |   special         [Process("special", self.out)]
                    |   string_         [Process("string", self.out)]
                    |   number          [Process("number", self.out)]
                    |   classic::repeat_p(1)[classic::anychar_p] [Unexpected(self.out)]
                    )
                    ;

                macro = 
                    classic::eps_p(self.macro                    // must not be followed by
                        >> (classic::eps_p - (classic::alpha_p | '_')))   // alpha or underscore
                    >> self.macro                       [self.do_macro]
                    ;

                qbk_phrase =
                   *(   common
                    |   (classic::anychar_p - classic::str_p("``"))   [self.escape_actions.plain_char]
                    )
                    ;

                escape =
                    classic::str_p("``")         [PreEscape(self.escape_actions, save)]
                    >>
                    (
                        (
                            (
                                (+(classic::anychar_p - "``") >> classic::eps_p("``"))
                                & qbk_phrase
                            )
                            >>  classic::str_p("``")
                        )
                        |
                        (
                            classic::eps_p       [self.escape_actions.error]
                            >> *classic::anychar_p
                        )
                    )                   [PostEscape(self.out, self.escape_actions, save)]
                    ;

                comment
                    =   classic::comment_p("#")
                    ;

                keyword
                    =   keyword_ >> (classic::eps_p - (classic::alnum_p | '_'))
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
                    =   +classic::chset_p("~!%^&*()+={[}]:;,<.>/|\\-")
                    ;

                string_prefix
                    =    classic::as_lower_d[classic::str_p("u") >> ! classic::str_p("r")]
                    ;
                
                string_
                    =   ! string_prefix >> (long_string | short_string)
                    ;

                string_char = ('\\' >> classic::anychar_p) | (classic::anychar_p - '\\');
            
                short_string
                    =   classic::confix_p('\'', * string_char, '\'') |
                        classic::confix_p('"', * string_char, '"')
                    ;
            
                long_string
                    // Note: the "str_p" on the next two lines work around
                    // an INTERNAL COMPILER ERROR when using VC7.1
                    =   classic::confix_p(classic::str_p("'''"), * string_char, "'''") |
                        classic::confix_p(classic::str_p("\"\"\""), * string_char, "\"\"\"")
                    ;
                
                number
                    =   (
                            classic::as_lower_d["0x"] >> classic::hex_p
                        |   '0' >> classic::oct_p
                        |   classic::real_p
                        )
                        >>  *classic::as_lower_d[classic::chset_p("lj")]
                    ;

                identifier
                    =   (classic::alpha_p | '_') >> *(classic::alnum_p | '_')
                    ;
            }

            classic::rule<Scanner>
                            program, macro, comment, special, string_, string_prefix, 
                            short_string, long_string, number, identifier, keyword, 
                            qbk_phrase, escape, string_char;

            classic::symbols<> keyword_;
            phrase_grammar<EscapeActions> common;
            std::string save;
            bool unused;

            classic::rule<Scanner> const&
            start() const { return program; }
        };

        Out& out;
        Macro const& macro;
        DoMacro do_macro;
        EscapeActions& escape_actions;
    };

    // Grammar for plain text (no actual highlighting)
    template <
        typename CharProcess
      , typename Macro
      , typename DoMacro
      , typename PreEscape
      , typename PostEscape
      , typename EscapeActions
      , typename Out>
    struct teletype_highlight
    : public classic::grammar<teletype_highlight<CharProcess, Macro, DoMacro, PreEscape, PostEscape, EscapeActions, Out> >
    {
        teletype_highlight(Out& out, Macro const& macro, DoMacro do_macro, EscapeActions& escape_actions)
        : out(out), macro(macro), do_macro(do_macro), escape_actions(escape_actions) {}

        template <typename Scanner>
        struct definition
        {
            definition(teletype_highlight const& self)
                : common(self.escape_actions, unused)
                , unused(false)
            {
                program
                    =
                    *(  macro
                    |   escape          
                    |   classic::repeat_p(1)[classic::anychar_p]          [CharProcess(self.out)]
                    )
                    ;

                macro = 
                    classic::eps_p(self.macro                    // must not be followed by
                        >> (classic::eps_p - (classic::alpha_p | '_')))   // alpha or underscore
                    >> self.macro                       [self.do_macro]
                    ;

                qbk_phrase =
                   *(   common
                    |   (classic::anychar_p - classic::str_p("``"))   [self.escape_actions.plain_char]
                    )
                    ;

                escape =
                    classic::str_p("``")         [PreEscape(self.escape_actions, save)]
                    >>
                    (
                        (
                            (
                                (+(classic::anychar_p - "``") >> classic::eps_p("``"))
                                & qbk_phrase
                            )
                            >>  classic::str_p("``")
                        )
                        |
                        (
                            classic::eps_p       [self.escape_actions.error]
                            >> *classic::anychar_p
                        )
                    )                   [PostEscape(self.out, self.escape_actions, save)]
                    ;
            }

            classic::rule<Scanner> program, macro, qbk_phrase, escape;

            phrase_grammar<EscapeActions> common;
            std::string save;
            bool unused;

            classic::rule<Scanner> const&
            start() const { return program; }
        };

        Out& out;
        Macro const& macro;
        DoMacro do_macro;
        EscapeActions& escape_actions;
    };

}

#endif // BOOST_SPIRIT_QUICKBOOK_SYNTAX_HIGHLIGHT_HPP
