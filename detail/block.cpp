#include "../block.hpp"
#include "./actions_class.hpp"

namespace quickbook
{
    typedef boost::spirit::classic::scanner<iterator> scanner;

    void instantiate_phrase(block_grammar<quickbook::actions>& self)
    {
        block_grammar<quickbook::actions>::definition<scanner> definition(self);
    }
}