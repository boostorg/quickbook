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
#include <boost/fusion/include/adapt_struct.hpp>
#include "./parse_types.hpp"
#include "./grammars.hpp"
#include "./phrase.hpp"

namespace quickbook
{
    struct code_token
    {
        std::string text;
        char const* type;
    };
    
    struct space
    {
        std::string text;
        char const* dummy;
    };
    
    void process(quickbook::actions& actions, code_token const& x)
    {
        std::string type = x.type;
        if(type == "space") {
            actions.phrase << x.text;
        }
        else {
            actions.phrase << "<phrase role=\"" << x.type << "\">";
            detail::print_string(x.text, actions.phrase.get());
            actions.phrase << "</phrase>";
        }
    }
}   

BOOST_FUSION_ADAPT_STRUCT(
    quickbook::code_token,
    (std::string, text)
    (char const*, type)
)

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    using boost::spirit::unused_type;

    struct parse_escaped_impl
    {
        parse_escaped_impl(quickbook::actions& actions)
            : actions(actions) {}

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
    struct cpp_highlight : public qi::grammar<iterator>
    {
        cpp_highlight(quickbook::actions& actions)
        : cpp_highlight::base_type(program), actions(actions)
        , parse_escaped(actions)
        {
            program
                =
                *(  space
                |   macro
                |   escape
                |   preprocessor
                |   comment
                |   keyword
                |   identifier
                |   special
                |   string_
                |   char_
                |   number
                |   unexpected
                )                           [actions.process]
                ;

            macro =
                (   actions.macro           // must not be followed by
                >>  !(qi::alpha | '_')      // alpha or underscore
                )
                ;

            escape =
                "``" >> (
                    (qi::raw[+(qi::char_ - "``")] >> "``")
                                                        [parse_escaped]
                    | qi::raw[*qi::char_]               [actions.error]
                )
                ;

            space
                =   qi::raw[+qi::space]
                >>  qi::attr("space");

            preprocessor
                =   qi::raw[
                        '#' >> *qi::space
                    >>  ((qi::alpha | '_') >> *(qi::alnum | '_'))
                    ]
                >>  qi::attr("preprocessor")
                ;

            comment
                =   qi::raw[
                        qi::lit("//") >> *(qi::char_ - qi::eol) >> -qi::eol
                    |   qi::lit("/*") >> *(qi::char_ - "*/") >> -qi::lit("*/")
                    ]
                >>  qi::attr("comment")
                ;

            keyword
                =   qi::raw[keyword_ >> !(qi::alnum | '_')]
                >>  qi::attr("keyword")
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
                >>  qi::attr("special")
                ;

            string_char = ('\\' >> qi::char_) | (qi::char_ - '\\');

            string_
                =   qi::raw[
                        -qi::no_case['l']
                    >>  '"' >> *(string_char - '"') >> -qi::lit('"')
                    ]
                >>  qi::attr("string")
                ;

            char_ =
                    qi::raw[
                        -qi::no_case['l']
                    >>  '\'' >> *(string_char - '\'') >> -qi::lit('\'')
                    ]
                >>  qi::attr("char")
                ;

            number
                =   qi::raw
                    [
                        (
                            qi::no_case["0x"] >> qi::hex
                        |   '0' >> qi::oct
                        |   qi::long_double
                        )
                    >>  *qi::no_case[qi::char_("ldfu")]
                    ]
                >>  qi::attr("number")
                ;

            identifier
                =   qi::raw[(qi::alpha | '_') >> *(qi::alnum | '_')]
                >>  qi::attr("identifier")
                ;
            
            // TODO: warn user?
            unexpected
                =   qi::raw[qi::char_]
                >>  qi::attr("error")
                ;
        }

        qi::rule<iterator> program, escape, string_char;
        qi::rule<iterator, quickbook::macro()> macro;
        qi::rule<iterator, code_token()>
                        space, preprocessor, comment, special, string_, 
                        char_, number, identifier, keyword, unexpected;

        quickbook::actions& actions;

        qi::symbols<> keyword_;
        parse_escaped_impl parse_escaped;
        std::string save;
    };

    // Grammar for Python highlighting
    // See also: The Python Reference Manual
    // http://docs.python.org/ref/ref.html
    struct python_highlight : public qi::grammar<iterator>
    {
        python_highlight(quickbook::actions& actions)
        : python_highlight::base_type(program), actions(actions)
        , parse_escaped(actions)
        {
            program
                =
                *(  space
                |   macro
                |   escape
                |   comment
                |   keyword
                |   identifier
                |   special
                |   string_
                |   number
                |   unexpected
                )                           [actions.process]
                ;

            macro =
                (   actions.macro           // must not be followed by
                >>  !(qi::alpha | '_')      // alpha or underscore
                )
                ;

            escape =
                "``" >> (
                    (qi::raw[+(qi::char_ - "``")] >> "``")
                                                        [parse_escaped]
                    | qi::raw[*qi::char_]               [actions.error]
                )
                ;

            space
                =   qi::raw[+qi::space]
                >>  qi::attr("space");

            comment
                =   qi::raw[
                        '#' >> *(qi::char_ - qi::eol) >> -qi::eol
                    ]
                >>  qi::attr("comment")
                ;

            keyword
                =   qi::raw[keyword_ >> !(qi::alnum | '_')]
                >>  qi::attr("keyword")
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
                >>  qi::attr("special")
                ;

            string_prefix
                =    qi::no_case[qi::string("u") >> - qi::string("r")]
                ;
            
            string_
                =   qi::raw[
                        -string_prefix
                    >>  (long_string | short_string)
                    ]
                >>  qi::attr("string")
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
                =   qi::raw[
                        (
                            qi::no_case["0x"] >> qi::hex
                        |   '0' >> qi::oct
                        |   qi::long_double
                        )
                        >>  *qi::no_case[qi::char_("lj")]
                    ]
                >>  qi::attr("number")
                ;

            identifier
                =   qi::raw[(qi::alpha | '_') >> *(qi::alnum | '_')]
                >>  qi::attr("identifier")
                ;

            // TODO: warn user?
            unexpected
                =   qi::raw[qi::char_]
                >>  qi::attr("error")
                ;
        }

        qi::rule<iterator>
                        program, string_prefix, short_string, long_string,
                        escape, string_char;
        qi::rule<iterator, quickbook::macro()> macro;
        qi::rule<iterator, code_token()>
                        space, comment, special, string_, 
                        number, identifier, keyword, unexpected;

        quickbook::actions& actions;

        qi::symbols<> keyword_;
        parse_escaped_impl parse_escaped;
        std::string save;
    };

    // Grammar for plain text (no actual highlighting)
    struct teletype_highlight : public qi::grammar<iterator>
    {
        teletype_highlight(quickbook::actions& actions)
        : teletype_highlight::base_type(program), actions(actions)
        , parse_escaped(actions)
        {
            program
                =
                *(  macro                   [actions.process]
                |   escape          
                |   qi::char_               [actions.plain_char]
                )
                ;

            macro =
                (   actions.macro           // must not be followed by
                >>  !(qi::alpha | '_')      // alpha or underscore
                )
                ;

            escape =
                "``" >> (
                    (qi::raw[+(qi::char_ - "``")] >> "``")
                                            [parse_escaped]
                    | qi::raw[*qi::char_]   [actions.error]
                )
                ;
        }

        qi::rule<iterator> program, escape;
        qi::rule<iterator, quickbook::macro()> macro;

        quickbook::actions& actions;

        parse_escaped_impl parse_escaped;
        std::string save;
    };

}

#endif // BOOST_SPIRIT_QUICKBOOK_SYNTAX_HIGHLIGHT_HPP
