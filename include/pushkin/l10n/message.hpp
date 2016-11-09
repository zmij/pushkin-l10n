/**
 * message.hpp
 *
 *  Created on: 11 окт. 2015 г.
 *      Author: zmij
 */

#ifndef PUSHKIN_L10N_MESSAGE_HPP_
#define PUSHKIN_L10N_MESSAGE_HPP_

#include <string>
#include <memory>
#include <iosfwd>
#include <functional>

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/locale.hpp>

namespace psst {
namespace l10n {

class message;
using message_list = ::std::vector<message>;

namespace detail {

struct abstract_arg_value {
    using arg_ptr                   = ::std::unique_ptr<abstract_arg_value>;
    using formatted_message         = ::boost::locale::format;

    virtual ~abstract_arg_value() = default;

    virtual arg_ptr
    clone() = 0;
    virtual void
    format(formatted_message&) const = 0;
    virtual void
    collect(message_list&) const {}
};

template < typename T >
struct arg_value : abstract_arg_value {
    using value_type        = T;

    explicit
    arg_value(value_type const& v) : value{v} {}
    explicit
    arg_value(value_type&& v) : value{::std::move(v)} {}
    arg_value(arg_value const& rhs) : value{rhs.value} {}
    arg_value(arg_value&& rhs) : value{::std::move(rhs.value)} {}
    virtual ~arg_value() = default;

    void
    swap(arg_value& rhs) noexcept
    {
        using ::std::swap;
        swap(value, rhs.value);
    }

    arg_ptr
    clone() override
    {
        return arg_ptr{ new arg_value{ *this } };
    }

    void
    format(formatted_message& fmt) const override
    {
        fmt % value;
    }

    value_type value;
};

class message_args {
public:
    using arg_holder        = ::std::unique_ptr<abstract_arg_value>;
    using arg_values        = ::std::vector<arg_holder>;
    using size_type         = arg_values::size_type;
    using const_iterator    = arg_values::const_iterator;
public:
    message_args() {}
    message_args(message_args const& rhs);
    message_args(message_args&& rhs);

    void
    swap(message_args& rhs) noexcept
    {
        using ::std::swap;
        swap(args_, rhs.args_);
    }

    message_args&
    operator = (message_args const& rhs)
    {
        message_args tmp(rhs);
        swap(tmp);
        return *this;
    }
    message_args&
    operator = (message_args&& rhs)
    {
        message_args tmp{::std::move(rhs)};
        swap(tmp);
        return *this;
    }

    bool
    empty() const
    { return args_.empty(); }

    size_type
    size() const
    { return args_.size(); }

    const_iterator
    begin() const
    { return args_.cbegin(); }
    const_iterator
    cbegin() const
    { return args_.cbegin(); }

    const_iterator
    end() const
    { return args_.cend(); }
    const_iterator
    cend() const
    { return args_.cend(); }

    abstract_arg_value const&
    back() const
    {
        return *args_.back();
    }

    template < typename T >
    message_args&
    operator << (T&& v)
    {
        using arg_type = arg_value<typename ::std::decay<T>::type>;
        args_.emplace_back( new arg_type{ ::std::forward<T>(v) });
        return *this;
    }

    message_args&
    operator << (char const* v)
    {
        return *this << ::std::string{v};
    }
private:
    arg_values args_;
};

/**
 * Class to move ::boost::locale::format around
 * Caches nested arguments and formats
 */
class format {
public:
    using localized_message         = ::boost::locale::message;
    using formatted_message         = ::boost::locale::format;
    using formatted_message_ptr     = ::std::unique_ptr<formatted_message>;
public:
    explicit
    format(localized_message const&);
    explicit
    format(localized_message&&);
    explicit
    format(formatted_message_ptr&& fmt);

    format(format const&) = delete; // noncopyable
    format(format&&) = default;     // move only

    ::std::string
    str(::std::locale const& loc = ::std::locale{}) const
    {
        return fmt_->str(loc);
    }

    format&
    operator % (format&& v)
    {
        nested_.push_back(::std::move(v));
        *fmt_ % *nested_.back().fmt_;
        return *this;
    }
    template < typename T >
    format&
    operator % (T&& v)
    {
        tmps_ << v;
        tmps_.back().format(*fmt_);
        return *this;
    }
    format&
    operator % (abstract_arg_value const& v)
    {
        v.format(*fmt_);
        return *this;
    }
    format&
    operator % (abstract_arg_value::arg_ptr const& v)
    {
        v->format(*fmt_);
        return *this;
    }
private:
    using nested_formats = ::std::vector<format>;
    formatted_message_ptr   fmt_;
    nested_formats          nested_;
    message_args            tmps_;

