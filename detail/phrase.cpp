#include "../phrase.hpp"
#include "./actions_class.hpp"

namespace quickbook
{
    void instantiate_simple_phrase(actions& a)
    {
        simple_phrase_grammar<iterator, actions> spgd(a);
    }

    void instantiate_phrase(actions& a, bool& b)
    {
        phrase_grammar<iterator, actions> pgd(a, b);
    }
}