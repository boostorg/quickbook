/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "./phrase.hpp"
#include "./grammars.hpp"
#include "./detail/actions_class.hpp"
#include "./detail/quickbook.hpp"

namespace quickbook
{    
    namespace
    {
        std::string::size_type find_bracket_end(std::string const& str, std::string::size_type pos)
        {
            unsigned int depth = 1;

            while(depth > 0) {
                pos = str.find_first_of("[]\\", pos);
                if(pos == std::string::npos) return pos;

                if(str[pos] == '\\')
                {
                    pos += 2;
                }
                else
                {
                    depth += (str[pos] == '[') ? 1 : -1;
                    ++pos;
                }
            }

            return pos;
        }

        std::string::size_type find_first_seperator(std::string const& str)
        {
            if(qbk_version_n < 105) {
                return str.find_first_of(" \t\r\n");
            }
            else {
                std::string::size_type pos = 0;

                while(true)
                {
                    pos = str.find_first_of(" \t\r\n\\[", pos);
                    if(pos == std::string::npos) return pos;

                    switch(str[pos])
                    {
                    case '[':
                        pos = find_bracket_end(str, pos + 1);
                        break;
                    case '\\':
                        pos += 2;
                        break;
                    default:
                        return pos;
                    }
                }
            }
        }
    
        bool break_arguments(
            std::vector<std::string>& params
          , std::vector<std::string> const& template_
          , boost::spirit::classic::file_position const& pos
        )
        {
            // Quickbook 1.4-: If there aren't enough parameters seperated by
            //                 '..' then seperate the last parameter using
            //                 whitespace.
            // Quickbook 1.5+: If '..' isn't used to seperate the parameters
            //                 then use whitespace to separate them
            //                 (2 = template name + argument).

            if (qbk_version_n < 105 || params.size() == 1)
            {
                // template_.size() - 2 because template_ also includes the name and body.
                while (params.size() < template_.size() - 2 )
                {
                    // Try to break the last argument at the first space found
                    // and push it into the back of params. Do this
                    // recursively until we have all the expected number of
                    // arguments, or if there are no more spaces left.

                    std::string& str = params.back();
                    std::string::size_type l_pos = find_first_seperator(str);
                    if (l_pos == std::string::npos)
                        break;
                    std::string first(str.begin(), str.begin()+l_pos);
                    std::string::size_type r_pos = str.find_first_not_of(" \t\r\n", l_pos);
                    if (r_pos == std::string::npos)
                        break;
                    std::string second(str.begin()+r_pos, str.end());
                    str = first;
                    params.push_back(second);
                }
            }

            if (params.size() != template_.size() - 2)
            {
                detail::outerr(pos.file, pos.line)
                    << "Invalid number of arguments passed. Expecting: "
                    << template_.size()-2
                    << " argument(s), got: "
                    << params.size()
                    << " argument(s) instead."
                    << std::endl;
                return false;
            }
            return true;
        }

        std::pair<bool, std::vector<std::string>::const_iterator>
        get_arguments(
            std::vector<std::string>& params
          , std::vector<std::string> const& template_
          , template_scope const& scope
          , boost::spirit::classic::file_position const& pos
          , quickbook::actions& actions
        )
        {
            std::vector<std::string>::const_iterator arg = params.begin();
            std::vector<std::string>::const_iterator tpl = template_.begin()+1;

            // Store each of the argument passed in as local templates:
            while (arg != params.end())
            {
                std::vector<std::string> tinfo;
                tinfo.push_back(*tpl);
                tinfo.push_back(*arg);
                template_symbol template_(tinfo, pos, &scope);

                if (actions.templates.find_top_scope(*tpl))
                {
                    detail::outerr(pos.file,pos.line)
                        << "Duplicate Symbol Found" << std::endl;
                    ++actions.error_count;
                    return std::make_pair(false, tpl);
                }
                else
                {
                    actions.templates.add(*tpl, template_);
                }
                ++arg; ++tpl;
            }
            return std::make_pair(true, tpl);
        }

