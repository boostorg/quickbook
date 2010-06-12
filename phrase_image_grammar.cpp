/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include "grammar_impl.hpp"
#include "phrase.hpp"
#include "actions.hpp"
#include "misc_rules.hpp"
#include "parse_utils.hpp"

namespace quickbook
{
    namespace qi = boost::spirit::qi;
    namespace ph = boost::phoenix;

    void quickbook_grammar::impl::init_phrase_image()
    {
        // Images

        qi::rule<iterator, quickbook::image()>& image = store_.create();
        qi::rule<iterator, quickbook::image()>& image_1_4 = store_.create();
        qi::rule<iterator, quickbook::image()>& image_1_5 = store_.create();
        qi::rule<iterator, std::string()>& image_filename = store_.create();
        qi::rule<iterator, quickbook::image::attribute_map()>& image_attributes = store_.create();
        qi::rule<iterator, std::pair<std::string, std::string>()>& image_attribute = store_.create();
        qi::rule<iterator, std::string()>& image_attribute_key = store_.create();
        qi::rule<iterator, std::string()>& image_attribute_value = store_.create();
        
        phrase_symbol_rules.add("$", image [actions.process]);

        image =
            (qi::eps(qbk_since(105u)) >> image_1_5) |
            (qi::eps(qbk_before(105u)) >> image_1_4);
        
        image_1_4 =
                position                            [member_assign(&quickbook::image::position)]
            >>  blank
            >>  (*(qi::char_ - phrase_end))         [member_assign(&quickbook::image::image_filename)]
            >>  &qi::lit(']')
            ;
        
        image_1_5 =
                position                            [member_assign(&quickbook::image::position)]
            >>  blank
            >>  image_filename                      [member_assign(&quickbook::image::image_filename)]
            >>  hard_space
            >>  image_attributes                    [member_assign(&quickbook::image::attributes)]
            >>  &qi::lit(']')
            ;

        image_filename = qi::raw[
                +(qi::char_ - (qi::space | phrase_end | '['))
            >>  *(
                    +qi::space
                >>  +(qi::char_ - (qi::space | phrase_end | '['))
             )];

        image_attributes = *(image_attribute >> space);
        
        image_attribute =
                '['
            >>  image_attribute_key                 [member_assign(&std::pair<std::string, std::string>::first)]
            >>  space
            >>  image_attribute_value               [member_assign(&std::pair<std::string, std::string>::second)]
            >>  ']'
            ;
            
        image_attribute_key = *(qi::alnum | '_');
        image_attribute_value = *(qi::char_ - (phrase_end | '['));
    }
}