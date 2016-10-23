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

std::locale::id locale_name_facet::id;

namespace {
    const std::map< message::type, std::string > TYPE_TO_STRING {
        { message::EMPTY, "" },
        { message::SIMPLE, "L10N" },
        { message::PLURAL, "L10NN" },
        { message::CONTEXT, "L10NC" },
        { message::CONTEXT_PLURAL, "L10NNC" },
    }; // TYPE_TO_STRING
    const std::map< std::string, message::type > STRING_TO_TYPE {
        { "L10N", message::SIMPLE },
        { "L10NN", message::PLURAL },
        { "L10NC", message::CONTEXT },
        { "L10NNC", message::CONTEXT_PLURAL },
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
                val = message::EMPTY;
                in.setstate(std::ios_base::failbit);
            }
        }
    }
    return in;
}

message::message() : type_(EMPTY), n_(0)
{
}

message::message(optional_string const& domain)
    : type_(EMPTY), domain_(domain), n_(0)
{
}

message::message(std::string const& id, optional_string const& domain)
    : type_(SIMPLE), id_(id), domain_(domain), n_(0)
{
}

message::message(std::string const& context_str,
        std::string const& id, optional_string const& domain)
    : type_(CONTEXT), id_(id), context_(context_str), domain_(domain), n_(0)
{
}

message::message(std::string const& singular,
        std::string const& plural_str,
        int n, optional_string const& domain)
    : type_(PLURAL), id_(singular), plural_(plural_str), domain_(domain), n_(n)
{
}

message::message(std::string const& context,
        std::string const& singular,
        std::string const& plural,
        int n, optional_string const& domain)
    : type_(CONTEXT_PLURAL), id_(singular), context_(context),
      plural_(plural), domain_(domain), n_(n)
{
}

message::message(message const& rhs)
    : type_(rhs.type_), id_(rhs.id_), context_(rhs.context_),
      plural_(rhs.plural_), domain_(rhs.domain_), n_(rhs.n_)
{
    if (rhs.format_args_) {
        format_args_.reset( new format_args{ *rhs.format_args_ });
    }
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
    swap(format_args_, rhs.format_args_);
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
        case EMPTY:
            return localized_message();
        case SIMPLE:
            return localized_message(id_);
        case CONTEXT:
            return localized_message(context_.value(), id_);
        case PLURAL:
            return localized_message(id_, plural_.value(), n_);
        case CONTEXT_PLURAL:
            return localized_message(context_.value(), id_, plural_.value(), n_);
        default:
            throw std::runtime_error("Unknown message type");
    }
}

void
message::write(std::ostream& os) const
{
    namespace locn = boost::locale;
    if (domain_.is_initialized()) {
        os << locn::as::domain(domain_.value());
    }
    if (has_plural() || has_format_args()) {
        std::vector< std::string > format_buffers;
        locn::format fmt(translate());
        if (has_plural()) {
            fmt % n_;
        }
        if (has_format_args()) {
            std::locale loc = os.getloc();
            if (std::has_facet<locale_name_facet>(loc)) {
                locale_name_facet const& lname = std::use_facet<locale_name_facet>(loc);
                std::cerr << "Locale name " << lname.name() << " (@write)\n";
            }
            format_buffers.reserve(format_args_->size());
            format_args_->feed_arguments(fmt, format_buffers, loc, domain_);
        }
        os << fmt;
    } else {
        os << translate();
    }
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


//----------------------------------------------------------------------------
//void
//format_args::load(cereal::JSONInputArchive& ar, size_type sz)
//{
//    while (sz > 0) {
//        if (ar.nodeValue().IsString()) {
//            std::string v;
//            ar(v);
//            arguments_.emplace_back(v);
//        } else if (ar.nodeValue().IsArray()) {
//            message v;
//            v.load(ar);
//            arguments_.emplace_back(v);
//        } else if (ar.nodeValue().IsBool_()) {
//            bool v;
//            ar(v);
//            arguments_.emplace_back(v);
//        } else if (ar.nodeValue().IsDouble()) {
//            double v;
//            ar(v);
//            arguments_.emplace_back(v);
//        } else if (ar.nodeValue().IsInt() || ar.nodeValue().IsInt64()) {
//            int64_t v;
//            ar(v);
//            arguments_.emplace_back(v);
//        }
//        --sz;
//    }
//}

struct arg_feeder : boost::static_visitor<> {
    boost::locale::format* msg;
    std::vector< std::string >* fmt_buffers_;
    std::locale const& loc;
    boost::optional< std::string > const& domain;

    arg_feeder(boost::locale::format& m,
            std::vector< std::string >& buffers,
            std::locale const& l,
            boost::optional< std::string > const& d)
        : msg(&m), fmt_buffers_(&buffers), loc(l), domain(d) {}
    template < typename T >
    void
    operator()(T const& v) const
    {
        *msg % v;
    }

    void
    operator()(message const& v) const
    {
        std::ostringstream os;
        os.imbue(loc);
        if (domain.is_initialized()) {
            os << boost::locale::as::domain(domain.value());
        }
        v.write(os);
        fmt_buffers_->push_back(os.str());
        *msg % fmt_buffers_->back();
    }
};

void
format_args::feed_arguments(formatted_message& msg,
        format_buffers_type& buffers, std::locale const& loc,
        optional_string const& domain) const
{
    std::locale l1(loc);
    if (std::has_facet<locale_name_facet>(l1)) {
        locale_name_facet const& lname = std::use_facet<locale_name_facet>(l1);
        std::cerr << "Locale name " << lname.name() << " (@feed)\n";
    }
    arg_feeder visitor{ msg, buffers, loc, domain };
    std::for_each(
        arguments_.begin(), arguments_.end(),
        boost::apply_visitor(visitor)
    );
}

} /* namespace l10n */
} /* namespace psst */
