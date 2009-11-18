#include "../doc_info.hpp"
#include "./actions_class.hpp"

namespace quickbook
{
    typedef boost::spirit::classic::scanner<iterator> scanner;

    void instantiate_phrase(doc_info_grammar<quickbook::actions>& self)
    {
        doc_info_grammar<quickbook::actions>::definition<scanner> definition(self);
    }
}