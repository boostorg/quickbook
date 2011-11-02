/*=============================================================================
    Copyright (c) 2011 Daniel James

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include "id_manager.hpp"
#include "utils.hpp"
#include "string_ref.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>
#include <deque>
#include <vector>

namespace quickbook
{
    //
    // Forward declarations
    //

    struct id_placeholder;
    struct id_data;
    std::string process_ids(id_state&, std::string const&);

    static const std::size_t max_size = 32;

    //
    // id_placeholder
    //

    struct id_placeholder
    {
        enum state_enum { child, unresolved, resolved, generated };

        unsigned index;         // The poisition in the placeholder deque.
        state_enum state;       // Placeholder's position in generation
                                // process.
        std::string id;         // The id so far.
        id_placeholder* parent; // Placeholder of the parent id.
                                // Only when state == child
        id_category category;
        unsigned level;         // Level in the document.
                                // 0 = doc_id
                                // 1 = id directly in doc_id or
                                // first level of section.
        unsigned order;         // Order of the placeholders in the generated
                                // xml. Stored because it can be slightly
                                // different to the order they're generated
                                // in. e.g. for nested tables the cells
                                // are processed before the title id.
                                // Only set when processing ids.
        id_data* data;          // Assigned data shared by duplicate ids
                                // used to detect them. Only when
                                // state == resolved

        id_placeholder(
                unsigned index,
                std::string const& id,
                id_category category,
                id_placeholder* parent_ = 0)
          : index(index),
            state(parent_ ? child : unresolved),
            id(id),
            parent(parent_),
            category(category),
            level(boost::range::count(id, '.') +
                (parent_ ? parent_->level + 1 : 0)),
            order(0),
            data(0)
        {
        }

        std::string to_string()
        {
            return '$' + boost::lexical_cast<std::string>(index);
        }

        bool check_state() const
        {
            return (
                (state == child) == (bool) parent &&
                (state == resolved) == (bool) data);
        }

        bool check_state(state_enum s) const
        {
            return s == state && check_state();
        }
    };

    //
    // id_state
    //

    struct id_state
    {
        std::deque<id_placeholder> placeholders;

        id_placeholder* add_placeholder(
                std::string const&, id_category, id_placeholder* parent = 0);

        id_placeholder* get_placeholder(string_ref);
    };

    id_placeholder* id_state::add_placeholder(
            std::string const& id, id_category category,
            id_placeholder* parent)
    {
        placeholders.push_back(id_placeholder(
            placeholders.size(), id, category, parent));
        return &placeholders.back();
    }

    id_placeholder* id_state::get_placeholder(string_ref value)
    {
        // If this isn't a placeholder id.
        if (value.size() <= 1 || *value.begin() != '$')
            return 0;

        unsigned index = boost::lexical_cast<int>(std::string(
                value.begin() + 1, value.end()));

        return &placeholders.at(index);
    }

    //
    // section_manager
    //

    struct section_manager
    {
        section_manager(
                id_state& ids,
                unsigned qbk_version)
          : ids(ids),
            qbk_version(qbk_version),
            level(0)
        {}

        virtual ~section_manager() {
            assert(level <= 1);
        }

        virtual std::auto_ptr<section_manager> start_file(
                bool have_docinfo,
                unsigned qbk_version,
                std::string const& include_doc_id,
                std::string const& id,
                std::string const& title,
                std::string* doc_id_result,
                std::string* placeholder) = 0;

        virtual std::string docinfo(
                std::string const& id = std::string(),
                id_category = id_category()) = 0;

        virtual bool end_file() = 0;

        virtual std::string add_id(
                std::string const&,
                id_category) = 0;

        virtual std::string old_style_id(
                std::string const&,
                id_category) = 0;

        virtual std::string begin_section(
                std::string const&,
                id_category) = 0;

        virtual void end_section() = 0;

        id_state& ids;
        unsigned qbk_version;
        int level;
        boost::scoped_ptr<section_manager> parent;
    };

    //
    // id_manager implementation
    //

    namespace {
        std::auto_ptr<section_manager> create_section_manager(
                id_state& ids,
                unsigned qbk_version);
    }

    id_manager::id_manager()
      : state(new id_state),
        current_section(create_section_manager(*state, 0))
    {
    }

    id_manager::~id_manager() {}

    void id_manager::start_file(
            std::string const& include_doc_id,
            std::string const& id,
            std::string const& title)
    {
        boost::scoped_ptr<section_manager> new_section(
            current_section->start_file(false, current_section->qbk_version,
                include_doc_id, id, title, 0, 0));

        if (new_section) {
            boost::swap(current_section, new_section->parent);
            boost::swap(current_section, new_section);
        }
    }

    id_manager::start_file_info id_manager::start_file_with_docinfo(
            unsigned qbk_version,
            std::string const& include_doc_id,
            std::string const& id,
            std::string const& title)
    {
        start_file_info result;

        boost::scoped_ptr<section_manager> new_section(
            current_section->start_file(true, qbk_version,
                include_doc_id, id, title,
                &result.doc_id, &result.placeholder));

        if (new_section) {
            boost::swap(current_section, new_section->parent);
            boost::swap(current_section, new_section);
        }

        return result;
    }

    void id_manager::end_file()
    {
        if (current_section->end_file()) {
            boost::scoped_ptr<section_manager> old_section;
            boost::swap(current_section, old_section);
            boost::swap(current_section, old_section->parent);
        }
    }

    std::string id_manager::begin_section(std::string const& id, id_category category)
    {
        // Shouldn't begin_section without calling docinfo first.
        assert(current_section->level > 0);
        return current_section->begin_section(id, category);
    }

    void id_manager::end_section()
    {
        assert(current_section->level > 1);
        return current_section->end_section();
    }

    int id_manager::section_level() const
    {
        return current_section->level;
    }

    std::string id_manager::old_style_id(std::string const& id, id_category category)
    {
        return current_section->old_style_id(id, category);
    }

    std::string id_manager::add_id(std::string const& id, id_category category)
    {
        return current_section->add_id(id, category);
    }

    std::string id_manager::add_anchor(std::string const& id, id_category category)
    {
        return state->add_placeholder(id, category)->to_string();
    }

    std::string id_manager::replace_placeholders(std::string const& xml) const
    {
        assert(current_section && !current_section->parent);
        return process_ids(*state, xml);
    }

    //
    // normalize_id
    //
    // Normalizes generated ids.
    //

    namespace
    {
        std::string normalize_id(
                std::string src_id,
                std::size_t prefix = 0,
                std::size_t size = max_size)
        {
            std::string id;
            id.swap(src_id);

            std::size_t src = prefix;
            std::size_t dst = prefix;
            size += prefix;

            while (src < id.length() && dst < size) {
                if (id[src] == '_') {
                    do {
                        ++src;
                    } while(src < id.length() && id[src] == '_');

                    if (src < id.length()) id[dst++] = '_';
                }
                else {
                    id[dst++] = id[src++];
                }
            }

            id.erase(dst);

            return id;
        }
    }

    //
    // section_manager_1_1
    //
    // Handles sections for quickbook 1.1 to 1.5
    //

    struct section_manager_1_1 : section_manager
    {
        section_manager_1_1(
                id_state& ids,
                unsigned qbk_version)
          : section_manager(ids, qbk_version),
            doc_id(),
            section_id(),
            qualified_id()
        {}

        virtual std::auto_ptr<section_manager> start_file(
                bool have_docinfo,
                unsigned qbk_version,
                std::string const& include_doc_id,
                std::string const& id,
                std::string const& title,
                std::string* doc_id_result,
                std::string* placeholder);

        virtual std::string docinfo(
                std::string const& id,
                id_category);

        virtual bool end_file();

        virtual std::string add_id(
                std::string const&,
                id_category);

        virtual std::string old_style_id(
                std::string const&,
                id_category);

        virtual std::string begin_section(
                std::string const&,
                id_category);

        virtual void end_section();

        std::string doc_id;
        std::string section_id;
        std::string qualified_id;
        std::vector<std::string> doc_id_stack;
    };

    std::auto_ptr<section_manager> section_manager_1_1::start_file(
            bool have_docinfo,
            unsigned qbk_version,
            std::string const& include_doc_id,
            std::string const& id,
            std::string const& title,
            std::string* doc_id_result,
            std::string* placeholder)
    {
        std::string initial_doc_id =
            !id.empty() ? id :
            !include_doc_id.empty() ? include_doc_id :
            detail::make_identifier(title);

        id_category category =
            !id.empty() || !include_doc_id.empty() ?
                id_category::explicit_id :
                id_category::generated_doc;

        if (doc_id_result) *doc_id_result = initial_doc_id;

        if (have_docinfo) {
            std::auto_ptr<section_manager> new_section_manager =
                create_section_manager(ids, qbk_version);
            std::string initial_placeholder = new_section_manager->docinfo(
                initial_doc_id, category);
            if (placeholder) *placeholder = initial_placeholder;
            return new_section_manager;
        }
        else {
            doc_id_stack.push_back(doc_id);
            doc_id = initial_doc_id;
            if (placeholder) *placeholder = "";
            return std::auto_ptr<section_manager>();
        }
    }

    std::string section_manager_1_1::docinfo(
        std::string const& id,
        id_category category)
    {
        doc_id = id;
        ++level;
        return ids.add_placeholder(id, category)->to_string();
    }

    bool section_manager_1_1::end_file()
    {
        // If there are not stacked doc_ids then this is the end
        // of the document.
        if (doc_id_stack.empty()) return true;

        doc_id = doc_id_stack.back();
        doc_id_stack.pop_back();
        return false;
    }

    std::string section_manager_1_1::add_id(
            std::string const& id,
            id_category category)
    {
        std::string new_id = doc_id;
        if (!new_id.empty() && !qualified_id.empty()) new_id += '.';
        new_id += qualified_id;
        if (!new_id.empty() && !id.empty()) new_id += '.';
        new_id += id;
        return ids.add_placeholder(new_id, category)->to_string();
    }

    std::string section_manager_1_1::old_style_id(
        std::string const& id,
        id_category category)
    {
        return qbk_version < 103 ?
            ids.add_placeholder(section_id + "." + id, category)->to_string() :
            add_id(id, category);
    }

    std::string section_manager_1_1::begin_section(
            std::string const& id,
            id_category category)
    {
        if (level > 1) qualified_id += '.';
        qualified_id += id;
        section_id = id;
        ++level;
        return (qbk_version < 103u ?
            ids.add_placeholder(doc_id + "." + id, category) :
            ids.add_placeholder(doc_id + "." + qualified_id, category))->to_string();
    }

    void section_manager_1_1::end_section()
    {
        // Note: This doesn't change section_id here - in order to
        // be bug compatible with early versions of quickbook.

        --level;
        if (level == 1)
        {
            qualified_id.clear();
        }
        else
        {
            std::string::size_type const n =
                qualified_id.find_last_of('.');
            qualified_id.erase(n, std::string::npos);
        }
    }

    //
    // section_manager_1_6
    //
    // Handles sections for quickbook 1.6+
    //

    struct section_manager_1_6 : section_manager
    {
        section_manager_1_6(
                id_state& ids,
                unsigned qbk_version)
          : section_manager(ids, qbk_version),
            current_placeholder(0),
            depth(0)
        {}

        virtual std::auto_ptr<section_manager> start_file(
                bool have_docinfo,
                unsigned qbk_version,
                std::string const& include_doc_id,
                std::string const& id,
                std::string const& title,
                std::string* doc_id_result,
                std::string* placeholder);

        virtual std::string docinfo(
                std::string const& id,
                id_category);

        virtual bool end_file();

        virtual std::string add_id(
                std::string const&,
                id_category);

        virtual std::string old_style_id(
                std::string const&,
                id_category);

        virtual std::string begin_section(
                std::string const&,
                id_category);

        virtual void end_section();

        id_placeholder* current_placeholder;
        unsigned depth;
    };

    std::auto_ptr<section_manager> section_manager_1_6::start_file(
            bool have_docinfo,
            unsigned qbk_version,
            std::string const& include_doc_id,
            std::string const& id,
            std::string const& title,
            std::string* doc_id_result,
            std::string* placeholder)
    {
        if (have_docinfo || !include_doc_id.empty()) {
            std::string initial_doc_id =
                !include_doc_id.empty() ? include_doc_id :
                !id.empty() ? id :
                detail::make_identifier(title);

            id_category category =
                !include_doc_id.empty() || !id.empty() ?
                    id_category::explicit_id :
                    id_category::generated_doc;

            if (doc_id_result) *doc_id_result = initial_doc_id;

            std::auto_ptr<section_manager> new_section_manager =
                create_section_manager(ids, qbk_version);
            std::string initial_placeholder = new_section_manager->docinfo(
                initial_doc_id, category);
            if (placeholder) *placeholder = initial_placeholder;
            return new_section_manager;
        }
        else {
            ++depth;
            if (doc_id_result) *doc_id_result = "";
            if (placeholder) *placeholder = "";
            return std::auto_ptr<section_manager>();
        }
    }

    std::string section_manager_1_6::docinfo(
        std::string const& id,
        id_category category)
    {
        return begin_section(id, category);
    }

    bool section_manager_1_6::end_file()
    {
        if (depth == 0) {
            end_section();
            assert(!current_placeholder);
            return true;
        }
        else {
            --depth;
            return false;
        }
    }

    std::string section_manager_1_6::add_id(
            std::string const& id,
            id_category category)
    {
        return ids.add_placeholder(
            category.c == id_category::explicit_id ? id : normalize_id(id),
            category, current_placeholder)->to_string();
    }

    std::string section_manager_1_6::old_style_id(
        std::string const& id,
        id_category category)
    {
        return add_id(id, category);
    }

    std::string section_manager_1_6::begin_section(
            std::string const& id,
            id_category category)
    {
        ++level;

        current_placeholder = ids.add_placeholder(
            category.c == id_category::explicit_id ? id : normalize_id(id),
            category, current_placeholder);

        return current_placeholder->to_string();
    }

    void section_manager_1_6::end_section()
    {
        --level;
        current_placeholder = current_placeholder->parent;
    }

    //
    // create_section_manager
    //

    namespace {
        std::auto_ptr<section_manager> create_section_manager(
                id_state& ids,
                unsigned qbk_version)
        {
            return std::auto_ptr<section_manager>(qbk_version < 106u ?
                (section_manager*)(new section_manager_1_1(ids, qbk_version)) :
                (section_manager*)(new section_manager_1_6(ids, qbk_version)));
        }
    }

    //
    // Xml subset parser used for finding id values.
    //
    // I originally tried to integrate this into the post processor
    // but that proved tricky. Alternatively it could use a proper
    // xml parser, but I want this to be able to survive badly
    // marked up escapes.
    //

    struct xml_processor
    {
        xml_processor();

        std::vector<std::string> id_attributes;

        struct callback {
            virtual void start(string_ref) {}
            virtual void id_value(string_ref) {}
            virtual void finish(string_ref) {}
            virtual ~callback() {}
        };

        void parse(std::string const&, callback&);
    };

    namespace
    {
        char const* id_attributes_[] =
        {
            "id",
            "linkend",
            "linkends",
            "arearefs"
        };
    }

    xml_processor::xml_processor()
    {
        static int const n_id_attributes = sizeof(id_attributes_)/sizeof(char const*);
        for (int i = 0; i != n_id_attributes; ++i)
        {
            id_attributes.push_back(id_attributes_[i]);
        }

        boost::sort(id_attributes);
    }

    template <typename Iterator>
    bool read(Iterator& it, Iterator end, char const* text)
    {
        for(Iterator it2 = it;; ++it2, ++text) {
            if (!*text) {
                it = it2;
                return true;
            }

            if (it2 == end || *it2 != *text)
                return false;
        }
    }

    template <typename Iterator>
    void read_past(Iterator& it, Iterator end, char const* text)
    {
        while (it != end && !read(it, end, text)) ++it;
    }

    bool find_char(char const* text, char c)
    {
        for(;*text; ++text)
            if (c == *text) return true;
        return false;
    }

    template <typename Iterator>
    void read_some_of(Iterator& it, Iterator end, char const* text)
    {
        while(it != end && find_char(text, *it)) ++it;
    }

    template <typename Iterator>
    void read_to_one_of(Iterator& it, Iterator end, char const* text)
    {
        while(it != end && !find_char(text, *it)) ++it;
    }

    void xml_processor::parse(std::string const& source, callback& c)
    {
        typedef std::string::const_iterator iterator;

        string_ref source_ref(source.begin(), source.end());
        c.start(source_ref);

        iterator it = source_ref.begin(), end = source_ref.end();

        for(;;)
        {
            read_past(it, end, "<");
            if (it == end) break;

            if (read(it, end, "!--quickbook-escape-prefix-->"))
            {
                read_past(it, end, "<!--quickbook-escape-postfix-->");
                continue;
            }

            switch(*it)
            {
            case '?':
                ++it;
                read_past(it, end, "?>");
                break;

            case '!':
                if (read(it, end, "!--"))
                    read_past(it, end, "-->");
                else
                    read_past(it, end, ">");
                break;

            default:
                if ((*it >= 'a' && *it <= 'z') ||
                        (*it >= 'A' && *it <= 'Z') ||
                        *it == '_' || *it == ':')
                {
                    read_to_one_of(it, end, " \t\n\r>");

                    for (;;) {
                        read_some_of(it, end, " \t\n\r");
                        iterator name_start = it;
                        read_to_one_of(it, end, "= \t\n\r>");
                        if (it == end || *it == '>') break;
                        string_ref name(name_start, it);
                        ++it;

                        read_some_of(it, end, "= \t\n\r");
                        if (it == end || (*it != '"' && *it != '\'')) break;

                        char delim = *it;
                        ++it;

                        iterator value_start = it;

                        it = std::find(it, end, delim);
                        if (it == end) break;
                        string_ref value(value_start, it);
                        ++it;

                        if (boost::find(id_attributes, name)
                                != id_attributes.end())
                        {
                            c.id_value(value);
                        }
                    }
                }
                else
                {
                    read_past(it, end, ">");
                }
            }
        }

        c.finish(source_ref);
    }

    //
    // process_ids
    //

    typedef boost::unordered_map<std::string, id_data> allocated_ids;
    typedef std::vector<id_placeholder*> placeholder_index;

    placeholder_index index_placeholders(id_state&, std::string const& xml);
    void resolve_id(id_placeholder&, allocated_ids&);
    void generate_id(id_placeholder&, allocated_ids&);
    std::string replace_ids(id_state& state, std::string const& xml);

    std::string process_ids(id_state& state, std::string const& xml)
    {
        placeholder_index placeholders = index_placeholders(state, xml);

        typedef std::vector<id_placeholder*>::iterator iterator;

        iterator it = placeholders.begin(),
            end = placeholders.end();

        // Placeholder ids are processed in blocks of ids with
        // an equal level.
        while (it != end) {
            unsigned level = (*it)->level;

            // ids can't clash with ids at a different level, so
            // this only needs to track the id generation data
            // for a single level at a time.
            allocated_ids ids;

            iterator it2 = it;
            do {
                resolve_id(**it2++, ids);
            } while(it2 != end && (*it2)->level == level);

            do {
                generate_id(**it++, ids);
            } while(it != it2);
        }

        return replace_ids(state, xml);
    }

    //
    // index_placeholders
    //
    // Create a sorted index of the placeholders, in ordered
    // to make numbering duplicates easy.
    //

    struct placeholder_compare
    {
        bool operator()(id_placeholder* x, id_placeholder* y) const
        {
            bool x_explicit = x->category.c == id_category::explicit_id;
            bool y_explicit = y->category.c == id_category::explicit_id;

            return
                x->level < y->level ? true :
                x->level > y->level ? false :
                x_explicit > y_explicit ? true :
                x_explicit < y_explicit ? false :
                x->order < y->order;
        }
    };

    struct number_placeholders_callback : xml_processor::callback
    {
        id_state& ids;
        unsigned count;

        number_placeholders_callback(id_state& ids)
          : ids(ids),
            count(0)
        {}

        void id_value(string_ref value)
        {
            id_placeholder* p = ids.get_placeholder(value);
            number(p);
        }

        void number(id_placeholder* p)
        {
            if (p && !p->order) {
                number(p->parent);
                p->order = ++count;
            }
        }
    };

    placeholder_index index_placeholders(
            id_state& state,
            std::string const& xml)
    {
        xml_processor processor;
        number_placeholders_callback callback(state);
        processor.parse(xml, callback);

        placeholder_index sorted_placeholders;
        sorted_placeholders.reserve(state.placeholders.size());
        BOOST_FOREACH(id_placeholder& p, state.placeholders)
            if (p.order) sorted_placeholders.push_back(&p);
        boost::sort(sorted_placeholders, placeholder_compare());

        return sorted_placeholders;
    }

    //
    // Data used for generating placeholders that have duplicates.
    //

    struct id_generation_data
    {
        id_generation_data(std::string const& src_id)
          : child_start(src_id.rfind('.') + 1),
            id(normalize_id(src_id, child_start, max_size - 1)),
                // 'max_size - 1' leaves a character to append
                // a number.
            count(0)
        {
            if (std::isdigit(id[id.length() - 1]))
            {
                if (child_length() < max_size - 1)
                    id += '_';
                else
                    reduce_id();
            }
        }

        void reduce_id()
        {
            assert(id.length() > child_start);
            std::size_t length = id.length() - 1;
            while(length > child_start && std::isdigit(id[length - 1])) --length;
            id.erase(length);
            count = 0;
        }

        std::size_t child_length() const
        {
            return id.length() - child_start;
        }

        std::size_t child_start;
        std::string id;
        int count;
    };

    // Created for all desired ids, either when resolving an id or due to
    // generating a new id to avoid duplicates.
    struct id_data
    {
        id_data()
          : category(id_category::numbered),
            used(false),
            generation_data()
        {}

        void update_category(id_category c)
        {
            if (c.c > category.c) category = c;
        }

        id_category category;   // The highest priority category of the
                                // placeholders that want to use this id.
        bool used;              // Whether this id has been used.
        boost::shared_ptr<id_generation_data> generation_data;
                                // If a duplicates are found, this is
                                // created to generate new ids.
                                //
                                // Many to one relationship, because truncation
                                // can lead to different ids contending for the
                                // same id prefix.
    };

    //
    // resolve_id
    //
    // Convert child ids to full ids, and add to the
    // allocated ids (although not yet set in stone because
    // there might be duplicates).
    //
    // Note that the parent ids has to be generated before resolving
    // the child id.
    //

    void resolve_id(id_placeholder& p, allocated_ids& ids)
    {
        if (p.state == id_placeholder::child)
        {
            assert(p.check_state());
            assert(p.parent->check_state(id_placeholder::generated));

            p.id = p.parent->id + "." + p.id;
            p.state = id_placeholder::unresolved;
            p.parent = 0;
        }

        assert(p.check_state(id_placeholder::unresolved));

        id_data& data = ids.emplace(p.id, id_data()).first->second;
        data.update_category(p.category);

        p.data = &data;
        p.state = id_placeholder::resolved;
    }

    //
    // generate_id
    //
    // Finally generate the final id.
    //

    void register_generation_data(id_placeholder&, allocated_ids&);

    void generate_id(id_placeholder& p, allocated_ids& ids)
    {
        assert(p.check_state(id_placeholder::resolved));

        // If the placeholder id is available, then update data
        // and return.
        if (p.category == p.data->category && !p.data->used &&
            p.category.c != id_category::numbered)
        {
            p.data->used = true;
            p.state = id_placeholder::generated;
            p.data = 0;
            return;
        }

        if (!p.data->generation_data)
        {
            p.data->generation_data.reset(new id_generation_data(p.id));
            register_generation_data(p, ids);
        }

        // Loop until an available id is found.
        for(;;)
        {
            id_generation_data& generation_data = *p.data->generation_data;

            std::string postfix =
                boost::lexical_cast<std::string>(generation_data.count++);

            if (generation_data.child_length() + postfix.length() > max_size) {
                // The resulting id is too long, so move to a shorter id.
                generation_data.reduce_id();
                register_generation_data(p, ids);
            }
            else {
                std::string id = generation_data.id + postfix;

                if (ids.find(id) == ids.end()) {
                    p.id.swap(id);
                    p.state = id_placeholder::generated;
                    p.data = 0;
                    return;
                }
            }
        }
    }

    // Every time the generation id is changed, this is called to
    // check if that id is already in use.
    void register_generation_data(id_placeholder& p, allocated_ids& ids)
    {
        std::string const& id = p.data->generation_data->id;

        id_data& new_data = ids.emplace(id, id_data()).first->second;

        // If there is already generation_data for the new id then use that.
        // Otherwise use the placeholder's existing generation_data.
        if (new_data.generation_data)
            p.data->generation_data = new_data.generation_data;
        else
            new_data.generation_data = p.data->generation_data;
    }

    //
    // replace_ids
    //
    // Return a copy of the xml with all the placeholders replaced by
    // generated_ids.
    //

    struct replace_ids_callback : xml_processor::callback
    {
        id_state& ids;
        std::string::const_iterator source_pos;
        std::string result;

        replace_ids_callback(id_state& ids)
          : ids(ids),
            source_pos(),
            result()
        {}

        void start(string_ref xml)
        {
            source_pos = xml.begin();
        }

        void id_value(string_ref value)
        {
            if (id_placeholder* p = ids.get_placeholder(value))
            {
                assert(p->check_state(id_placeholder::generated));

                result.append(source_pos, value.begin());
                result.append(p->id.begin(), p->id.end());
                source_pos = value.end();
            }
        }

        void finish(string_ref xml)
        {
            result.append(source_pos, xml.end());
            source_pos = xml.end();
        }
    };

    std::string replace_ids(id_state& state, std::string const& xml)
    {
        xml_processor processor;
        replace_ids_callback callback(state);
        processor.parse(xml, callback);
        return callback.result;
    }
}
