/*
 * message_util.hpp
 *
 *  Created on: Nov 9, 2016
 *      Author: zmij
 */

#ifndef PUSHKIN_L10N_MESSAGE_UTIL_HPP_
#define PUSHKIN_L10N_MESSAGE_UTIL_HPP_

#include <string>
#include <vector>
#include <iosfwd>
#include <boost/variant.hpp>

namespace psst {
namespace l10n {

/**
 * Placeholder description
 * Placeholder syntax:
 * {number|[n:]name(,options)}
 * Prefix n: denotes that the named placeholder is used for pluralization of
 * the message
 */
struct placeholder {
    enum placeholder_type {
        number,
        name
    };
    using id_type = ::boost::variant<::std::size_t, ::std::string>;

    id_type         id;
    bool            is_pluralizer;
    ::std::string   options;
    ::std::size_t   uses{0};

    placeholder_type
    type() const
    {
        return static_cast<placeholder_type>(id.which());
    }
};

using placeholders = ::std::vector<placeholder>;

::std::ostream&
operator << (::std::ostream& os, placeholder const& val);
::std::ostream&
operator << (::std::ostream& os, placeholders const& val);


placeholders
extract_placeholders(::std::string const&);

/**
 * Extract named placeholders, renumber all placeholders
 * and return a pair of modified string and named placeholders.
 * @param
 * @return
 */
::std::pair<::std::string, placeholders>
extract_named_placeholders(::std::string const&);

}  /* namespace l10n */
}  /* namespace psst */


#endif /* PUSHKIN_L10N_MESSAGE_UTIL_HPP_ */
