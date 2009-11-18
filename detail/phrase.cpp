#include "../phrase.hpp"
#include "./actions_class.hpp"

namespace quickbook
{
    typedef boost::spirit::classic::scanner<iterator> scanner;

    void instantiate_simple_phrase(simple_phrase_grammar<quickbook::actions>& self)
    {
        simple_phrase_grammar<quickbook::actions>::definition<scanner> spgd(self);
    }

    void instantiate_phrase(phrase_grammar<quickbook::actions>& self)
    {
        phrase_grammar<quickbook::actions>::definition<scanner> pgd(self);
    }
}