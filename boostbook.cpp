#include "fwd.hpp"
#include "boostbook.hpp"
#include "phrase.hpp"
#include "actions_class.hpp"
#include "utils.hpp"

namespace quickbook
{
    void output(quickbook::actions& actions, std::string const& x)
    {
        actions.phrase << x;
    }

    void output(quickbook::actions& actions, anchor const& x) {
        actions.phrase << "<anchor id=\"";
        detail::print_string(x.id, actions.phrase.get());
        actions.phrase << "\" />\n";
    }

    void output(quickbook::actions& actions, link const& x) {
        actions.phrase << x.type.pre;
        detail::print_string(x.destination, actions.phrase.get());
        actions.phrase << "\">";
        actions.phrase << x.content;
        actions.phrase << x.type.post;
    }

    void output(quickbook::actions& actions, formatted const& x) {
        actions.phrase << x.type.pre << x.content << x.type.post;
    }
}
