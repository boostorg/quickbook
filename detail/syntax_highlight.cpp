#include "../syntax_highlight.hpp"
#include "./actions_class.hpp"

namespace quickbook
{
    typedef cpp_highlight<
        span
      , space
      , string_symbols
      , do_macro_action
      , pre_escape_back
      , post_escape_back
      , actions
      , unexpected_char
      , collector
      , iterator>
    cpp_p_type;

    typedef python_highlight<
        span
      , space
      , string_symbols
      , do_macro_action
      , pre_escape_back
      , post_escape_back
      , actions
      , unexpected_char
      , collector
      , iterator>
    python_p_type;
    
    typedef teletype_highlight<
        plain_char_action
      , string_symbols
      , do_macro_action
      , pre_escape_back
      , post_escape_back
      , actions
      , collector
      , iterator>
    teletype_p_type;
    
    std::string syntax_highlight::operator()(iterator first, iterator last) const
    {
        // print the code with syntax coloring
        if (source_mode == "c++")
        {
            cpp_p_type cpp_p(temp, macro, do_macro_action(temp), escape_actions);
            parse(first, last, cpp_p);
        }
        else if (source_mode == "python")
        {
            python_p_type python_p(temp, macro, do_macro_action(temp), escape_actions);
            parse(first, last, python_p);
        }
        else if (source_mode == "teletype")
        {
            teletype_p_type teletype_p(temp, macro, do_macro_action(temp), escape_actions);
            parse(first, last, teletype_p);
        }
        else
        {
            BOOST_ASSERT(0);
        }

        std::string str;
        temp.swap(str);
        
        return str;
    }
}