    friend ::std::ostream&
    operator << (::std::ostream& os, format const& val)
    {
        ::std::ostream::sentry s (os);
        if (s) {
            os << *val.fmt_;
        }
        return os;
    }
};

inline ::boost::locale::format&
operator % (::boost::locale::format& fmt, message_args const& args)
{
    for (auto const& arg : args) {
        arg->format(fmt);
    }
    return fmt;
}

}  /* namespace detail */

/**
 * Class for representing a message that must be translated when sending
 * to output. The message can contain predefined format arguments and
 * they can contain other l10n::message's recursively, so that the message
 * is fully translated when sent to output.
 * The message can be read in runtime from text files and used to generate
 * gettext po files.
 */
class message {
public:
    using optional_string                   = ::boost::optional< std::string >;
    using domain_type                       = optional_string;
    using size_type                         = ::std::string::size_type;
    using localized_message                 = ::boost::locale::message;
    using get_named_param_func              = ::std::function<void(message&,
                                                    ::std::string const& param_name)>;
    using get_n_func                        = ::std::function<int(::std::string const& param_name)>;

    enum class message_type {
        empty           = 0x00,
        simple          = 0x01,
        plural          = 0x02,
        context         = 0x04,
        context_plural  = plural | context
    };
public:
    /**
     * Construct a default empty message
     */
    message();
    explicit
    message(domain_type const& domain);
    /**
     * Construct a message with message id
     */
    explicit
    message(std::string const& id,
            domain_type const& domain = domain_type{});
    /**
     * Construct a message with message id and a context
     */
    message(std::string const& context,
            std::string const& id,
            domain_type const& domain = domain_type{});
    /**
     * Construct a message with singular/plural
     */
    message(std::string const& singular,
            std::string const& plural,
            int n,
            domain_type const& domain = domain_type{});
    /**
     * Construct a message with singular/plural and a context
     */
    message(std::string const& context,
            std::string const& singular,
            std::string const& plural,
            int n,
            optional_string const& domain = optional_string());

    message(message const&) = default;
    message(message&&) = default;

    void
    swap(message& rhs) noexcept;
    void
    swap(message&& rhs) noexcept
    {
        swap(rhs);
    }

    message&
    operator = (message const& rhs)
    {
        message tmp(rhs);
        swap(tmp);
        return *this;
    }
    message&
    operator = (message&& rhs)
    {
        message tmp(::std::move(rhs));
        swap(tmp);
        return *this;
    }

    //@{
    /** @name Comparison operators */
    /**
     * Test for equality. Don't take format arguments into account.
     * @param rhs
     * @return
     */
    bool
    operator == (message const& rhs) const;
    /**
     * Test for not equality. Don't take format arguments into account.
     * @param rhs
     * @return
     */
    bool
    operator != (message const& rhs) const;
    /**
     * Test for sorting order. Don't take format arguments into account.
     * @param rhs
     * @return
     */
    bool
    operator < (message const& rhs) const;
    //@}

    bool
    empty() const
    { return type_ == message_type::empty || id_.empty(); }

    //@{
    /** @name Accessors */
    message_type
    type() const
    { return type_; }

    ::std::string const&
    id() const
    { return id_; }

    ::std::string const&
    plural() const;
    ::std::string const&
    context() const;
    ::std::string const&
    domain() const;
    /**
     * Set message domain
     * @param New domain value
     */
    void
    domain( std::string const& );
    int
    get_n() const
    { return n_; }
    void
    set_n(int n)
    { n_ = n; }
    //@}
    //@{
    /**
     * Make a plural message from a simple or context message.
     * If the message is already pluralized, the plural form will be ignored.
     * @param plural
     * @param n
     * @return
     */
    message
    make_plural(::std::string const& plural, int n) const;
    //@}
    //@{
    /** @name Check functions */
    bool
    has_context() const
    { return context_.is_initialized(); }
    bool
    has_plural() const
    { return plural_.is_initialized(); }
    bool
    has_format_args() const
    { return !args_.empty(); }
    //@}

