//---------------------------------------------------------------------------
// FILE    : hermes_test.cpp
// SUBJECT : Main procedure of the Hermes unit test program.
// AUTHOR  : (C) Copyright 2015 by Peter Chapin
//           Modernized to C++ 2025
//
// This is the main test runner that executes all Hermes test suites:
//   - OID tests (check_oid_test.cpp)
//   - DER encoding tests (check_der_encode_test.cpp)
//   - DER decoding tests (check_der_decode_test.cpp)
//
// Google Test automatically discovers and runs all TEST() and TEST_F()
// test cases linked into this executable. No explicit suite construction
// is needed as in the original Ada/AUnit implementation.
//
// Usage:
//   ./hermes_test                      # Run all tests
//   ./hermes_test --gtest_filter=OID*  # Run only OID tests
//   ./hermes_test --help               # Show Google Test options
//
// Please send comments or bug reports to
//
//      Peter Chapin <chapinp@acm.org>
//---------------------------------------------------------------------------

#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
