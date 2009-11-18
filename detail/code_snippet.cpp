#include "../code_snippet.hpp"

namespace quickbook
{
    typedef boost::spirit::classic::scanner<iterator> scanner;

    void instantiate_code_snippet(
        python_code_snippet_grammar& python,
        cpp_code_snippet_grammar& cpp)
    {
        python_code_snippet_grammar::definition<scanner> python_definition(python);
        cpp_code_snippet_grammar::definition<scanner> cpp_definition(cpp);
    }
}