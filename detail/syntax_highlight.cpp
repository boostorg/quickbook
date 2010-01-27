#include "../syntax_highlight.hpp"
#include "./actions_class.hpp"

namespace quickbook
{
    typedef cpp_highlight<
        span
      , space
      , unexpected_char
      , collector>
    cpp_p_type;

    typedef python_highlight<
        span
      , space
      , unexpected_char
      , collector>
    python_p_type;
    
    typedef teletype_highlight<
        plain_char_action
      , collector>
    teletype_p_type;
    
    std::string syntax_highlight::operator()(iterator first, iterator last) const
    {
        escape_actions.phrase.push();

        // print the code with syntax coloring
        if (source_mode == "c++")
        {
            cpp_p_type cpp_p(escape_actions.phrase, escape_actions);
            parse(first, last, cpp_p);
        }
        else if (source_mode == "python")
        {
            python_p_type python_p(escape_actions.phrase, escape_actions);
            parse(first, last, python_p);
        }
        else if (source_mode == "teletype")
        {
            teletype_p_type teletype_p(escape_actions.phrase, escape_actions);
            parse(first, last, teletype_p);
        }
        else
        {
            BOOST_ASSERT(0);
        }

        std::string str;
        escape_actions.phrase.swap(str);
        escape_actions.phrase.pop();

        return str;
    }
}