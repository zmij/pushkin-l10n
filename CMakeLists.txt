#    CMakeLists.txt for tip-l10n library
#
#    @author zmij

cmake_minimum_required(VERSION 2.6)

set(lib_name l10n)
string(TOUPPER ${lib_name} LIB_NAME)

if (NOT PROJECT_PREFIX)
    set(PROJECT_PREFIX pushkin)
endif()

if (PROJECT_VERSION)
    # Built as a subproject
    set(_pversion ${PROJECT_VERSION})
else()
    set(_pversion 0.1.0)
endif()

set(_pname ${PROJECT_PREFIX}-l10n)

if (${CMAKE_VERSION} VERSION_GREATER "3.0")
    cmake_policy(SET CMP0048 NEW)
    project(${_pname} VERSION ${_pversion})
else()
    project(${_pname})
    set(PROJECT_VERSION ${_pversion})
endif()

option(BUILD_TESTS "Build tests for ${lib_name} library" ON)
#if(NOT CEREAL_INCLUDE_DIR)
#    message(FATAL_ERROR "Please set CEREAL_INCLUDE_DIR (requires cereal fork https://github.com/zmij/cereal)")
#endif()

add_definitions("-std=c++11")

set(BOOST_COMPONENTS
    system
    thread
    locale
)

find_package(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
find_package (Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CEREAL_INCLUDE_DIR}
)

set(PUSHKIN_${LIB_NAME}_LIB ${PROJECT_PREFIX}-${lib_name})

add_subdirectory(include/pushkin)
add_subdirectory(src)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

get_directory_property(has_parent PARENT_DIRECTORY)
if (has_parent)
    set(PUSHKIN_${LIB_NAME}_LIBRARIES ${PUSHKIN_${LIB_NAME}_LIB} CACHE INTERNAL "Name of tip-util library target")
    set(PUSHKIN_${LIB_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "Path to tip-util libaray includes" )
endif()
