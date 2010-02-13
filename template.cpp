/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_symbols.hpp>
#include "template.hpp"
#include "phrase_actions.hpp"
#include "grammars.hpp"
#include "state.hpp"
#include "utils.hpp"

#ifdef BOOST_MSVC
#pragma warning(disable : 4355)
#endif

namespace quickbook
{
    struct template_symbol
    {
        template_symbol(
                std::string const& identifier,
                std::vector<std::string> const& params,
                template_value const& body,
                template_scope const* parent)
           : identifier(identifier)
           , params(params)
           , body(body)
           , parent(parent) {}
    
        std::string identifier;
        std::vector<std::string> params;
        template_value body;
        template_scope const* parent;
    };

    typedef boost::spirit::qi::symbols<char, template_symbol> template_symbols;    

    // template scope
    //
    // 1.4-: parent_scope is the previous scope on the stack
    //       (the template's dynamic parent).
    // 1.5+: parent_scope is the template's lexical parent.
    //
    // This means that a search along the parent_scope chain will follow the
    // correct lookup chain for that version of quickboook.
    //
    // symbols contains the templates defined in this scope.
    
    struct template_scope
    {
        template_scope() : parent_scope() {}
        template_scope const* parent_scope;
        template_symbols symbols;
    };

    template_stack::template_stack()
        : scope(template_stack::parser(*this))
        , scopes()
    {
        scopes.push_front(template_scope());
    }
    
    template_stack::~template_stack() {}
    
    template_symbol const* template_stack::prefix_find(iterator& first, iterator const& last) const
    {
        // search all scopes for the longest matching symbol.
        iterator found = first;
        template_symbol const* result = 0;
        for (template_scope const* i = &*scopes.begin(); i; i = i->parent_scope)
        {
            iterator iter = first;
            template_symbol const* symbol = i->symbols.prefix_find(iter, last);
            if(symbol && iter.base() > found.base())
            {
                found = iter;
                result = symbol;
            }
        }
        first = found;
        return result;
    }

    template_symbol const* template_stack::find(std::string const& symbol) const
    {
        for (template_scope const* i = &*scopes.begin(); i; i = i->parent_scope)
        {
            if (template_symbol const* ts = i->symbols.find(symbol.c_str()))
                return ts;
        }
        return 0;
    }

    template_symbol const* template_stack::find_top_scope(std::string const& symbol) const
    {
        return scopes.front().symbols.find(symbol.c_str());
    }

    template_scope const& template_stack::top_scope() const
    {
        BOOST_ASSERT(!scopes.empty());
        return scopes.front();
    }
        
    bool template_stack::add(
            define_template const& definition,
            template_scope const* parent)
    {
        BOOST_ASSERT(!scopes.empty());

        if (this->find_top_scope(definition.id)) {
            return false;
        }

        template_symbol ts(
            definition.id,
            definition.params,
            definition.body,
            parent ? parent : &top_scope());

        scopes.front().symbols.add(ts.identifier.c_str(), ts);
        
        return true;
    }

    void template_stack::push()
    {
        template_scope const& old_front = scopes.front();
        scopes.push_front(template_scope());
        set_parent_scope(old_front);
    }

    void template_stack::pop()
    {
        scopes.pop_front();
    }

