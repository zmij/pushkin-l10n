/*
 * po_generator.cpp
 *
 *  Created on: 23 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <pushkin/l10n/po_generator.hpp>
#include <boost/program_options.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/local_time/local_time_io.hpp>
#include <set>

namespace psst {
namespace l10n {

struct po_entry {
    using strings = ::std::vector<::std::string>;
    ::std::string   id;
    ::std::string   context;
    ::std::string   plural;
    ::std::string   reference;
    strings mutable comments;

    po_entry(message const& msg)
        : id(msg.id())
    {
        if (msg.has_context()) {
            context = msg.context();
        }
        if (msg.has_plural()) {
            plural = msg.plural();
        }
    }

    // TODO Flags
    bool
    operator == (po_entry const& rhs) const
    {
        return id == rhs.id &&
            context == rhs.context &&
            plural.empty() == rhs.plural.empty(); // Plural forms cannot be used to distinguish messages
    }

    bool
    operator < (po_entry const& rhs) const
    {
        if (id < rhs.id)
            return true;
        if (id == rhs.id) {
            if (context < rhs.context)
                return true;
            if (context == rhs.context) {
                // Message with no plural is less than
                // message with plural
                if (plural.empty() == rhs.plural.empty())
                    return false;
                return !plural.empty();
            }
        }
        return false;
    }
};

::std::ostream&
escape_string(::std::ostream& os, ::std::string const& str)
{
    for (auto c : str) {
        if (c == '"')
            os.put('\\');
        os.put(c);
    }
    return os;
}

::std::ostream&
operator << (::std::ostream& os, po_entry const& val)
{
    ::std::ostream::sentry s (os);
    if (s) {
        os << "\n";
        if (!val.comments.empty()) {
            for (auto const& c : val.comments) {
                os << "#. " << c << "\n";
            }
        }
        if (!val.reference.empty()) {
            os << "#: " << val.reference << "\n";
        } else {
            os << "#: Somewhere in a galaxy far, far away\n";
        }
        if (!val.context.empty()) {
            os << "msgctxt \""; escape_string(os, val.context) << "\"\n";
        }
        os << "msgid \""; escape_string(os, val.id) << "\"\n";
        if (!val.plural.empty()) {
            os  << "msgid_plural \""; escape_string(os, val.plural) << "\"\n";
            os  << "msgstr[0] \""; escape_string(os, val.id) << "\"\n";
            os  << "msgstr[1] \""; escape_string(os, val.plural) << "\"\n";
        } else {
            os << "msgstr \""; escape_string(os, val.id) << "\"\n";
        }
    }
    return os;
}


struct po_generator::impl {
    using po_entries            = ::std::set<po_entry>;
    using options_description   = ::boost::program_options::options_description;
    using time_type             = ::boost::local_time::local_date_time;

    options_description opts_desc;
    ::std::string       package_name;
    ::std::string       package_version;
    ::std::string       copyright_holder;
    ::std::string       bugs_address;
    bool                foreign_user{false};

    time_type            gen_time;// = ::boost::posix_time::second_clock::local_time();

    po_entries entries;

    impl()
        : opts_desc(::boost::locale::translate("PO Generator Options").str()),
          gen_time{ ::boost::posix_time::second_clock::universal_time(), {} }
    {
        namespace po = ::boost::program_options;
        //gen_time.
        opts_desc.add_options()
            ("package-name",
                po::value<::std::string>(&package_name),
                ::boost::locale::translate("Package name").str().c_str())
            ("package-version",
                po::value<::std::string>(&package_version),
                ::boost::locale::translate("Package version").str().c_str())
            ("copyright-holder",
                po::value<::std::string>(&copyright_holder),
                ::boost::locale::translate("Copyright holder").str().c_str())
            ("foreign-user",
                po::bool_switch(&foreign_user),
                ::boost::locale::translate("Foreign user").str().c_str())
            ("msgid-bugs-address",
                po::value<::std::string>(&bugs_address),
                ::boost::locale::translate("Send string problems to").str().c_str())
        ;
    }
    void
    add_message(message const& msg, ::std::string const& comment)
    {
        if (msg.id().empty())
            return;
        po_entry entry{msg};
        auto res = entries.insert(entry);
        if (!res.second) {
            // Already there
            // Check plural form
            if (res.first->plural != entry.plural) {
                throw ::std::runtime_error{
                    "Message msgid '" + entry.id +
                    "' ambiguous plural form '" + entry.plural + "'" +
                    " (was '" + res.first->plural + "')"};
            }
        }
        if (!comment.empty()) {
            res.first->comments.push_back(comment);
        }
    }

    void
    write(::std::ostream& os)
    {
        using time_facet = boost::local_time::local_time_facet;
        time_facet* fct = new time_facet("%Y-%m-%d %H:%M%q");
        os.imbue(::std::locale{os.getloc(), fct});
        os <<
R"~(msgid ""
msgstr ""
)~";
        os << R"~("Project-Id-Version: )~" << package_name << " " << package_version << "\\n\"\n";
        if (!bugs_address.empty()) {
            os << R"~("Report-Msgid-Bugs-To: )~" << bugs_address << "\\n\"\n";
        }
        os  << R"~("POT-Creation-Date: )~" << gen_time << "\\n\"\n"
            << R"~("PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\n")~" << "\n"
            << R"~("Last-Translator: FULL NAME <EMAIL@ADDRESS>\n")~" << "\n"
            << R"~("Language-Team: LANGUAGE <LL@li.org>\n")~" << "\n"
            << R"~("Language: \n")~" << "\n"
            << R"~("MIME-Version: 1.0\n")~" << "\n"
            << R"~("Content-Type: text/plain; charset=UTF-8\n")~" << "\n"
            << R"~("Content-Transfer-Encoding: 8bit\n")~" << "\n"
        ;

        os << "\n";
        for (auto const& entry : entries) {
            os << entry;
        }
    }

};

po_generator::po_generator()
    : pimpl_{ new impl{} }
{
}

po_generator::~po_generator() = default;

void
po_generator::add_message(message const& msg, ::std::string const& comment)
{
    pimpl_->add_message(msg, comment);
}

void
po_generator::write(::std::ostream& os) const
{
    pimpl_->write(os);
}

::boost::program_options::options_description const&
po_generator::options_description() const
{
    return pimpl_->opts_desc;
}

}  /* namespace l10n */
}  /* namespace psst */
