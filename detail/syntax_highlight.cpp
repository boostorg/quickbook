#include "../syntax_highlight.hpp"
#include "./actions_class.hpp"

namespace quickbook
{
    std::string syntax_highlight::operator()(iterator first, iterator last) const
    {
        escape_actions.phrase.push();

        // print the code with syntax coloring
        if (source_mode == "c++")
        {
            cpp_highlight cpp_p(escape_actions);
            parse(first, last, cpp_p);
        }
        else if (source_mode == "python")
        {
            python_highlight python_p(escape_actions);
            parse(first, last, python_p);
        }
        else if (source_mode == "teletype")
        {
            teletype_highlight teletype_p(escape_actions);
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