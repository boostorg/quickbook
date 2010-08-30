/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/lexical_cast.hpp>
#include "template.hpp"
#include "phrase_actions.hpp"
#include "grammar.hpp"
#include "state.hpp"
#include "utils.hpp"

#ifdef BOOST_MSVC
#pragma warning(disable : 4355)
#endif

namespace quickbook
{
    namespace detail
    {
        int callout_id = 0;
    }

    struct template_symbol
    {
        template_symbol(
                std::string const& identifier,
                std::vector<std::string> const& params,
                template_body const& body,
                quickbook::callouts const& callouts,
                template_scope const* parent)
           : identifier(identifier)
           , params(params)
           , body(body)
           , callouts(callouts)
           , parent(parent) {}

        std::string identifier;
        std::vector<std::string> params;
        template_body body;
        quickbook::callouts callouts;
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
        boost::scoped_ptr<template_scope> next;
    };

    template_stack::template_stack()
        : scope(template_stack::parser(*this))
        , top_scope(new template_scope())
    {
    }
    
    template_stack::~template_stack() {}
    
    template_symbol const* template_stack::prefix_find(iterator& first, iterator const& last) const
    {
        // search all scopes for the longest matching symbol.
        iterator found = first;
        template_symbol const* result = 0;
        for (template_scope const* i = top_scope.get(); i; i = i->parent_scope)
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
        for (template_scope const* i = top_scope.get(); i; i = i->parent_scope)
        {
            if (template_symbol const* ts = i->symbols.find(symbol.c_str()))
                return ts;
        }
        return 0;
    }

    template_symbol const* template_stack::find_top_scope(std::string const& symbol) const
    {
        return top_scope->symbols.find(symbol.c_str());
    }

    bool template_stack::add(
            define_template const& definition,
            template_scope const* parent)
    {
        BOOST_ASSERT(top_scope);

        if (this->find_top_scope(definition.id)) {
            return false;
        }

        template_symbol ts(
            definition.id,
            definition.params,
            definition.body,
            definition.callouts,
            parent);

        top_scope->symbols.add(ts.identifier.c_str(), ts);
        
        return true;
    }

    void template_stack::push()
    {
        boost::scoped_ptr<template_scope> new_scope(
            new template_scope());
        new_scope->parent_scope = top_scope.get();

        new_scope->next.swap(top_scope);
        new_scope.swap(top_scope);
    }

    void template_stack::pop()
    {
        boost::scoped_ptr<template_scope> popped_scope;
        popped_scope.swap(top_scope);
        popped_scope->next.swap(top_scope);
    }

    namespace
    {
        iterator find_bracket_end(iterator begin, iterator const& end)
        {
            unsigned int depth = 1;

            while(depth > 0) {
                char const* search_chars = "[]\\";
                begin = std::find_first_of(begin, end, search_chars, search_chars + 3);
                if(begin == end) return begin;

                if(*begin == '\\')
                {
                    if(++begin == end) return begin;
                    ++begin;
                }
                else
                {
                    depth += (*begin == '[') ? 1 : -1;
                    ++begin;
                }
            }

            return begin;
        }

        iterator find_first_seperator(iterator const& begin, iterator const& end)
        {
            if(qbk_version_n < 105) {
                char const* whitespace = " \t\r\n";
                return std::find_first_of(begin, end, whitespace, whitespace + 4);
            }
            else {
                iterator pos = begin;

                while(true)
                {
                    char const* search_chars = " \t\r\n\\[";
                    pos = std::find_first_of(pos, end, search_chars, search_chars + 6);
                    if(pos == end) return pos;

                    switch(*pos)
                    {
                    case '[':
                        pos = find_bracket_end(++pos, end);
                        break;
                    case '\\':
                        if(++pos == end) return pos;
                        ++pos;
                        break;
                    default:
                        return pos;
                    }
                }
            }
        }
    
        bool break_arguments(
            std::vector<template_body>& args
          , std::vector<std::string> const& params
          , boost::spirit::classic::file_position const& pos
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
           
                while (args.size() < params.size())
                {
                    // Try to break the last argument at the first space found
                    // and push it into the back of args. Do this
                    // recursively until we have all the expected number of
                    // arguments, or if there are no more spaces left.

                    template_body& body = args.back();
                    iterator begin(body.content.begin(), body.content.end(), body.position.file);
                    iterator end(body.content.end(), body.content.end());
                    
                    iterator l_pos = find_first_seperator(begin, end);
                    if (l_pos == end)
                        break;
                    char const* whitespace = " \t\r\n";
                    char const* whitespace_end = whitespace + 4;
                    iterator r_pos = l_pos;
                    while(r_pos != end && std::find(whitespace, whitespace_end, *r_pos) != whitespace_end) ++r_pos;
                    if (r_pos == end)
                        break;
                    template_body second(std::string(r_pos, end), begin.get_position(), false);
                    body.content = std::string(begin, l_pos);
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
            std::vector<template_body>& args
          , std::vector<std::string> const& params
          , template_scope const& scope
          , file_position const& pos
          , quickbook::state& state
        )
        {
            std::vector<template_body>::const_iterator arg = args.begin();
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
            template_body const& body
          , bool escape
          , std::string& result
          , quickbook::state& state
        )
        {
            // How do we know if we are to parse the template as a block or
            // a phrase? We apply a simple heuristic: if the body starts with
            // a newline, then we regard it as a block, otherwise, we parse
            // it as a phrase.
            //
            // Note: this is now done in the grammar.
            
            if (escape)
            {
                //  escape the body of the template
                //  we just copy out the literal body
                result = body.content;
                return true;
            }
            else if (!body.is_block)
            {
                quickbook::actions actions(state);
                quickbook_grammar g(actions);

                //  do a phrase level parse
                iterator first(body.content.begin(), body.content.end(), body.position);
                iterator last(body.content.end(), body.content.end());
                bool r = boost::spirit::qi::parse(first, last, g.simple_phrase) && first == last;
                //  do a phrase level parse
                std::string phrase;
                state.phrase.swap(phrase);
                result = phrase;
                return r;
            }
            else
            {
                quickbook::actions actions(state);
                quickbook_grammar g(actions, true);

                //  do a block level parse
                //  ensure that we have enough trailing newlines to eliminate
                //  the need to check for end of file in the grammar.
                
                std::string content = body.content + "\n\n";
                iterator first(content.begin(), content.end(), body.position);
                iterator last(content.end(), content.end());
                bool r = boost::spirit::qi::parse(first, last, g.block) && first == last;
                state.paragraph_output();
                std::string block;
                state.block.swap(block);
                result = block;
                return r;                
            }
        }
    }

