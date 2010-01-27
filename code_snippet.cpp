/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2005 Thomas Guest
    Copyright (c) 2010 Daniel James
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <numeric>
#include <functional>
#include <algorithm>
#include <iterator>
#include <boost/lexical_cast.hpp>
#include "./detail/utils.hpp"
#include "./detail/markups.hpp"
#include "./detail/actions_class.hpp"
#include "./grammars.hpp"
#include "./code_snippet.hpp"

namespace quickbook
{
    void code_snippet_actions::pass_thru(char x)
    {
        code += x;
    }

    namespace detail
    {
        int callout_id = 0;
    }

    void code_snippet_actions::callout(std::string const& x, char const* role)
    {
        using detail::callout_id;
        code += "``'''";
        code += std::string("<phrase role=\"") + role + "\">";
        code += "<co id=\"";
        code += doc_id + boost::lexical_cast<std::string>(callout_id + callouts.size()) + "co\" ";
        code += "linkends=\"";
        code += doc_id + boost::lexical_cast<std::string>(callout_id + callouts.size()) + "\" />";
        code += "</phrase>";
        code += "'''``";

        callouts.push_back(x);
    }

    void code_snippet_actions::inline_callout(std::string const& x)
    {
        callout(x, "callout_bug");
    }

    void code_snippet_actions::line_callout(std::string const& x)
    {
        callout(x, "line_callout_bug");
    }

    void code_snippet_actions::escaped_comment(std::string const& x)
    {
        if (!code.empty())
        {
            detail::unindent(code); // remove all indents
            if (code.size() != 0)
            {
                snippet += "\n\n";
                snippet += source_type;
                snippet += "``\n" + code + "``\n\n";
                code.clear();
            }
        }
        std::string temp(x);
        detail::unindent(temp); // remove all indents
        if (temp.size() != 0)
        {
            snippet += "\n" + temp; // add a linebreak to allow block marskups
        }
    }

    void code_snippet_actions::compile(boost::iterator_range<iterator> x)
    {
        using detail::callout_id;
        if (!code.empty())
        {
            detail::unindent(code); // remove all indents
            if (code.size() != 0)
            {
                snippet += "\n\n";
                snippet += source_type;
                snippet += "```\n" + code + "```\n\n";
            }

            if(callouts.size() > 0)
            {
              snippet += "'''<calloutlist>'''";
              for (size_t i = 0; i < callouts.size(); ++i)
              {
                  snippet += "'''<callout arearefs=\"";
                  snippet += doc_id + boost::lexical_cast<std::string>(callout_id + i) + "co\" ";
                  snippet += "id=\"";
                  snippet += doc_id + boost::lexical_cast<std::string>(callout_id + i) + "\">";
                  snippet += "'''";

                  snippet += "'''<para>'''";
                  snippet += callouts[i];
                  snippet += "'''</para>'''";
                  snippet += "'''</callout>'''";
              }
              snippet += "'''</calloutlist>'''";
            }
        }

        std::vector<std::string> tinfo;
        tinfo.push_back(id);
        tinfo.push_back(snippet);
        storage.push_back(template_symbol(tinfo, x.begin().get_position()));

        callout_id += callouts.size();
        callouts.clear();
        code.clear();
        snippet.clear();
        id.clear();
    }
}
