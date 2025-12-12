cmake_minimum_required(VERSION 3.20)
project(Thumper VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard required for modern features (concepts, ranges, etc.)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for clang-tidy and IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler-specific warning flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wmisleading-indentation
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -Wmisleading-indentation
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wuseless-cast
    )
elseif(MSVC)
    add_compile_options(/W4 /permissive-)
endif()

# Optional: Warnings as errors (enable in CI/CD)
option(WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
if(WARNINGS_AS_ERRORS)
    if(MSVC)
        add_compile_options(/WX)
    else()
        add_compile_options(-Werror)
    endif()
endif()

# Sanitizers: Enabled by default in Debug builds
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" ON)
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        message(STATUS "Enabling AddressSanitizer and UndefinedBehaviorSanitizer")
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()

# Clang-tidy integration
option(ENABLE_CLANG_TIDY "Enable clang-tidy static analysis" ON)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy-20" "clang-tidy")
    if(CLANG_TIDY_EXE)
        message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy not found, static analysis disabled")
    endif()
endif()

# Find dependencies
find_package(GTest REQUIRED)

# OpenSSL for cryptographic operations (RSA signatures, hashing)
find_package(OpenSSL REQUIRED)

# Main library target
add_library(thumper_core
    src/hermes/hermes.cpp
)

target_include_directories(thumper_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Testing
enable_testing()

add_executable(thumper_tests
    tests/test_main.cpp
    tests/test_trivial.cpp
    tests/test_hermes_core.cpp
)

target_link_libraries(thumper_tests
    PRIVATE
        thumper_core
        GTest::gtest
        GTest::gtest_main
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(thumper_tests)

# Custom targets for code quality
find_program(CLANG_FORMAT_EXE NAMES "clang-format-20" "clang-format")
if(CLANG_FORMAT_EXE)
    # Collect all source files for formatting
    file(GLOB_RECURSE FORMAT_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/thumper/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    )

    if(FORMAT_SOURCES)
        # Format check target
        add_custom_target(format-check
            COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${FORMAT_SOURCES}
            COMMENT "Checking code formatting"
        )

        # Format apply target
        add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXE} -i ${FORMAT_SOURCES}
            COMMENT "Applying code formatting"
        )
    endif()
endif()

# Manual clang-tidy target (in addition to automatic integration)
if(CLANG_TIDY_EXE)
    # Collect source files for clang-tidy
    file(GLOB_RECURSE TIDY_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
    )

    if(TIDY_SOURCES)
        add_custom_target(run-clang-tidy
            COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} ${TIDY_SOURCES}
            COMMENT "Running clang-tidy"
        )
    endif()
endif()

# Installation rules (optional)
install(TARGETS thumper_core
    EXPORT ThumperTargets
)

install(DIRECTORY include/
    DESTINATION include
)