    void process(quickbook::state& state, call_template const& x)
    {
        ++state.template_depth;
        if (state.template_depth > state.max_template_depth)
        {
            detail::outerr(x.position.file, x.position.line)
                << "Infinite loop detected" << std::endl;
            --state.template_depth;
            ++state.error_count;
            return;
        }

        // The template arguments should have the scope that the template was
        // called from, not the template's own scope.
        //
        // Note that for quickbook 1.4- this value is just ignored when the
        // arguments are expanded.
        template_scope const& call_scope = *state.templates.top_scope;

        std::string result;
        state.push(); // scope the state
        {
            // Store the current section level so that we can ensure that
            // [section] and [endsect] tags in the template are balanced.
            state.min_section_level = state.section_level;

            // Quickbook 1.4-: When expanding the tempalte continue to use the
            //                 current scope (the dynamic scope).
            // Quickbook 1.5+: Use the scope the template was defined in
            //                 (the static scope).
            if (qbk_version_n >= 105)
                state.templates.top_scope->parent_scope = x.symbol->parent;

            std::vector<template_body> args = x.args;
    
            ///////////////////////////////////
            // Initialise the arguments
            
            if (!x.symbol->callouts.size())
            {
                // Break the arguments for a template
            
                if (!break_arguments(args, x.symbol->params, x.position))
                {
                    state.pop(); // restore the state
                    --state.template_depth;
                    ++state.error_count;
                    return;
                }
            }
            else
            {
                if (!args.empty())
                {
                    detail::outerr(x.position.file, x.position.line)
                        << "Arguments for code snippet."
                        <<std::endl;
                    ++state.error_count;

                    args.clear();
                }

                unsigned int size = x.symbol->params.size();

                for(unsigned int i = 0; i < size; ++i)
                {
                    std::string callout_id = state.doc_id.value +
                        boost::lexical_cast<std::string>(detail::callout_id + i);

                    std::string code;
                    code += "[[callout]";
                    code += x.symbol->callouts[i].role;
                    code += " ";
                    code += callout_id;
                    code += "]";

                    args.push_back(template_body(code, x.position, false));
                }
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
                return;
            }

            ///////////////////////////////////
            // parse the template body:

            if (!parse_template(x.symbol->body, x.escape, result, state))
            {
                detail::outerr(x.position.file,x.position.line)
                    << "Expanding "
                    << (x.symbol->body.is_block ? "block" : "phrase")
                    << " template:" << x.symbol->identifier << std::endl
                    << std::endl
                    << "------------------begin------------------" << std::endl
                    << x.symbol->body.content
                    << "------------------end--------------------" << std::endl
                    << std::endl;
                state.pop(); // restore the state
                --state.template_depth;
                ++state.error_count;
                return;
            }

            if (state.section_level != state.min_section_level)
            {
                detail::outerr(x.position.file,x.position.line)
                    << "Mismatched sections in template " << x.symbol->identifier << std::endl;
                state.pop(); // restore the actions' states
                --state.template_depth;
                ++state.error_count;
                return; 
            }
        }

        state.pop(); // restore the state

        if(x.symbol->callouts.size()) {
            callout_list list;
            BOOST_FOREACH(callout_source const& c, x.symbol->callouts) {
                std::string callout_id = state.doc_id.value +
                    boost::lexical_cast<std::string>(detail::callout_id++);

                std::string callout_value;
                state.push();
                // TODO: adjust the position?
                bool r = parse_template(c.body, false, callout_value, state);
                state.pop();

                if(!r)
                {
                    detail::outerr(c.body.position.file, c.body.position.line)
                        << "Expanding callout."
                        << std::endl;
                    --state.template_depth;
                    ++state.error_count;
                    return;
                }

                callout_item item;
                item.identifier = callout_id;
                item.content = callout_value;
                list.push_back(item);
            }

            state.push();
            {
                quickbook::actions actions(state);
                actions.process(list);
            }
            result += state.block.str();
            state.pop();
        }

        --state.template_depth;

        if(x.symbol->body.is_block) {
            state.paragraph_output();
            state.block << result;
        }
        else {
            state.phrase << result;
        }
    }
}
