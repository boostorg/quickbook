#include "encoder_impl.hpp"

namespace quickbook
{
    encoder_ptr create_encoder(std::string const& name)
    {
        if(name == "html") {
            return boost::shared_ptr<encoder>(new html_encoder());
        }
        else if(name == "boostbook") {
            return boost::shared_ptr<encoder>(new boostbook_encoder());
        }
        else {
            BOOST_ASSERT(false);
        }
    }
}