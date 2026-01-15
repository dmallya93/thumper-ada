///////////////////////////////////////////////////////////////////////////
// FILE    : check_oid_test.cpp
// SUBJECT : Unit tests for Object Identifier support
// AUTHOR  : (C) Copyright 2022 by Peter C. Chapin
//
// Comprehensive tests for OID construction, conversion, and validation.
// Replaces the incomplete Ada test in check_oid.adb.
//
// Maps from Ada test file: check_oid.ads/adb
///////////////////////////////////////////////////////////////////////////

#include "hermes/oid.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace hermes::oid;

// Test fixture for OID tests
class OIDTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

///////////////////////////////////////////////////////////////////////////
// Test: Round Trip Conversion
// Maps from Ada: Test_Round_Trip procedure (which was unimplemented)
///////////////////////////////////////////////////////////////////////////

TEST_F(OIDTest, RoundTrip_ValidOID_Root0) {
    // Test OID with root component 0
    // Example: 0.9.2342.19200300.100.1.1 (uid attribute)
    ComponentArray original = {0, 9, 2342, 19200300, 100, 1, 1};
    ObjectIdentifier oid;
    Status status;

    // Convert to OID
    status = toObjectIdentifier(original, oid);
    ASSERT_EQ(status, Status::Success) << "Failed to create OID with root 0";

    // Verify component count
    EXPECT_EQ(oid.componentCount(), original.size());

    // Convert back to components
    ComponentArray result;
    std::size_t numComponents;
    status = toSeparates(oid, result, numComponents);
    ASSERT_EQ(status, Status::Success) << "Failed to convert OID back to components";

    // Verify the components match
    ASSERT_EQ(numComponents, original.size());
    ASSERT_EQ(result.size(), original.size());
    for (std::size_t i = 0; i < original.size(); ++i) {
        EXPECT_EQ(result[i], original[i]) << "Component mismatch at index " << i;
    }
}

TEST_F(OIDTest, RoundTrip_ValidOID_Root1) {
    // Test OID with root component 1
    // Example: 1.2.840.113549 (RSA algorithm OID prefix)
    ComponentArray original = {1, 2, 840, 113549};
    ObjectIdentifier oid;
    Status status;

    status = toObjectIdentifier(original, oid);
    ASSERT_EQ(status, Status::Success) << "Failed to create OID with root 1";

    EXPECT_EQ(oid.componentCount(), original.size());

    ComponentArray result;
    std::size_t numComponents;
    status = toSeparates(oid, result, numComponents);
    ASSERT_EQ(status, Status::Success);

    ASSERT_EQ(numComponents, original.size());
    EXPECT_EQ(result, original);
}

TEST_F(OIDTest, RoundTrip_ValidOID_Root2) {
    // Test OID with root component 2
    // Example: 2.16.840.1.101.3.4.2.1 (SHA-256 algorithm OID)
    ComponentArray original = {2, 16, 840, 1, 101, 3, 4, 2, 1};
    ObjectIdentifier oid;
    Status status;

    status = toObjectIdentifier(original, oid);
    ASSERT_EQ(status, Status::Success) << "Failed to create OID with root 2";

    EXPECT_EQ(oid.componentCount(), original.size());

    ComponentArray result;
    std::size_t numComponents;
    status = toSeparates(oid, result, numComponents);
    ASSERT_EQ(status, Status::Success);

    ASSERT_EQ(numComponents, original.size());
    EXPECT_EQ(result, original);
}

TEST_F(OIDTest, RoundTrip_MinimalOID) {
    // Test minimal OID with only root and second level
    ComponentArray original = {1, 2};
    ObjectIdentifier oid;

    Status status = toObjectIdentifier(original, oid);
    ASSERT_EQ(status, Status::Success);

    EXPECT_EQ(oid.componentCount(), 2);

    ComponentArray result;
    std::size_t numComponents;
    status = toSeparates(oid, result, numComponents);
    ASSERT_EQ(status, Status::Success);

    EXPECT_EQ(result, original);
}

