#include "../doc_info.hpp"
#include "./actions_class.hpp"

namespace quickbook
{
    void instantiate_doc_info(quickbook::actions& a)
    {
        doc_info_grammar<iterator, quickbook::actions> grammar(a);
    }
}