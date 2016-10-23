/**
 * message.cpp
 *
 *  Created on: 11 окт. 2015 г.
 *      Author: zmij
 */

#include <pushkin/l10n/message.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <sstream>
#include <map>
#include <algorithm>

namespace psst {
namespace l10n {

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

std::locale::id locale_name_facet::id;

namespace {
    const std::map< message::type, std::string > TYPE_TO_STRING {
        { message::type::empty, "" },
        { message::type::simple, "L10N" },
        { message::type::plural, "L10NN" },
        { message::type::context, "L10NC" },
        { message::type::context_plural, "L10NNC" },
    }; // TYPE_TO_STRING
    const std::map< std::string, message::type > STRING_TO_TYPE {
        { "L10N", message::type::simple },
        { "L10NN", message::type::plural },
        { "L10NC", message::type::context },
        { "L10NNC", message::type::context_plural },
    }; // STRING_TO_TYPE
} // namespace

// Generated output operator
std::ostream&
operator << (std::ostream& out, message::type val)
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
operator >> (std::istream& in, message::type& val)
{
    std::istream::sentry s (in);
    if (s) {
        std::string name;
        if (in >> name) {
            auto f = STRING_TO_TYPE.find(name);
            if (f != STRING_TO_TYPE.end()) {
                val = f->second;
            } else {
                val = message::type::empty;
                in.setstate(std::ios_base::failbit);
            }
        }
    }
    return in;
}

message::message() : type_(type::empty), n_(0)
{
}

message::message(optional_string const& domain)
    : type_(type::empty), domain_(domain), n_(0)
{
}

message::message(std::string const& id, optional_string const& domain)
    : type_(type::simple), id_(id), domain_(domain), n_(0)
{
}

message::message(std::string const& context_str,
        std::string const& id, optional_string const& domain)
    : type_(type::context), id_(id), context_(context_str), domain_(domain), n_(0)
{
}

message::message(std::string const& singular,
        std::string const& plural_str,
        int n, optional_string const& domain)
    : type_(type::plural), id_(singular), plural_(plural_str), domain_(domain), n_(n)
{
}

message::message(std::string const& context,
        std::string const& singular,
        std::string const& plural,
        int n, optional_string const& domain)
    : type_(type::context_plural), id_(singular), context_(context),
      plural_(plural), domain_(domain), n_(n)
{
}

void
message::swap(message& rhs) noexcept
{
    using std::swap;
    swap(type_, rhs.type_);
    swap(id_, rhs.id_);
    swap(plural_, rhs.plural_);
    swap(context_, rhs.context_);
    swap(n_, rhs.n_);
    swap(domain_, rhs.domain_);
    swap(args_, rhs.args_);
}

void
message::set_domain( std::string const& domain)
{
    domain_ = domain;
}

message::localized_message
message::translate() const
{
    switch (type_) {
        case type::empty:
            return localized_message();
        case type::simple:
            return localized_message(id_);
        case type::context:
            return localized_message(context_.value(), id_);
        case type::plural:
            return localized_message(id_, plural_.value(), n_);
        case type::context_plural:
            return localized_message(context_.value(), id_, plural_.value(), n_);
        default:
            throw std::runtime_error("Unknown message type");
    }
}

detail::format
message::format() const
{
    detail::format fmt{translate()};
    if (has_plural())
        fmt % n_;
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

std::ostream&
operator << (std::ostream& out, message const& val)
{
    std::ostream::sentry s(out);
    if (s) {
        val.write(out);
    }
    return out;
}

} /* namespace l10n */
} /* namespace psst */
