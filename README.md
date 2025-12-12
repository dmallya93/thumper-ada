# Thumper - RFC 3161 Timestamp Server

A C++20 implementation of an RFC 3161-compliant timestamp server, migrated from Ada/SPARK.

**Source:** https://github.com/dmallya93/thumper-ada.git

## Overview

Thumper is a secure, safety-critical timestamp server that provides cryptographically verifiable timestamps for documents. This C++ implementation maintains the strong type safety and reliability principles of the original Ada/SPARK codebase while leveraging modern C++ features and ecosystem.

## Requirements

### Build Dependencies

- **C++ Compiler:**
  - Clang 20+ (primary development compiler)
  - GCC 14+ (for compatibility testing)
- **Build System:**
  - CMake 3.20+
  - Make (Unix Makefiles)
- **Libraries:**
  - Google Test (gtest) - Unit testing framework
  - OpenSSL - Cryptographic operations
- **Static Analysis:**
  - clang-tidy-20 for static analysis
  - clang-format-20 for code formatting

### Installation on Debian/Ubuntu

```bash
# Install Clang and tools
apt-get install clang-20 clang-tidy-20 clang-format-20

# Install build dependencies
apt-get install cmake make

# Install libraries
apt-get install libgtest-dev libssl-dev
```

## Building

The project uses CMake Presets for standardized build configurations.

### Quick Start

```bash
# Configure with Clang debug build (includes sanitizers and clang-tidy)
cmake --preset clang-debug

# Build
cmake --build --preset clang-debug

# Run tests
ctest --preset clang-debug
```

### Available Presets

| Preset | Compiler | Build Type | Sanitizers | Clang-Tidy |
|--------|----------|------------|------------|------------|
| `clang-debug` | Clang | Debug | Enabled | Enabled |
| `clang-release` | Clang | Release | Disabled | Disabled |
| `gcc-debug` | GCC | Debug | Enabled | Disabled |
| `gcc-release` | GCC | Release | Disabled | Disabled |

### Build Options

You can override default options by passing `-D` flags to CMake:

```bash
# Disable sanitizers for faster iteration
cmake --preset clang-debug -DENABLE_SANITIZERS=OFF

# Enable warnings as errors (recommended for CI/CD)
cmake --preset clang-debug -DWARNINGS_AS_ERRORS=ON

# Disable clang-tidy for faster builds
cmake --preset clang-debug -DENABLE_CLANG_TIDY=OFF
```

## Testing

### Running Tests

```bash
# Run all tests with the configured preset
ctest --preset clang-debug

# Run tests with verbose output
ctest --preset clang-debug --verbose

# Run specific test
ctest --preset clang-debug -R TrivialTest
```

### Code Coverage

Code coverage support will be added in future milestones.

## Code Quality

### Static Analysis

The project uses clang-tidy with strict checks based on:
- SEI CERT C++ Coding Standard (`cert-*`)
- C++ Core Guidelines (`cppcoreguidelines-*`)
- Modernization checks (`modernize-*`)
- Performance checks (`performance-*`)

Static analysis runs automatically during compilation when `ENABLE_CLANG_TIDY=ON`.

To run clang-tidy manually:

```bash
cmake --build --preset clang-debug --target run-clang-tidy
```

### Code Formatting

The project uses clang-format based on LLVM style with custom adjustments.

```bash
# Check formatting without modifying files
cmake --build --preset clang-debug --target format-check

# Apply formatting to all source files
cmake --build --preset clang-debug --target format
```

## Runtime Analysis

### Sanitizers

Debug builds enable AddressSanitizer and UndefinedBehaviorSanitizer by default to catch:
- Memory errors (use-after-free, buffer overruns, memory leaks)
- Undefined behavior (integer overflow, null dereferences, etc.)

Sanitizers add runtime overhead but provide comprehensive safety checking. Disable them for performance testing:

```bash
cmake --preset clang-debug -DENABLE_SANITIZERS=OFF
```

## Project Structure

```
thumper-ada/
├── include/thumper/     # Public headers
│   └── types.hpp        # Foundational type definitions
├── src/                 # Implementation files
├── tests/               # Unit tests
│   ├── test_main.cpp    # Google Test entry point
│   └── test_trivial.cpp # Basic type tests
├── build/               # Build artifacts (generated, not in repo)
├── CMakeLists.txt       # CMake build configuration
├── CMakePresets.json    # Standard build presets
├── .clang-tidy          # Static analysis rules
├── .clang-format        # Code formatting rules
└── .gitignore           # Git ignore patterns
```

## Development Workflow

### Typical Development Cycle

```bash
# 1. Configure (once, or when CMakeLists.txt changes)
cmake --preset clang-debug

# 2. Build (iterative)
cmake --build --preset clang-debug

# 3. Test (after changes)
ctest --preset clang-debug

# 4. Check formatting
cmake --build --preset clang-debug --target format-check

# 5. Apply formatting if needed
cmake --build --preset clang-debug --target format
```

### Recommended Development Settings

- Use `clang-debug` preset during development for maximum safety checking
- Use `clang-release` preset for performance testing and final builds
- Test with both Clang and GCC presets before committing to catch compiler-specific issues
- Enable `-DWARNINGS_AS_ERRORS=ON` in CI/CD to enforce code quality

## Migration Notes

This project is a migration from Ada/SPARK to C++20. Key design decisions:

1. **Byte Representation:** Uses `uint8_t` (aliased as `thumper::Octet`) for compatibility with OpenSSL and clear numeric semantics
2. **Dynamic Arrays:** Uses `std::vector<uint8_t>` (aliased as `thumper::OctetArray`) for automatic memory management
3. **Safety Approach:** Combines static analysis (clang-tidy), runtime sanitizers, and modern C++ practices to approximate SPARK's formal verification guarantees

## License

See the original Ada/SPARK project for license information.

## Contributing

When contributing:
1. Follow the existing code style (enforced by clang-format)
2. Ensure all tests pass with sanitizers enabled
3. Fix all clang-tidy warnings (configured with `WarningsAsErrors`)
4. Add unit tests for new functionality
5. Test with both Clang and GCC configurations
