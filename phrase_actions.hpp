#include "phrase.hpp"

namespace quickbook
{
    nothing process(quickbook::actions&, source_mode const&);
    std::string process(quickbook::actions&, macro const&);
    link process(quickbook::actions&, link const&);
    formatted process(quickbook::actions&, simple_markup const&);
}
