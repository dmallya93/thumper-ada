# Thumper Timestamp Authority (C++ Modernization)

This is a modernized C++ implementation of the Thumper Timestamp Authority, originally written in Ada/SPARK. Thumper provides RFC 3161 compliant timestamp services.

**Source Repository:** https://github.com/dmallya93/thumper-ada.git
**Migration Status:** Milestone 1 (Hermes ASN.1 Library) - Complete
**Created:** Wed Jan 14 20:16:24 UTC 2026

## Overview

Thumper is a timestamp server and client implementation that provides cryptographic timestamp tokens per RFC 3161. The system consists of:

- **Hermes**: ASN.1 DER encoding/decoding library (currently implemented)
- **Server**: Timestamp authority server (planned)
- **Client**: GUI client for requesting and verifying timestamps (planned)

## Current Status

### Milestone 1: Hermes ASN.1 Library Foundation ✓

The Hermes library has been successfully migrated from Ada/SPARK to C++20. It provides:

- Core ASN.1 DER encoding and decoding functions
- Object Identifier (OID) support with string conversions
- Type-safe octet array handling
- Comprehensive test suite with 103 passing tests

## Building

### Requirements

- CMake 3.20 or later
- C++20 compatible compiler:
  - GCC 10+
  - Clang 10+
  - MSVC 2019+
- Google Test (for testing)

### Build Instructions

```bash
# From the repository root
mkdir build
cd build
cmake ../src/hermes -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

For debug builds with extra checks:
```bash
cmake ../src/hermes -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)
```

## Testing

The Hermes library includes a comprehensive test suite with 103 test cases covering:
- Object Identifier (OID) operations (31 tests)
- DER encoding (23 tests)
- DER decoding (49 tests)

### Running All Tests

```bash
# From the build directory

# Option 1: Run the unified test executable directly
./hermes_test

# Option 2: Run via CTest
ctest --output-on-failure

# Option 3: Run with verbose output
make test
```

### Running Specific Tests

Google Test supports filtering to run specific test suites or test cases:

```bash
# Run only OID tests
./hermes_test --gtest_filter=OIDTest.*

# Run only DER encoding tests
./hermes_test --gtest_filter=DEREncodeTest.*

# Run only DER decoding tests
./hermes_test --gtest_filter=DERDecodeTest.*

# Run a specific test case
./hermes_test --gtest_filter=OIDTest.RoundTrip_SHA256

# Run tests matching a pattern
./hermes_test --gtest_filter=*RoundTrip*

# See all available options
./hermes_test --help
```

### Test Organization

The test suite is organized into separate modules:

- **check_oid_test.cpp**: Object Identifier validation and conversion tests
- **check_der_encode_test.cpp**: DER encoding tests for all ASN.1 primitive types
- **check_der_decode_test.cpp**: DER decoding tests including round-trip validation
- **check_trivial_test.cpp**: Template for creating new tests (disabled by default)
- **hermes_test.cpp**: Main test runner that aggregates all test modules

All test modules are linked into a single `hermes_test` executable, mirroring the Ada AUnit `hermes_suite` structure.

## Project Structure

```
src/
  hermes/               # ASN.1 library
    include/
      hermes/
        hermes.h        # Core types and utilities
        der.h           # DER encoding/decoding
        oid.h           # Object Identifier support
    src/
      hermes.cpp
      der_encode.cpp
      der_decode.cpp
      oid.cpp
    tests/
      hermes_test.cpp           # Main test runner
      check_oid_test.cpp        # OID tests
      check_der_encode_test.cpp # Encoding tests
      check_der_decode_test.cpp # Decoding tests
      check_trivial_test.cpp    # Test template
    CMakeLists.txt
```

## Installation

To install the Hermes library for use by other components:

```bash
# From the build directory
sudo cmake --install .
```

This installs:
- Headers to `/usr/local/include/hermes/`
- Library to `/usr/local/lib/`
- CMake config files for easy integration

## Migration Notes

This C++ implementation aims to preserve the behavior of the original Ada/SPARK implementation while leveraging modern C++ features for safety and expressiveness:

- **Type Safety**: Uses `enum class`, `std::array`, `std::span`, and C++20 concepts
- **Error Handling**: Status enumerations matching Ada's approach for explicit error checking
- **Testing**: Comprehensive test suite ported from Ada AUnit to Google Test
- **Build System**: CMake replaces GNAT project files

Key differences from the Ada version:
- No formal verification (SPARK contracts replaced with runtime checks and tests)
- Dynamic memory allocation available but used sparingly
- Standard C++ library types where appropriate

## License

(License information to be added based on original project)

## Contact

Peter Chapin <chapinp@acm.org>

## Next Milestones

- [ ] Milestone 2: Core Types and Cryptographic Services
- [ ] Milestone 3: Server Core (UDP, Database, RFC 3161)
- [ ] Milestone 4: HTTP/HTTPS Interface
- [ ] Milestone 5: Client GUI (GTK)
