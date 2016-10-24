/*
 * config.in.hpp
 *
 *  Created on: 23 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#ifndef TEST_CONFIG_IN_HPP_
#define TEST_CONFIG_IN_HPP_

#include <string>

namespace psst {
namespace l10n {
namespace test {

::std::string const TEST_ROOT = "@CMAKE_CURRENT_SOURCE_DIR@";
::std::string const TEST_DATA_FILE = "@TEST_DATA_FILE@";
::std::string const L10N_LANGUAGES = "@L10N_LANGUAGES@";
::std::string const L10N_MO_DIRECTORY = "@L10N_MO_DIRECTORY@";

}  /* namespace test */
}  /* namespace l10n */
}  /* namespace psst */



#endif /* TEST_CONFIG_IN_HPP_ */