    void template_stack::set_parent_scope(template_scope const& parent)
    {
        scopes.front().parent_scope = &parent;
    }

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
            std::vector<template_value>& args
          , std::vector<std::string> const& params
          , file_position const& pos
        )
        {
            // Quickbook 1.4-: If there aren't enough parameters seperated by
            //                 '..' then seperate the last parameter using
            //                 whitespace.
            // Quickbook 1.5+: If '..' isn't used to seperate the parameters
            //                 then use whitespace to separate them
            //                 (2 = template name + argument).

            if (qbk_version_n < 105 || args.size() == 1)
            {
                while (args.size() < params.size() )
                {
                    // Try to break the last argument at the first space found
                    // and push it into the back of args. Do this
                    // recursively until we have all the expected number of
                    // arguments, or if there are no more spaces left.

                    template_value& str = args.back();
                    std::string::size_type l_pos = find_first_seperator(str.content);
                    if (l_pos == std::string::npos)
                        break;
                    template_value first(
                        str.position,
                        std::string(str.content.begin(), str.content.begin() + l_pos)
                        );
                    std::string::size_type r_pos = str.content.find_first_not_of(" \t\r\n", l_pos);
                    if (r_pos == std::string::npos)
                        break;
                    // TODO: Work out position?
                    template_value second(
                        str.position,
                        std::string(str.content.begin()+r_pos, str.content.end())
                    );
                    str = first;
                    args.push_back(second);
                }
            }

            if (args.size() != params.size())
            {
                detail::outerr(pos.file, pos.line)
                    << "Invalid number of arguments passed. Expecting: "
                    << params.size()
                    << " argument(s), got: "
                    << args.size()
                    << " argument(s) instead."
                    << std::endl;
                return false;
            }
            return true;
        }

        std::pair<bool, std::vector<std::string>::const_iterator>
        get_arguments(
            std::vector<template_value>& args
          , std::vector<std::string> const& params
          , template_scope const& scope
          , file_position const& pos
          , quickbook::state& state
        )
        {
            std::vector<template_value>::const_iterator arg = args.begin();
            std::vector<std::string>::const_iterator tpl = params.begin();
            std::vector<std::string> empty_params;

            // Store each of the argument passed in as local templates:
            while (arg != args.end())
            {
                if (!state.templates.add(
                        define_template(*tpl, empty_params, *arg),
                        &scope))
                {
                    detail::outerr(pos.file,pos.line)
                        << "Duplicate Symbol Found" << std::endl;
                    ++state.error_count;
                    return std::make_pair(false, tpl);
                }
                ++arg; ++tpl;
            }
            return std::make_pair(true, tpl);
        }

        bool parse_template(
            std::string body
          , std::string& result
          , file_position const& template_pos
          , bool template_escape
          , quickbook::state& state
        )
        {
            // How do we know if we are to parse the template as a block or
            // a phrase? We apply a simple heuristic: if the body starts with
            // a newline, then we regard it as a block, otherwise, we parse
            // it as a phrase.
            
            body.reserve(body.size() + 2);

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
                quickbook::actions actions(state);
                simple_phrase_grammar phrase_p(actions);

                //  do a phrase level parse
                iterator first(body.begin(), body.end(), state.filename.native_file_string().c_str());
                first.set_position(template_pos);
                iterator last(body.end(), body.end());
                r = boost::spirit::qi::parse(first, last, phrase_p) && first == last;
                state.phrase.swap(result);
            }
            else
            {
                quickbook::actions actions(state);
                block_grammar block_p(actions);

                //  do a block level parse
                //  ensure that we have enough trailing newlines to eliminate
                //  the need to check for end of file in the grammar.
                body += "\n\n";
                while (iter != body.end() && ((*iter == '\r') || (*iter == '\n')))
                    ++iter; // skip initial newlines
                iterator first(iter, body.end(), state.filename.native_file_string().c_str());
                first.set_position(template_pos);
                iterator last(body.end(), body.end());
                r = boost::spirit::qi::parse(first, last, block_p) && first == last;
                state.phrase.swap(result);
            }
            return r;
        }
    }

    std::string process(quickbook::state& state, call_template const& x)
    {
        ++state.template_depth;
        if (state.template_depth > state.max_template_depth)
        {
            detail::outerr(x.position.file, x.position.line)
                << "Infinite loop detected" << std::endl;
            --state.template_depth;
            ++state.error_count;
            return "";
        }

        // The template arguments should have the scope that the template was
        // called from, not the template's own scope.
        //
        // Note that for quickbook 1.4- this value is just ignored when the
        // arguments are expanded.
        template_scope const& call_scope = state.templates.top_scope();

        std::string result;
        state.push(); // scope the state
        {
            // Quickbook 1.4-: When expanding the tempalte continue to use the
            //                 current scope (the dynamic scope).
            // Quickbook 1.5+: Use the scope the template was defined in
            //                 (the static scope).
            if (qbk_version_n >= 105)
                state.templates.set_parent_scope(*x.symbol->parent);

            std::vector<template_value> args = x.args;
    
            ///////////////////////////////////
            // Break the arguments
            if (!break_arguments(args, x.symbol->params, x.position))
            {
                state.pop(); // restore the state
                --state.template_depth;
                ++state.error_count;
                return "";
            }

            ///////////////////////////////////
            // Prepare the arguments as local templates
            bool get_arg_result;
            std::vector<std::string>::const_iterator tpl;
            boost::tie(get_arg_result, tpl) =
                get_arguments(args, x.symbol->params,
                    call_scope, x.position, state);

            if (!get_arg_result)
            {
                state.pop(); // restore the state
                --state.template_depth;
                return "";
            }

            ///////////////////////////////////
            // parse the template body:

            if (!parse_template(x.symbol->body.content, result, x.symbol->body.position, x.escape, state))
            {
                detail::outerr(x.position.file,x.position.line)
                    << "Expanding template:" << x.symbol->identifier << std::endl
                    << std::endl
                    << "------------------begin------------------" << std::endl
                    << x.symbol->body.content
                    << "------------------end--------------------" << std::endl
                    << std::endl;
                state.pop(); // restore the state
                --state.template_depth;
                ++state.error_count;
                return "";
            }
        }

        state.pop(); // restore the state
        --state.template_depth;
        
        return result;
    }
}


