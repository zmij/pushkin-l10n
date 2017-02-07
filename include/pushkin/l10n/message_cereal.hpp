/*
 * message_cereal.hpp
 *
 *  Created on: 23 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef PUSHKIN_L10N_MESSAGE_CEREAL_HPP_
#define PUSHKIN_L10N_MESSAGE_CEREAL_HPP_

#include <pushkin/l10n/message.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>

namespace psst {
namespace l10n {

void
load(cereal::JSONInputArchive&, message&);
void
save(cereal::JSONOutputArchive&, message const&);

}  /* namespace l10n */
}  /* namespace psst */

namespace cereal {

inline void
prologue(JSONInputArchive&, ::psst::l10n::message const&) {}
inline void
epilogue(JSONInputArchive&, ::psst::l10n::message const& ) {}

inline void
prologue(JSONOutputArchive&, ::psst::l10n::message const&) {}
inline void
epilogue(JSONOutputArchive&, ::psst::l10n::message const& ) {}

}  // namespace cereal

#endif /* PUSHKIN_L10N_MESSAGE_CEREAL_HPP_ */
