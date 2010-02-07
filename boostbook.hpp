#include "fwd.hpp"
#include "phrase.hpp"

namespace quickbook
{
    // Output function for boostbook, these should eventually become an
    // interface with implementations for boostbook and html.
    // They probably shouldn't use quickbook::actions, instead they
    // should either take a stream/collector to write to, or return their
    // output by value.

    void output(quickbook::actions&, std::string const&);
    void output(quickbook::actions&, anchor const&);
    void output(quickbook::actions&, link const&);
    void output(quickbook::actions&, formatted const&);
}
