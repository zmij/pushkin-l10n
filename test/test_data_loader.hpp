/*
 * test_data_loader.hpp
 *
 *  Created on: 23 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef TEST_TEST_DATA_LOADER_HPP_
#define TEST_TEST_DATA_LOADER_HPP_

#include <pushkin/l10n/message.hpp>

namespace psst {
namespace l10n {
namespace test {

class test_data_loader {
public:
    using message_list = ::std::vector<message>;
public:
    test_data_loader();

    message_list const&
    messages() const
    { return messages_; }
private:
    message_list messages_;
};

} /* namespace test */
} /* namespace l10n */
} /* namespace psst */

#endif /* TEST_TEST_DATA_LOADER_HPP_ */
