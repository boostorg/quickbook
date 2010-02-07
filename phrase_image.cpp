/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include "phrase.hpp"
#include "gen_types.hpp"
#include "actions_class.hpp"
#include "utils.hpp"

namespace quickbook
{
    namespace fs = boost::filesystem;

    image2 process(quickbook::actions& actions, image const& x)
    {
        std::map<std::string, std::string> attributes(
            x.attributes.begin(), x.attributes.end());
        
        if(attributes.size() != x.attributes.size()) {
            std::map<std::string, std::string> duplicates;
            std::set_difference(
                x.attributes.begin(), x.attributes.end(),
                attributes.begin(), attributes.end(),
                std::inserter(duplicates, duplicates.end()));
            
            for(std::map<std::string, std::string>::iterator
                begin = duplicates.begin(), end = duplicates.end();
                begin != end; ++begin)
            {
                detail::outerr(x.position.file, x.position.line)
                    << "Duplicate image attribute: "
                    << begin->first
                    << std::endl;
                ++actions.error_count;
            }
        }
    
        fs::path const img_path(x.image_filename);
        attributes.insert(attribute_map::value_type("fileref", x.image_filename));
        // Note: If there is already an alt attribute this is a no-op.
        attributes.insert(attribute_map::value_type("alt", fs::basename(img_path)));

        if(fs::extension(img_path) == ".svg")
        {
           //
           // SVG's need special handling:
           //
           // 1) We must set the "format" attribute, otherwise
           //    HTML generation produces code that will not display
           //    the image at all.
           // 2) We need to set the "contentwidth" and "contentdepth"
           //    attributes, otherwise the image will be displayed inside
           //    a tiny box with scrollbars (Firefox), or else cropped to
           //    fit in a tiny box (IE7).
           //
           attributes.insert(attribute_map::value_type("format", "SVG"));
           //
           // Image paths are relative to the html subdirectory:
           // TODO: This only works when you're running in the correct directory.
           // Support 'boost:' directories? Include paths?
           //
           fs::path img;
           if(img_path.root_path().empty())
              img = "html" / img_path;  // relative path
           else
              img = img_path;   // absolute path
           //
           // Now load the SVG file:
           //
           std::string svg_text;
           fs::ifstream fs(img);
           char c;
           while(fs.get(c) && fs.good())
              svg_text.push_back(c);
           //
           // Extract the svg header from the file:
           //
           std::string::size_type a, b;
           a = svg_text.find("<svg");
           b = svg_text.find('>', a);
           svg_text = (a == std::string::npos) ? "" : svg_text.substr(a, b - a);
           //
           // Now locate the "width" and "height" attributes
           // and borrow their values:
           //
           a = svg_text.find("width");
           a = svg_text.find('=', a);
           a = svg_text.find('\"', a);
           b = svg_text.find('\"', a + 1);
           if(a != std::string::npos)
           {
              attributes.insert(attribute_map::value_type("contentwidth",
                std::string(svg_text.begin() + a + 1, svg_text.begin() + b)));
           }
           a = svg_text.find("height");
           a = svg_text.find('=', a);
           a = svg_text.find('\"', a);
           b = svg_text.find('\"', a + 1);
           if(a != std::string::npos)
           {
              attributes.insert(attribute_map::value_type("contentdepth",
                std::string(svg_text.begin() + a + 1, svg_text.begin() + b)));
           }
        }
        
        return image2(attributes);
    }
}