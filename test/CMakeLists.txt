# CMakeLists.txt
# Created on: 23 окт. 2016 г.
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

if (NOT GTEST_INCLUDE_DIRS)
    find_package(GTest REQUIRED)
endif()
if (NOT CMAKE_THREAD_LIBS_INIT)
    find_package(Threads REQUIRED)
endif()

set(
    test_data_url
    https://docs.google.com/spreadsheets/d/1RZmEBwOhRAlC12BUJmQECt-pf5gx8G656ThVuvHQEOM/pub?gid=0&single=true&output=csv
)
file(DOWNLOAD ${test_data_url} ${CMAKE_CURRENT_BINARY_DIR}/l10n_test_data.csv
    SHOW_PROGRESS)

include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include)

set(
    test_l10n_SRCS
    arg_value_test.cpp
    message_test.cpp
)

add_executable(test-pushkin-l10n ${test_l10n_SRCS})
target_link_libraries(
    test-pushkin-l10n
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${PUSHKIN_L10N_LIB}
)

if (GTEST_XML_OUTPUT)
    set (
        TEST_ARGS
        --gtest_output=xml:test-wire-connector-detail.xml
    )
endif()

add_test(
    NAME    test-pushkin-l10n
    COMMAND test-pushkin-l10n ${TEST_ARGS}
)
