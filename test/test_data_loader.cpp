/*
 * test_data_loader.cpp
 *
 *  Created on: 23 окт. 2016 г.
 *      Author: sergey.fedorov
 */

#include "test_data_loader.hpp"
#include "config.hpp"

#include <iostream>
#include <fstream>

namespace psst {
namespace l10n {
namespace test {

::std::vector<::std::string>
split(::std::string const& str, char delim = ',')
{
    ::std::vector<::std::string> res{1};
    for (auto c : str) {
        if (c == delim) {
            res.push_back(::std::string{});
        } else {
            res.back().push_back(c);
        }
    }
    return res;
}

enum fields {
    MSG_ID          = 0,
    MSG_PLURAL      = 1,
    MSG_CONTEXT     = 2,
    MSG_N           = 3,
    MSG_COMMENT     = 4
};

test_data_loader::test_data_loader()
{
    ::std::ifstream is{ TEST_ROOT + "/" + TEST_DATA_FILE };
    if (!is) {
        throw ::std::runtime_error{ "Failed to open test data file" };
    }

    ::std::string line;
    ::std::getline(is, line);
    // Skip first line
    auto line_no = 2;
    while (is) {
        if (!::std::getline(is, line))
            break;
        auto row = split(line);
        if (row[MSG_ID].empty()) {
            throw ::std::runtime_error{ "Message id in line " +
                ::std::to_string(line_no) + " is empty" };
        }
        if (!row[MSG_PLURAL].empty()) {
            ::std::istringstream is{row[MSG_N]};
            int n;
            is >> n;
            if (row[MSG_CONTEXT].empty()) {
                // L10NN
                messages_.emplace_back( row[MSG_ID], row[MSG_PLURAL], n );
            } else {
                // L10NNC
                messages_.emplace_back( row[MSG_CONTEXT], row[MSG_ID], row[MSG_PLURAL], n );
            }
        } else {
            if (row[MSG_CONTEXT].empty()) {
                // L10N
                messages_.emplace_back( row[MSG_ID] );
            } else {
                // L10NC
                messages_.emplace_back( row[MSG_CONTEXT], row[MSG_ID] );
            }
        }
        // TODO Set comment
        ++line_no;
    }
}

} /* namespace test */
} /* namespace l10n */
} /* namespace psst */
