/**
 * message.cpp
 *
 *  Created on: 11 окт. 2015 г.
 *      Author: zmij
 */

#include <pushkin/l10n/message.hpp>
#include <pushkin/l10n/message_io.hpp>
#include <pushkin/l10n/message_util.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <sstream>
#include <map>
#include <algorithm>

namespace psst {
namespace l10n {

::std::locale::id domain_name_facet::id;
::std::locale::id context_name_facet::id;
::std::locale::id locale_name_facet::id;

namespace detail {

format::format(localized_message const& msg)
    : fmt_{ new formatted_message{ msg } }
{
}

format::format(localized_message&& msg)
    : fmt_{ new formatted_message{ ::std::move(msg) } }
{
}

format::format(formatted_message_ptr&& fmt)
    : fmt_{ ::std::move(fmt) }
{
}

message_args::message_args(message_args const& rhs)
    : args_{}
{
    args_.reserve(rhs.args_.size());
    ::std::transform(rhs.args_.begin(), rhs.args_.end(),
        ::std::back_inserter(args_),
         [](arg_holder const& arg)
         {
            return arg->clone();
         });
}

message_args::message_args(message_args&& rhs)
    : args_{::std::move(rhs.args_)}
{
}

}  /* namespace detail */

namespace {
    const std::map< message::message_type, std::string > TYPE_TO_STRING {
        { message::message_type::empty, "" },
        { message::message_type::simple, "L10N" },
        { message::message_type::plural, "L10NN" },
        { message::message_type::context, "L10NC" },
        { message::message_type::context_plural, "L10NNC" },
    }; // TYPE_TO_STRING
    const std::map< std::string, message::message_type > STRING_TO_TYPE {
        { "L10N", message::message_type::simple },
        { "L10NN", message::message_type::plural },
        { "L10NC", message::message_type::context },
        { "L10NNC", message::message_type::context_plural },
    }; // STRING_TO_TYPE