        bool parse_template(
            std::string& body
          , std::string& result
          , boost::spirit::classic::file_position const& template_pos
          , bool template_escape
          , quickbook::actions& actions
        )
        {
            // How do we know if we are to parse the template as a block or
            // a phrase? We apply a simple heuristic: if the body starts with
            // a newline, then we regard it as a block, otherwise, we parse
            // it as a phrase.

            std::string::const_iterator iter = body.begin();
            while (iter != body.end() && ((*iter == ' ') || (*iter == '\t')))
                ++iter; // skip spaces and tabs
            bool is_block = (iter != body.end()) && ((*iter == '\r') || (*iter == '\n'));
            bool r = false;

            if (template_escape)
            {
                //  escape the body of the template
                //  we just copy out the literal body
                result = body;
                r = true;
            }
            else if (!is_block)
            {
                simple_phrase_grammar phrase_p(actions);

                //  do a phrase level parse
                iterator first(body.begin(), body.end(), actions.filename.native_file_string().c_str());
                first.set_position(template_pos);
                iterator last(body.end(), body.end());
                r = boost::spirit::qi::parse(first, last, phrase_p) && first == last;
                actions.phrase.swap(result);
            }
            else
            {
                block_grammar block_p(actions);

                //  do a block level parse
                //  ensure that we have enough trailing newlines to eliminate
                //  the need to check for end of file in the grammar.
                body.push_back('\n');
                body.push_back('\n');
                while (iter != body.end() && ((*iter == '\r') || (*iter == '\n')))
                    ++iter; // skip initial newlines
                iterator first(iter, body.end(), actions.filename.native_file_string().c_str());
                first.set_position(template_pos);
                iterator last(body.end(), body.end());
                r = boost::spirit::qi::parse(first, last, block_p) && first == last;
                actions.phrase.swap(result);
            }
            return r;
        }
    }

    void process(quickbook::actions& actions, template_ const& x)
    {
        ++actions.template_depth;
        if (actions.template_depth > actions.max_template_depth)
        {
            detail::outerr(x.position.file,x.position.line)
                << "Infinite loop detected" << std::endl;
            --actions.template_depth;
            ++actions.error_count;
            return;
        }

        // The template arguments should have the scope that the template was
        // called from, not the template's own scope.
        //
        // Note that for quickbook 1.4- this value is just ignored when the
        // arguments are expanded.
        template_scope const& call_scope = actions.templates.top_scope();

        std::string result;
        actions.push(); // scope the actions' states
        {
            // Quickbook 1.4-: When expanding the tempalte continue to use the
            //                 current scope (the dynamic scope).
            // Quickbook 1.5+: Use the scope the template was defined in
            //                 (the static scope).
            if (qbk_version_n >= 105)
                actions.templates.set_parent_scope(*boost::get<2>(x.symbol));

            std::vector<std::string> template_ = boost::get<0>(x.symbol);
            boost::spirit::classic::file_position template_pos = boost::get<1>(x.symbol);

            std::vector<std::string> params = x.params;
    
            ///////////////////////////////////
            // Break the arguments
            if (!break_arguments(params, template_, x.position))
            {
                actions.pop(); // restore the actions' states
                --actions.template_depth;
                ++actions.error_count;
                return;
            }

            ///////////////////////////////////
            // Prepare the arguments as local templates
            bool get_arg_result;
            std::vector<std::string>::const_iterator tpl;
            boost::tie(get_arg_result, tpl) =
                get_arguments(params, template_,
                    call_scope, x.position, actions);

            if (!get_arg_result)
            {
                actions.pop(); // restore the actions' states
                --actions.template_depth;
                return;
            }

            ///////////////////////////////////
            // parse the template body:
            std::string body;
            body.assign(tpl->begin(), tpl->end());
            body.reserve(body.size()+2); // reserve 2 more

            if (!parse_template(body, result, template_pos, x.escape, actions))
            {
                detail::outerr(x.position.file,x.position.line)
                    //<< "Expanding template:" << template_info[0] << std::endl
                    << std::endl
                    << "------------------begin------------------" << std::endl
                    << body
                    << "------------------end--------------------" << std::endl
                    << std::endl;
                actions.pop(); // restore the actions' states
                --actions.template_depth;
                ++actions.error_count;
                return;
            }
        }

        actions.pop(); // restore the actions' states
        actions.phrase << result; // print it!!!
        --actions.template_depth;
    }
}