TEST_F(OIDTest, RoundTrip_MaximalComponents) {
    // Test OID with maximum number of components (15)
    ComponentArray original = {2, 16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    ASSERT_EQ(original.size(), MaximumComponentCount);

    ObjectIdentifier oid;
    Status status = toObjectIdentifier(original, oid);
    ASSERT_EQ(status, Status::Success);

    EXPECT_EQ(oid.componentCount(), MaximumComponentCount);

    ComponentArray result;
    std::size_t numComponents;
    status = toSeparates(oid, result, numComponents);
    ASSERT_EQ(status, Status::Success);

    EXPECT_EQ(result, original);
}

///////////////////////////////////////////////////////////////////////////
// Test: Invalid Root Component
///////////////////////////////////////////////////////////////////////////

TEST_F(OIDTest, InvalidRoot_ValueTooLarge) {
    // Root component must be 0, 1, or 2
    ComponentArray components = {3, 5};
    ObjectIdentifier oid;

    Status status = toObjectIdentifier(components, oid);
    EXPECT_EQ(status, Status::InvalidRoot);
}

TEST_F(OIDTest, InvalidRoot_EmptyArray) {
    ComponentArray components;  // Empty
    ObjectIdentifier oid;

    Status status = toObjectIdentifier(components, oid);
    EXPECT_EQ(status, Status::InvalidRoot);
}

TEST_F(OIDTest, InvalidRoot_OnlyOneComponent) {
    ComponentArray components = {1};
    ObjectIdentifier oid;

    Status status = toObjectIdentifier(components, oid);
    EXPECT_EQ(status, Status::InvalidSecondLevel);
}

///////////////////////////////////////////////////////////////////////////
// Test: Invalid Second Level Component
///////////////////////////////////////////////////////////////////////////

TEST_F(OIDTest, InvalidSecondLevel_Root0_SecondTooLarge) {
    // For root 0, second component must be < 40
    ComponentArray components = {0, 40};
    ObjectIdentifier oid;

    Status status = toObjectIdentifier(components, oid);
    EXPECT_EQ(status, Status::InvalidSecondLevel);
}

TEST_F(OIDTest, InvalidSecondLevel_Root1_SecondTooLarge) {
    // For root 1, second component must be < 40
    ComponentArray components = {1, 40};
    ObjectIdentifier oid;

    Status status = toObjectIdentifier(components, oid);
    EXPECT_EQ(status, Status::InvalidSecondLevel);
}

TEST_F(OIDTest, InvalidSecondLevel_Root2_SecondTooLarge) {
    // For root 2, second component must be <= 175
    ComponentArray components = {2, 176};
    ObjectIdentifier oid;

    Status status = toObjectIdentifier(components, oid);
    EXPECT_EQ(status, Status::InvalidSecondLevel);
}

TEST_F(OIDTest, ValidSecondLevel_Root2_SecondAt175) {
    // For root 2, second component can be exactly 175
    ComponentArray components = {2, 175};
    ObjectIdentifier oid;

    Status status = toObjectIdentifier(components, oid);
    EXPECT_EQ(status, Status::Success);
    EXPECT_EQ(oid.getRootComponent(), 2);
    EXPECT_EQ(oid.getSecondLevelComponent(), 175);
}

TEST_F(OIDTest, ValidSecondLevel_Root0_SecondAt39) {
    // For root 0, second component can be exactly 39
    ComponentArray components = {0, 39};
    ObjectIdentifier oid;

    Status status = toObjectIdentifier(components, oid);
    EXPECT_EQ(status, Status::Success);
}

TEST_F(OIDTest, ValidSecondLevel_Root1_SecondAt39) {
    // For root 1, second component can be exactly 39
    ComponentArray components = {1, 39};
    ObjectIdentifier oid;

    Status status = toObjectIdentifier(components, oid);
    EXPECT_EQ(status, Status::Success);
}

///////////////////////////////////////////////////////////////////////////
// Test: Component Count
///////////////////////////////////////////////////////////////////////////

TEST_F(OIDTest, ComponentCount_VariousSizes) {
    // Test component count for various OID sizes

    // 2 components
    ComponentArray comp2 = {1, 2};
    ObjectIdentifier oid2;
    ASSERT_EQ(toObjectIdentifier(comp2, oid2), Status::Success);
    EXPECT_EQ(oid2.componentCount(), 2);

    // 5 components
    ComponentArray comp5 = {2, 16, 840, 1, 101};
    ObjectIdentifier oid5;
    ASSERT_EQ(toObjectIdentifier(comp5, oid5), Status::Success);
    EXPECT_EQ(oid5.componentCount(), 5);

    // 15 components (maximum)
    ComponentArray comp15 = {2, 16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    ObjectIdentifier oid15;
    ASSERT_EQ(toObjectIdentifier(comp15, oid15), Status::Success);
    EXPECT_EQ(oid15.componentCount(), 15);
}

///////////////////////////////////////////////////////////////////////////
// Test: String Conversions
///////////////////////////////////////////////////////////////////////////

TEST_F(OIDTest, OIDToString_SimpleOID) {
    ComponentArray components = {1, 2, 840, 113549};
    std::string expected = "1.2.840.113549";

    std::string result = oidToString(components);
    EXPECT_EQ(result, expected);
}

TEST_F(OIDTest, OIDToString_SHA256) {
    // SHA-256 algorithm OID
    ComponentArray components = {2, 16, 840, 1, 101, 3, 4, 2, 1};
    std::string expected = "2.16.840.1.101.3.4.2.1";

    std::string result = oidToString(components);
    EXPECT_EQ(result, expected);
}

TEST_F(OIDTest, OIDToString_EmptyArray) {
    ComponentArray components;
    std::string result = oidToString(components);
    EXPECT_EQ(result, "");
}

TEST_F(OIDTest, StringToOID_SimpleOID) {
    std::string oidStr = "1.2.840.113549";
    ComponentArray expected = {1, 2, 840, 113549};

    ComponentArray result;
    Status status = stringToOID(oidStr, result);

    ASSERT_EQ(status, Status::Success);
    EXPECT_EQ(result, expected);
}

TEST_F(OIDTest, StringToOID_SHA256) {
    std::string oidStr = "2.16.840.1.101.3.4.2.1";
    ComponentArray expected = {2, 16, 840, 1, 101, 3, 4, 2, 1};

    ComponentArray result;
    Status status = stringToOID(oidStr, result);

    ASSERT_EQ(status, Status::Success);
    EXPECT_EQ(result, expected);
}

TEST_F(OIDTest, StringToOID_EmptyString) {
    std::string oidStr = "";

    ComponentArray result;
    Status status = stringToOID(oidStr, result);

    EXPECT_EQ(status, Status::Success);
    EXPECT_TRUE(result.empty());
}

TEST_F(OIDTest, StringToOID_InvalidCharacters) {
    std::string oidStr = "1.2.abc.4";

    ComponentArray result;
    EXPECT_THROW(stringToOID(oidStr, result), std::invalid_argument);
}

TEST_F(OIDTest, StringToOID_NegativeNumber) {
    std::string oidStr = "1.2.-5.4";

    ComponentArray result;
    EXPECT_THROW(stringToOID(oidStr, result), std::invalid_argument);
}

TEST_F(OIDTest, StringToOID_LeadingTrailingDots) {
    // Leading/trailing dots should be handled gracefully
    std::string oidStr = ".1.2.3.";
    ComponentArray expected = {1, 2, 3};

    ComponentArray result;
    Status status = stringToOID(oidStr, result);

    EXPECT_EQ(status, Status::Success);
    EXPECT_EQ(result, expected);
}

///////////////////////////////////////////////////////////////////////////
// Test: Round Trip with String Conversion
///////////////////////////////////////////////////////////////////////////

TEST_F(OIDTest, RoundTripWithString_SHA256) {
    // Complete round trip: string -> components -> OID -> components -> string
    std::string original = "2.16.840.1.101.3.4.2.1";

    // String to components
    ComponentArray components1 = stringToOID(original);

    // Components to OID
    ObjectIdentifier oid;
    Status status = toObjectIdentifier(components1, oid);
    ASSERT_EQ(status, Status::Success);

    // OID back to components
    ComponentArray components2 = toSeparates(oid);

    // Components to string
    std::string result = oidToString(components2);

    EXPECT_EQ(result, original);
}

TEST_F(OIDTest, RoundTripWithString_RSA) {
    std::string original = "1.2.840.113549.1.1.1";

    ComponentArray components1 = stringToOID(original);
    ObjectIdentifier oid;
    ASSERT_EQ(toObjectIdentifier(components1, oid), Status::Success);
    ComponentArray components2 = toSeparates(oid);
    std::string result = oidToString(components2);

    EXPECT_EQ(result, original);
}

///////////////////////////////////////////////////////////////////////////
// Test: Edge Cases
///////////////////////////////////////////////////////////////////////////

TEST_F(OIDTest, EdgeCase_TooManyComponents) {
    // More than MaximumComponentCount should fail
    ComponentArray components = {2, 16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    ASSERT_GT(components.size(), MaximumComponentCount);

    ObjectIdentifier oid;
    Status status = toObjectIdentifier(components, oid);
    EXPECT_EQ(status, Status::InsufficientSpace);
}

TEST_F(OIDTest, EdgeCase_LargeComponentValues) {
    // Test with large component values (within uint32_t range)
    ComponentArray components = {2, 16, 0xFFFFFFFF, 0x12345678, 100};

    ObjectIdentifier oid;
    Status status = toObjectIdentifier(components, oid);
    ASSERT_EQ(status, Status::Success);

    ComponentArray result = toSeparates(oid);
    EXPECT_EQ(result, components);
}

TEST_F(OIDTest, EdgeCase_BoundaryValues) {
    // Test boundary values for second component based on root

    // Root 0, second = 0 (minimum)
    ComponentArray comp1 = {0, 0};
    ObjectIdentifier oid1;
    EXPECT_EQ(toObjectIdentifier(comp1, oid1), Status::Success);

    // Root 0, second = 39 (maximum)
    ComponentArray comp2 = {0, 39};
    ObjectIdentifier oid2;
    EXPECT_EQ(toObjectIdentifier(comp2, oid2), Status::Success);

    // Root 1, second = 0 (minimum)
    ComponentArray comp3 = {1, 0};
    ObjectIdentifier oid3;
    EXPECT_EQ(toObjectIdentifier(comp3, oid3), Status::Success);

    // Root 1, second = 39 (maximum)
    ComponentArray comp4 = {1, 39};
    ObjectIdentifier oid4;
    EXPECT_EQ(toObjectIdentifier(comp4, oid4), Status::Success);

    // Root 2, second = 0 (minimum)
    ComponentArray comp5 = {2, 0};
    ObjectIdentifier oid5;
    EXPECT_EQ(toObjectIdentifier(comp5, oid5), Status::Success);

    // Root 2, second = 175 (maximum)
    ComponentArray comp6 = {2, 175};
    ObjectIdentifier oid6;
    EXPECT_EQ(toObjectIdentifier(comp6, oid6), Status::Success);
}

///////////////////////////////////////////////////////////////////////////
// Test: Well-Known OIDs
///////////////////////////////////////////////////////////////////////////

TEST_F(OIDTest, WellKnownOID_SHA256) {
    // SHA-256: 2.16.840.1.101.3.4.2.1
    std::string sha256_oid = "2.16.840.1.101.3.4.2.1";
    ComponentArray components = stringToOID(sha256_oid);

    ObjectIdentifier oid;
    Status status = toObjectIdentifier(components, oid);
    ASSERT_EQ(status, Status::Success);

    EXPECT_EQ(oid.getRootComponent(), 2);
    EXPECT_EQ(oid.getSecondLevelComponent(), 16);
    EXPECT_EQ(oid.componentCount(), 9);
}

TEST_F(OIDTest, WellKnownOID_RSA_Encryption) {
    // RSA Encryption: 1.2.840.113549.1.1.1
    std::string rsa_oid = "1.2.840.113549.1.1.1";
    ComponentArray components = stringToOID(rsa_oid);

    ObjectIdentifier oid;
    Status status = toObjectIdentifier(components, oid);
    ASSERT_EQ(status, Status::Success);

    EXPECT_EQ(oid.getRootComponent(), 1);
    EXPECT_EQ(oid.getSecondLevelComponent(), 2);
    EXPECT_EQ(oid.componentCount(), 7);
}

// Main function
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