    const ::std::string DEFAULT_DOMAIN = "";
} // namespace

// Generated output operator
std::ostream&
operator << (std::ostream& out, message::message_type val)
{
    std::ostream::sentry s (out);
    if (s) {
        auto f = TYPE_TO_STRING.find(val);
        if (f != TYPE_TO_STRING.end()) {
            out << f->second;
        } else {
            out << "Unknown type " << (int)val;
        }
    }
    return out;
}
// Generated input operator
std::istream&
operator >> (std::istream& in, message::message_type& val)
{
    std::istream::sentry s (in);
    if (s) {
        std::string name;
        if (in >> name) {
            auto f = STRING_TO_TYPE.find(name);
            if (f != STRING_TO_TYPE.end()) {
                val = f->second;
            } else {
                val = message::message_type::empty;
                in.setstate(std::ios_base::failbit);
            }
        }
    }
    return in;
}

message::message() : type_(message_type::empty), n_(0)
{
}

message::message(optional_string const& domain)
    : type_(message_type::empty), domain_(domain), n_(0)
{
}

message::message(std::string const& id, optional_string const& domain)
    : type_(message_type::simple), msgid_(id), msgstr_(id), domain_(domain), n_(0)
{
}

message::message(id_str_pair const& id_n_str,
        domain_type const& domain)
    : type_(message_type::simple), msgid_(id_n_str.first), msgstr_(id_n_str.second), domain_(domain), n_(0)
{
}

message::message(std::string const& context_str,
        std::string const& id, optional_string const& domain)
    : type_(message_type::context), msgid_(id), msgstr_(id), context_(context_str), domain_(domain), n_(0)
{
}

message::message(std::string const& context_str,
        id_str_pair const& id_n_str,
        domain_type const& domain)
    : type_(message_type::context), msgid_(id_n_str.first), msgstr_(id_n_str.second), context_(context_str), domain_(domain), n_(0)
{
}

message::message(std::string const& singular,
        std::string const& plural_str,
        int n, optional_string const& domain)
    : type_(message_type::plural), msgid_(singular), msgstr_(singular), plural_(plural_str), domain_(domain), n_(n)
{
}

message::message(std::string const& context,
        std::string const& singular,
        std::string const& plural,
        int n, optional_string const& domain)
    : type_(message_type::context_plural), msgid_(singular), msgstr_(singular), context_(context),
      plural_(plural), domain_(domain), n_(n)
{
}

void
message::swap(message& rhs) noexcept
{
    using std::swap;
    swap(type_, rhs.type_);
    swap(msgid_, rhs.msgid_);
    swap(msgstr_, rhs.msgstr_);
    swap(plural_, rhs.plural_);
    swap(context_, rhs.context_);
    swap(n_, rhs.n_);
    swap(domain_, rhs.domain_);
    swap(args_, rhs.args_);
}

::std::string const&
message::plural() const
{
    if (!plural_.is_initialized())
        throw ::std::runtime_error{
            "Message msgid '" + msgid_ + "' doesn't contain a plural form" };
    return *plural_;
    }

::std::string const&
message::context() const
{
    if (!context_.is_initialized())
        throw ::std::runtime_error{
            "Message msgid '" + msgid_ + "' doesn't have context" };
    return *context_;
    }

::std::string const&
message::domain() const
{
    if (!domain_.is_initialized())
        return DEFAULT_DOMAIN;
    return *domain_;
}

void
message::domain( std::string const& domain)
{
    domain_ = domain;
}

void
message::set_plural(int n)
{
    switch (type_) {
        case message_type::simple:
        case message_type::plural:
            type_ = message_type::plural; break;
        case message_type::context:
        case message_type::context_plural:
            type_ = message_type::context_plural; break;
        default:
            throw ::std::runtime_error{"Cannot convert message to a plural form"};
    }

    plural_ = msgid_;
    n_ = n;
}

message::localized_message
message::translate(int n) const
{
    switch (type_) {
        case message_type::empty:
            return localized_message();
        case message_type::simple:
            return localized_message(msgid_);
        case message_type::context:
            return localized_message(context_.value(), msgid_);
        case message_type::plural:
            return localized_message(msgid_, plural_.value(), n);
        case message_type::context_plural:
            return localized_message(context_.value(), msgid_, plural_.value(), n);
        default:
            throw std::runtime_error("Unknown message type");
    }
}

detail::format
message::format(int n, bool feed_plural) const
{
    detail::format fmt{translate(n)};
    if (has_plural() && feed_plural)
        fmt % n;
    fmt % args_;
    return fmt;
}

void
message::write(std::ostream& os) const
{
    namespace locn = boost::locale;
    if (domain_.is_initialized()) {
        os << locn::as::domain(domain_.value());
    }
    os << format();
}

void
message::collect(message_list& messages) const
{
    for (auto const& arg : args_) {
        arg->collect(messages);
    }
}

message
message::create_message(::std::string const& id, get_named_param_func f,
            domain_type const& domain)
{
    auto id_phs = extract_named_placeholders(id);
    message msg{id_phs.first, domain};
    for (auto const& ph : id_phs.second) {
        auto const& nm = ::boost::get<::std::string>(ph.id);
        f(msg, nm);
    }
    return msg;
}

message
message::create_message(std::string const& context,
            std::string const& id,
            get_named_param_func f,
            domain_type const& domain)
{
    auto id_phs = extract_named_placeholders(id);
    message msg{context, id_phs.first, domain};
    for (auto const& ph : id_phs.second) {
        auto const& nm = ::boost::get<::std::string>(ph.id);
        f(msg, nm);
    }
    return msg;
}

message
message::create_message(std::string const& singular,
        std::string const& plural,
        get_named_param_func f,
        get_n_func get_n,
        int n,
        domain_type const& domain)
{
    auto singular_phs = extract_named_placeholders(singular);
    auto plural_phs   = extract_named_placeholders(plural);
    message msg{singular_phs.first, plural_phs.first, n, domain};
    for (auto const& ph : singular_phs.second) {
        auto const& nm = ::boost::get<::std::string>(ph.id);
        if (ph.is_pluralizer) {
            if (get_n) {
                msg.set_n(get_n(nm));
            } else {
                f(msg, nm);
            }
        } else {
            f(msg, nm);
        }
    }
    return msg;
}

message
message::create_message(std::string const& context,
        std::string const& singular,
        std::string const& plural,
        get_named_param_func f,
        get_n_func get_n,
        int n,
        domain_type const& domain)
{
    auto singular_phs = extract_named_placeholders(singular);
    auto plural_phs   = extract_named_placeholders(plural);
    message msg{context, singular_phs.first, plural_phs.first, n, domain};
    for (auto const& ph : singular_phs.second) {
        auto const& nm = ::boost::get<::std::string>(ph.id);
        if (ph.is_pluralizer) {
            if (get_n) {
                msg.set_n(get_n(nm));
            }
            if (ph.uses > 1)
                f(msg, nm);
        } else {
            f(msg, nm);
        }
    }
    return msg;
}

std::ostream&
operator << (std::ostream& out, message const& val)
{
    std::ostream::sentry s(out);
    if (s) {
        val.write(out);
    }
    return out;
}

::std::istream&
operator >> (::std::istream& is, message& val)
{
    ::std::istream::sentry s(is);
    if (s) {
        ::std::string msgstr;

        char c;
        while(is.get(c))
            msgstr.push_back(c);

        message::id_str_pair id_n_str{
            (val.id().empty() ? msgstr : val.id()), msgstr
        };

        message::optional_string domain;
        message::optional_string context;
        auto loc = is.getloc();
        if (::std::has_facet<domain_name_facet>(loc)) {
            domain = ::std::use_facet<domain_name_facet>(loc).domain();
        }
        if (::std::has_facet<context_name_facet>(loc)) {
            context = ::std::use_facet<context_name_facet>(loc).context();
        }

        if (context.is_initialized()) {
            val = message{*context, id_n_str, domain};
        } else {
            val = message{id_n_str, domain};
        }
    }
    return is;
}


} /* namespace l10n */
} /* namespace psst */
