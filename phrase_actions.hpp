#include "phrase.hpp"
#include "code.hpp"

namespace quickbook
{
    nothing process(quickbook::actions&, source_mode const&);
    std::string process(quickbook::actions&, macro const&);
    link process(quickbook::actions&, link const&);
    formatted process(quickbook::actions&, simple_markup const&);
    std::string process(quickbook::actions&, cond_phrase const&);
    break_ process(quickbook::actions&, break_ const&);
    formatted process(quickbook::actions&, code const&);
}
