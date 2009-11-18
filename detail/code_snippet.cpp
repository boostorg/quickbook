#include "../code_snippet.hpp"

namespace quickbook
{
    void instantiate_code_snippet(code_snippet_actions& actions)
    {
        python_code_snippet_grammar<iterator> python(actions);
        cpp_code_snippet_grammar<iterator> cpp(actions);
    }
}