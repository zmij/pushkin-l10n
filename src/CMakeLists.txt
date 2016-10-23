# CMakeLists.txt
# Created on: 11 окт. 2015 г.
#     Author: zmij

cmake_minimum_required(VERSION 2.6)

set(l10n_SRCS
    message.cpp
)

add_library(${PUSHKIN_L10N_LIB} SHARED ${l10n_SRCS})
set_target_properties(
    ${PUSHKIN_L10N_LIB}
    PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)
target_link_libraries(
    ${PUSHKIN_L10N_LIB}
    ${Boost_LOCALE_LIBRARIES}
)

set(l10n_gen_SRCS
    po_generator.cpp
)
localize(SOURCES ${l10n_gen_SRCS} INSTALL)
add_library(${PUSHKIN_L10N_GEN_LIB} SHARED ${l10n_gen_SRCS})
set_target_properties(
    ${PUSHKIN_L10N_GEN_LIB}
    PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
)
target_link_libraries(
    ${PUSHKIN_L10N_GEN_LIB}
    ${PUSHKIN_L10N_LIB}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
)

install (
    TARGETS ${PUSHKIN_L10N_LIB}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
