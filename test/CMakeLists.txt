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

include_directories(${GTEST_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR})

set(
    test_data_url
    https://docs.google.com/spreadsheets/d/1RZmEBwOhRAlC12BUJmQECt-pf5gx8G656ThVuvHQEOM/pub?gid=0&single=true&output=csv
)
set(
    TEST_DATA_FILE
    l10n_test_data.csv
)

file(DOWNLOAD ${test_data_url}
    ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DATA_FILE}
    SHOW_PROGRESS)

l10n_project(
    PACKAGE l10ntest
    COPYRIGHT "Sergei A. Fedorov 2015-2016"
    BUGS "sergei.a.fedorov@gmail.com"
    PACKAGE_VERSION ${PROJECT_VERSION}
    POT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/translations
    MO_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/messages
    LANGUAGES en es ru
)

extract_l10n(
    PROGRAM test-gen-po
    DOMAIN l10ntest
    SOURCES ${TEST_DATA_FILE})

configure_file(config.in.hpp config.hpp)

add_library(test-data-load test_data_loader.cpp)

add_executable(test-gen-po generate_test_po_main.cpp)
target_link_libraries(test-gen-po
    ${PUSHKIN_L10N_LIB}
    ${PUSHKIN_L10N_GEN_LIB}
    test-data-load
)

set(
    test_l10n_SRCS
    arg_value_test.cpp
    message_test.cpp
    message_translate_test.cpp
)

add_executable(test-pushkin-l10n ${test_l10n_SRCS})
target_link_libraries(
    test-pushkin-l10n
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${PUSHKIN_L10N_LIB}
    test-data-load
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
