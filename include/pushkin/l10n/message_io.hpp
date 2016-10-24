/*
 * message_io.hpp
 *
 *  Created on: Oct 24, 2016
 *      Author: zmij
 */

#ifndef PUSHKIN_L10N_MESSAGE_IO_HPP_
#define PUSHKIN_L10N_MESSAGE_IO_HPP_

#include <pushkin/l10n/message.hpp>

namespace psst {
namespace l10n {

/**
 * A facet for setting domain when inputting from assets
 */
class domain_name_facet : public ::std::locale::facet {
public:
    static ::std::locale::id id;
public:
    domain_name_facet(std::string const& domain) : domain_(domain) {}

    ::std::string const&
    domain() const
    { return domain_; }
private:
    ::std::string domain_;
};

/**
 * A facet for setting domain when inputting from assets
 */
class context_name_facet : public ::std::locale::facet {
public:
    static ::std::locale::id id;
public:
    context_name_facet(std::string const& domain) : context_(domain) {}

    ::std::string const&
    context() const
    { return context_; }
private:
    ::std::string context_;
};

/**
 * Facet to carry locale name in a stream.
 */
class locale_name_facet : public std::locale::facet {
public:
    static std::locale::id id;
public:
    locale_name_facet(std::string const& n) : facet(), name_(n) {}
    virtual ~locale_name_facet() {}

    std::string const&
    name() const
    { return name_; }
private:
    std::string name_;
};

}  /* namespace l10n */
}  /* namespace psst */



#endif /* PUSHKIN_L10N_MESSAGE_IO_HPP_ */
