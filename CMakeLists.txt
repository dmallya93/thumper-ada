###########################################################################
# CMake configuration for the Thumper Timestamp System
#
# This is the top-level CMake configuration for the Thumper project,
# which includes:
#   - Hermes ASN.1 library
#   - Core timestamp logic (future)
#   - Timestamp server (future)
#   - GUI client (future)
#   - Tests (future)
#
# Requirements:
#   - CMake 3.20 or later
#   - C++20 compatible compiler
###########################################################################

cmake_minimum_required(VERSION 3.20)

project(Thumper
    VERSION 1.0.0
    DESCRIPTION "RFC-3161 Timestamp Server and Client"
    LANGUAGES CXX
)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Global C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Subdirectories
add_subdirectory(src/hermes)

# Future subdirectories will be added here:
# add_subdirectory(src/core)
# add_subdirectory(src/server)
# add_subdirectory(src/client)
# add_subdirectory(tests)

