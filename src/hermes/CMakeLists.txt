###########################################################################
# CMake configuration for the Hermes ASN.1 library
#
# This builds Hermes as a static library that can be linked with the
# Thumper timestamp server and client.
#
# Requirements:
#   - CMake 3.20 or later
#   - C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
###########################################################################

cmake_minimum_required(VERSION 3.20)

project(Hermes
    VERSION 1.0.0
    DESCRIPTION "ASN.1 DER encoding/decoding library"
    LANGUAGES CXX
)

# Require C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Hermes static library
add_library(hermes STATIC
    src/hermes.cpp
    src/oid.cpp
)

# Public include directory
target_include_directories(hermes
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Installation rules
include(GNUInstallDirs)

install(TARGETS hermes
    EXPORT HermesTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT HermesTargets
    FILE HermesTargets.cmake
    NAMESPACE Hermes::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Hermes
)

# Generate package config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/HermesConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/HermesConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Hermes
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/HermesConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/HermesConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/HermesConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Hermes
)

# Testing
include(CTest)
enable_testing()

# Find Google Test
find_package(GTest REQUIRED)

# OID tests
add_executable(check_oid_test
    tests/check_oid_test.cpp
)

target_link_libraries(check_oid_test
    PRIVATE
        hermes
        GTest::gtest
        GTest::gtest_main
)

# Register test with CTest
add_test(NAME OIDTests COMMAND check_oid_test)

