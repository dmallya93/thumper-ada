//---------------------------------------------------------------------------
// FILE    : check_trivial_test.cpp
// SUBJECT : Test cases for nothing in particular.
// AUTHOR  : (C) Copyright 2015 by Peter C. Chapin
//           Modernized to C++ 2025
//
// The "trivial" test is just a placeholder. It can be used as a template
// for other tests. This test is DISABLED by default as it contains no
// real test logic.
//
// To create a new test suite, copy this file and:
//   1. Change the test suite name (TrivialTest -> YourTestSuiteName)
//   2. Remove the DISABLED_ prefix
//   3. Add actual test logic with EXPECT_* or ASSERT_* macros
//   4. Add the new test file to CMakeLists.txt
//
// Please send comments or bug reports to
//
//      Peter C. Chapin <chapinp@acm.org>
//---------------------------------------------------------------------------

#include <gtest/gtest.h>

///////////////////////////////////////////////////////////////////////////
// Trivial placeholder tests
///////////////////////////////////////////////////////////////////////////

// This test is disabled by default as it's just a placeholder.
// Use DISABLED_ prefix to prevent it from running.
TEST(TrivialTest, DISABLED_Nothing) {
    // This is where you would put actual test assertions.
    // Examples:
    //   EXPECT_EQ(0, 0) << "Zero should equal zero!";
    //   EXPECT_TRUE(true) << "True should be true!";
    //   ASSERT_NE(nullptr, some_pointer) << "Pointer should not be null!";

    // For now, this test does nothing and is disabled.
}

// Another example of a disabled test that could serve as a template
TEST(TrivialTest, DISABLED_AnotherExample) {
    // You can have multiple disabled tests as templates
    // Remove DISABLED_ when you add real test logic
}
