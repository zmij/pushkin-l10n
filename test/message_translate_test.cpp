/*
 * message_translate_test.cpp
 *
 *  Created on: 24 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include <gtest/gtest.h>
#include <pushkin/l10n/message.hpp>
#include "test_data_loader.hpp"
#include "config.hpp"

namespace psst {
namespace l10n {
namespace test {

TEST(Message, DISABLED_Translate)
{
    namespace locn = ::boost::locale;
    locn::generator gen;
    gen.add_messages_path(L10N_MO_DIRECTORY);
    gen.add_messages_domain("l10ntest");

    ::std::locale::global(gen(""));
    ::std::cout.imbue(::std::locale{});

    test_data_loader ldr;
    for (auto const& msg : ldr.messages()) {
        ::std::cout << msg << "\n";
        // TODO Add checks
    }
}

}  /* namespace test */
}  /* namespace l10n */
}  /* namespace psst */
