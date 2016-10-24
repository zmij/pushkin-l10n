/*
 * po_generator.hpp
 *
 *  Created on: 23 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef PUSHKIN_L10N_PO_GENERATOR_HPP_
#define PUSHKIN_L10N_PO_GENERATOR_HPP_

#include <pushkin/l10n/message.hpp>
#include <memory>

namespace boost {
namespace program_options {

class options_description;

}  /* namespace program_options */
}  /* namespace boost */

namespace psst {
namespace l10n {

class po_generator {
public:
    po_generator();
    ~po_generator();

    void
    add_message(message const&);
    void
    write(::std::ostream& os) const;

    ::boost::program_options::options_description const&
    options_description() const;
private:
    struct impl;
    typedef ::std::unique_ptr<impl> pimpl;
    pimpl pimpl_;
};

inline ::std::ostream&
operator << (::std::ostream& os, po_generator const& val)
{
    ::std::ostream::sentry s (os);
    if (s) {
        val.write(os);
    }
    return os;
}


}  /* namespace l10n */
}  /* namespace psst */


#endif /* PUSHKIN_L10N_PO_GENERATOR_HPP_ */
