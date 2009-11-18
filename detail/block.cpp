#include "../block.hpp"
#include "./actions_class.hpp"

namespace quickbook
{
    void instantiate_phrase(actions& a)
    {
        block_grammar<iterator, quickbook::actions, false> bg1(a);
        block_grammar<iterator, quickbook::actions, true> bg2(a);
    }
}