    bool
    args_empty() const
    { return args_.empty(); }
    size_type
    args_size() const
    { return args_.size(); }

    /**
     * Add argument to predefined formatting arguments
     * @param v argument value
     * @return reference to self, for operator chaining
     */
    template < typename T >
    message&
    operator << (T&& v)
    {
        args_ << ::std::forward<T>(v);
        return *this;
    }

    /**
     * Write translated and formatted message to the output stream
     * @param os
     */
    void
    write(std::ostream& os) const;

    /**
     * Create a boost::locale::message object
     * @return
     */
    localized_message
    translate(int n) const;
    /**
     * Create a format object
     * @param feed_plural Feed plural number specified in the message to format.
     * @return
     */
    detail::format
    format(bool feed_plural = true) const
    {
        return format(n_, feed_plural);
    }
    /**
     * Create a format object, pluralized with n
     * @param n
     * @param feed_plural
     * @return
     */
    detail::format
    format(int n, bool feed_plural = true) const;

    /**
     * Get a string translated to specified locale
     * @param loc
     * @return
     */
    ::std::string
    str(::std::locale const& loc = ::std::locale{}) const
    {
        return format().str(loc);
    }
    /**
     * Create a format object and feed a value into it.
     * The value is not stored in the message object.
     * Returns the format object to enable feed operator chaining.
     */
    template < typename T >
    detail::format
    operator % (T&& val) const
    {
        auto fmt = format();
        fmt % ::std::forward<T>(val);
        return fmt;
    }

    /**
     * Collect nested messages for translation.
     * @param
     */
    void
    collect(message_list& ) const;

    /**
     * Create a message with the specified id
     * Will replace named placeholders to enumerated, renumber the placeholders
     * according to the order of named placeholders and call the user-provided
     * function to feed the named parameters.
     * @param id
     * @param f     Function to call for feeding a named parameter to the message
     * @return
     */
    static message
    create_message(::std::string const& id, get_named_param_func f,
            domain_type const& domain = domain_type{});
    static message
    create_message(std::string const& context,
            std::string const& id,
            get_named_param_func f,
            domain_type const& domain = domain_type{});
    static message
    create_message(std::string const& singular,
            std::string const& plural,
            get_named_param_func f,
            get_n_func get_n,
            int n = 0,
            domain_type const& domain = domain_type{});
    static message
    create_message(std::string const& context,
            std::string const& singular,
            std::string const& plural,
            get_named_param_func f,
            get_n_func get_n,
            int n = 0,
            optional_string const& domain = optional_string());
private:
    message_type                        type_;
    std::string                 id_;
    optional_string             context_;
    optional_string             plural_;
    optional_string             domain_;

    int                         n_;
    detail::message_args        args_;
};
::std::ostream&
operator << (::std::ostream& out, message::message_type val);
::std::istream&
operator >> (std::istream& in, message::message_type& val);

::std::ostream&
operator << (std::ostream& os, message const& v);

/**
 * Will use a domain and context facets to construct a value.
 * The message constructed will be a simple or context message.
 * @param is
 * @param val
 * @return
 */
::std::istream&
operator >> (::std::istream& is, message& val);

inline detail::format&
operator % (detail::format& fmt, message const& msg)
{
    return fmt % msg.format();
}

namespace detail {

template <>
struct arg_value<message> : abstract_arg_value {
    using value_type        = message;

    explicit
    arg_value(value_type const& v) : value{v} {}
    explicit
    arg_value(value_type&& v) : value{::std::move(v)} {}
    arg_value(arg_value const& rhs) : value{rhs.value} {}
    arg_value(arg_value&& rhs) : value{::std::move(rhs.value)} {}
    virtual ~arg_value() = default;

    void
    swap(arg_value& rhs) noexcept
    {
        using ::std::swap;
        swap(value, rhs.value);
    }

    arg_ptr
    clone() override
    {
        return arg_ptr{ new arg_value{ *this } };
    }

    void
    format(formatted_message& fmt) const override
    {
        fmt % value;
    }

    void
    collect(message_list& messages) const override
    {
        messages.push_back(value);
        value.collect(messages);
    }

    value_type value;
};

}  /* namespace detail */

} /* namespace l10n */
} /* namespace psst */

#endif /* PUSHKIN_L10N_MESSAGE_HPP_ */
