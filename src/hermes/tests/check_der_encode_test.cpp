///////////////////////////////////////////////////////////////////////////
// FILE    : check_der_encode_test.cpp
// SUBJECT : Test cases for DER encoding functions
// AUTHOR  : (C) Copyright 2022 by Peter Chapin
//
// This file contains comprehensive unit tests for the DER encoding
// functionality. Test vectors are taken directly from the Ada/SPARK
// implementation to ensure binary compatibility.
//
// Maps from Ada test file: check_der_encode.adb
//
///////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>
#include "hermes/der_encode.hpp"
#include <vector>

using namespace hermes;
using namespace hermes::der;
using namespace hermes::der::encode;

// Helper function to compare octet arrays
bool compareOctetArrays(const OctetArray& actual, const std::vector<Octet>& expected)
{
    if (actual.size() != expected.size()) {
        return false;
    }
    for (std::size_t i = 0; i < actual.size(); ++i) {
        if (actual[i] != expected[i]) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////
// Test_Put_Length - Tests length encoding with 10 test cases
// Maps from Ada: Test_Put_Length in check_der_encode.adb:21-71
///////////////////////////////////////////////////////////////////////////

TEST(DEREncodeTest, PutLength_Zero) {
    // Test case 1: Length 0
    OctetArray result = putLengthValue(0);
    std::vector<Octet> expected = {0x00};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #1 failed (length 0)";
}

TEST(DEREncodeTest, PutLength_One) {
    // Test case 2: Length 1
    OctetArray result = putLengthValue(1);
    std::vector<Octet> expected = {0x01};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #2 failed (length 1)";
}

TEST(DEREncodeTest, PutLength_127) {
    // Test case 3: Length 127 (max short form)
    OctetArray result = putLengthValue(127);
    std::vector<Octet> expected = {0x7F};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #3 failed (length 127)";
}

TEST(DEREncodeTest, PutLength_128) {
    // Test case 4: Length 128 (min long form with 1 byte)
    OctetArray result = putLengthValue(128);
    std::vector<Octet> expected = {0x81, 0x80};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #4 failed (length 128)";
}

TEST(DEREncodeTest, PutLength_255) {
    // Test case 5: Length 255 (max long form with 1 byte)
    OctetArray result = putLengthValue(255);
    std::vector<Octet> expected = {0x81, 0xFF};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #5 failed (length 255)";
}

TEST(DEREncodeTest, PutLength_256) {
    // Test case 6: Length 256 (min long form with 2 bytes)
    OctetArray result = putLengthValue(256);
    std::vector<Octet> expected = {0x82, 0x01, 0x00};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #6 failed (length 256)";
}

TEST(DEREncodeTest, PutLength_65535) {
    // Test case 7: Length 65535 (max long form with 2 bytes)
    OctetArray result = putLengthValue(0xFFFF);
    std::vector<Octet> expected = {0x82, 0xFF, 0xFF};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #7 failed (length 65535)";
}

TEST(DEREncodeTest, PutLength_65536) {
    // Test case 8: Length 65536 (min long form with 3 bytes)
    OctetArray result = putLengthValue(0x10000);
    std::vector<Octet> expected = {0x83, 0x01, 0x00, 0x00};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #8 failed (length 65536)";
}

TEST(DEREncodeTest, PutLength_16711680) {
    // Test case 9: Length 0xFF0000
    OctetArray result = putLengthValue(0xFF0000);
    std::vector<Octet> expected = {0x83, 0xFF, 0x00, 0x00};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #9 failed (length 16711680)";
}

TEST(DEREncodeTest, PutLength_2147483647) {
    // Test case 10: Length 0x7FFFFFFF (max positive signed 32-bit)
    OctetArray result = putLengthValue(0x7FFFFFFF);
    std::vector<Octet> expected = {0x84, 0x7F, 0xFF, 0xFF, 0xFF};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #10 failed (length 2147483647)";
}

///////////////////////////////////////////////////////////////////////////
// Test_Put_Boolean - Tests boolean encoding with 2 test cases
// Maps from Ada: Test_Put_Boolean in check_der_encode.adb:74-82
///////////////////////////////////////////////////////////////////////////

TEST(DEREncodeTest, PutBoolean_False) {
    // Test case: Boolean false
    OctetArray result = putBooleanValue(false);
    std::vector<Octet> expected = {0x01, 0x01, 0x00};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Boolean false test failed";
}

TEST(DEREncodeTest, PutBoolean_True) {
    // Test case: Boolean true
    OctetArray result = putBooleanValue(true);
    std::vector<Octet> expected = {0x01, 0x01, 0xFF};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Boolean true test failed";
}

///////////////////////////////////////////////////////////////////////////
// Test_Put_Integer - Tests integer encoding with 10 test cases
// Maps from Ada: Test_Put_Integer in check_der_encode.adb:85-135
///////////////////////////////////////////////////////////////////////////

TEST(DEREncodeTest, PutInteger_0) {
    // Test case 1: Integer 0
    OctetArray result = putIntegerValue(0);
    std::vector<Octet> expected = {0x02, 0x01, 0x00};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #1 failed (integer 0)";
}

TEST(DEREncodeTest, PutInteger_1) {
    // Test case 2: Integer 1
    OctetArray result = putIntegerValue(1);
    std::vector<Octet> expected = {0x02, 0x01, 0x01};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #2 failed (integer 1)";
}

TEST(DEREncodeTest, PutInteger_127) {
    // Test case 3: Integer 127 (max 1-byte value without padding)
    OctetArray result = putIntegerValue(127);
    std::vector<Octet> expected = {0x02, 0x01, 0x7F};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #3 failed (integer 127)";
}

TEST(DEREncodeTest, PutInteger_128) {
    // Test case 4: Integer 128 (needs 2 bytes - high bit set requires padding)
    OctetArray result = putIntegerValue(128);
    std::vector<Octet> expected = {0x02, 0x02, 0x00, 0x80};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #4 failed (integer 128)";
}

TEST(DEREncodeTest, PutInteger_255) {
    // Test case 5: Integer 255
    OctetArray result = putIntegerValue(255);
    std::vector<Octet> expected = {0x02, 0x02, 0x00, 0xFF};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #5 failed (integer 255)";
}

TEST(DEREncodeTest, PutInteger_256) {
    // Test case 6: Integer 256
    OctetArray result = putIntegerValue(256);
    std::vector<Octet> expected = {0x02, 0x02, 0x01, 0x00};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #6 failed (integer 256)";
}

TEST(DEREncodeTest, PutInteger_65535) {
    // Test case 7: Integer 65535 (max 2-byte value, needs padding)
    OctetArray result = putIntegerValue(0xFFFF);
    std::vector<Octet> expected = {0x02, 0x03, 0x00, 0xFF, 0xFF};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #7 failed (integer 65535)";
}

TEST(DEREncodeTest, PutInteger_65536) {
    // Test case 8: Integer 65536
    OctetArray result = putIntegerValue(0x10000);
    std::vector<Octet> expected = {0x02, 0x03, 0x01, 0x00, 0x00};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #8 failed (integer 65536)";
}

TEST(DEREncodeTest, PutInteger_16711680) {
    // Test case 9: Integer 0xFF0000 (needs padding)
    OctetArray result = putIntegerValue(0xFF0000);
    std::vector<Octet> expected = {0x02, 0x04, 0x00, 0xFF, 0x00, 0x00};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #9 failed (integer 16711680)";
}

TEST(DEREncodeTest, PutInteger_2147483647) {
    // Test case 10: Integer 0x7FFFFFFF (max positive signed 32-bit)
    OctetArray result = putIntegerValue(0x7FFFFFFF);
    std::vector<Octet> expected = {0x02, 0x04, 0x7F, 0xFF, 0xFF, 0xFF};
    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "Test case #10 failed (integer 2147483647)";
}

///////////////////////////////////////////////////////////////////////////
// Test_Put_OID - Tests OID encoding with SHA-256 test case
// Maps from Ada: Test_Put_OID in check_der_encode.adb:138-177
///////////////////////////////////////////////////////////////////////////

TEST(DEREncodeTest, PutOID_SHA256) {
    // Test case 1: SHA-256 algorithm OID (2.16.840.1.101.3.4.2.1)
    // This is the OID for the SHA-256 hashing algorithm used in RFC-3161

    // Create OID from component array
    oid::ComponentArray components = {2, 16, 840, 1, 101, 3, 4, 2, 1};
    oid::ObjectIdentifier oid;
    oid::Status status = oid::toObjectIdentifier(components, oid);

    ASSERT_EQ(status, oid::Status::Success)
        << "Failed to create OID from components";

    // Encode the OID
    OctetArray result = putOIDValue(oid);

    // Expected encoding:
    // Tag: 0x06 (Universal, Primitive, ObjectIdentifier)
    // Length: 0x09 (9 bytes of encoded components)
    // Value:
    //   First byte: 40*2 + 16 = 96 = 0x60
    //   840 = 0x348 = 0b0011_0100_1000 -> 0b110_1001000 -> [0x86, 0x48]
    //   1 = 0x01
    //   101 = 0x65
    //   3 = 0x03
    //   4 = 0x04
    //   2 = 0x02
    //   1 = 0x01
    std::vector<Octet> expected = {
        0x06, 0x09,  // Tag and Length
        0x60,        // First two components: 40*2+16
        0x86, 0x48,  // 840 in base-128
        0x01,        // 1
        0x65,        // 101
        0x03,        // 3
        0x04,        // 4
        0x02,        // 2
        0x01         // 1
    };

    EXPECT_TRUE(compareOctetArrays(result, expected))
        << "SHA-256 OID encoding failed";
}

///////////////////////////////////////////////////////////////////////////
// Main function for running all tests
///////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
