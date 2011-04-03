/*=============================================================================
    Copyright (c) 2002 2004 2006 Joel de Guzman
    Copyright (c) 2004 Eric Niebler
    Copyright (c) 2005 Thomas Guest
    http://spirit.sourceforge.net/

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include "actions_class.hpp"
#include "quickbook.hpp"
#include "grammar.hpp"
#include "input_path.hpp"

#if (defined(BOOST_MSVC) && (BOOST_MSVC <= 1310))
#pragma warning(disable:4355)
#endif

namespace quickbook
{
    actions::actions(fs::path const& filein_, fs::path const& xinclude_base_, string_stream& out_)
        : grammar_()

        , doc_type()
        , doc_title_qbk()
        , xinclude_base(xinclude_base_)

        , template_depth(0)
        , templates()
        , error_count(0)
        , anchors()
        , no_eols(true)
        , warned_about_breaks(false)
        , context(0)

        , process_state(process_normal)
        , macro()
        , source_mode("c++")
        , doc_id()
        , filename(filein_)
        , filename_relative(filein_.filename())

        , section_level(0)
        , min_section_level(0)
        , section_id()
        , qualified_section_id()

        , out(out_)
        , phrase()
        , values()

    // actions
        , phrase_value(*this, phrase)
        , out_value(*this, out)
        , docinfo_value(*this)

        , scoped_cond_phrase(*this)
        , scoped_output(*this)
        , scoped_no_eols(*this)
        , scoped_context(*this)
        , scoped_activate_processing(*this)

        , element(*this)
        , error(*this)
        , code(out, phrase, *this)
        , code_block(phrase, phrase, *this)
        , inline_code(phrase, *this)
        , paragraph(*this)
        , space_char(phrase)
        , plain_char(phrase, *this)
        , raw_char(phrase, *this)
        , escape_unicode(phrase, *this)

        , simple_markup(phrase, macro, *this)

        , break_(phrase, *this)
        , do_macro(phrase, *this)

        , element_id_warning(*this)
    {
        // add the predefined macros
        macro.add
            ("__DATE__", std::string(quickbook_get_date))
            ("__TIME__", std::string(quickbook_get_time))
            ("__FILENAME__", detail::path_to_generic(filename_relative))
        ;
        
        boost::scoped_ptr<quickbook_grammar> g(
            new quickbook_grammar(*this));
        grammar_.swap(g);
    }

    quickbook_grammar& actions::grammar() const {
        return *grammar_;
    }

    file_state::file_state(actions& a, scope_flags scope)
        : a(a)
        , scope(scope)
        , doc_id(a.doc_id)
        , filename(a.filename)
        , filename_relative(a.filename_relative)
        , source_mode(a.source_mode)
        , process_state(a.process_state)
        , macro()
    {
        if (scope & scope_macros) macro = a.macro;
        if (scope & scope_templates) a.templates.push();
        a.values.builder.save();
    }

    file_state::~file_state()
    {
        a.values.builder.restore();
        boost::swap(a.doc_id, doc_id);
        boost::swap(a.filename, filename);
        boost::swap(a.filename_relative, filename_relative);
        boost::swap(a.source_mode, source_mode);
        boost::swap(a.process_state, process_state);
        if (scope & scope_templates) a.templates.pop();
        if (scope & scope_macros) a.macro = macro;
    }
    
    template_state::template_state(actions& a)
        : file_state(a, file_state::scope_all)
        , section_level(a.section_level)
        , min_section_level(a.min_section_level)
        , section_id(a.section_id)
        , qualified_section_id(a.qualified_section_id)
    {
        a.out.push();
        a.phrase.push();
    }

    template_state::~template_state()
    {
        a.phrase.pop();
        a.out.pop();
        boost::swap(a.section_level, section_level);
        boost::swap(a.min_section_level, min_section_level);
        boost::swap(a.section_id, section_id);
        boost::swap(a.qualified_section_id, qualified_section_id);
    }
}
