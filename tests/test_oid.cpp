#include <vector>

#include <gtest/gtest.h>

#include "thumper/hermes/der_decode.hpp"
#include "thumper/hermes/der_encode.hpp"
#include "thumper/hermes/oid.hpp"
#include "thumper/types.hpp"

using namespace thumper;
using namespace thumper::hermes::oid;
namespace der = thumper::hermes::der;

// Test fixture for OID tests
class OIDTest : public ::testing::Test {};

// Test basic OID creation from components
TEST_F(OIDTest, CreateValidOID) {
  std::vector<ComponentType> components = {2, 5, 4, 3};
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(components, oid);

  EXPECT_EQ(status, Status::Success);
  EXPECT_TRUE(oid.is_valid());
  EXPECT_EQ(oid.component_count(), 4);
}

// Test OID with root 0
TEST_F(OIDTest, CreateOIDWithRoot0) {
  std::vector<ComponentType> components = {0, 39};
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(components, oid);

  EXPECT_EQ(status, Status::Success);
  EXPECT_TRUE(oid.is_valid());
  EXPECT_EQ(oid.component_count(), 2);
}

// Test OID with root 1
TEST_F(OIDTest, CreateOIDWithRoot1) {
  std::vector<ComponentType> components = {1, 2, 840, 113549};
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(components, oid);

  EXPECT_EQ(status, Status::Success);
  EXPECT_TRUE(oid.is_valid());
  EXPECT_EQ(oid.component_count(), 4);
}

// Test OID with root 2
TEST_F(OIDTest, CreateOIDWithRoot2) {
  std::vector<ComponentType> components = {2, 16, 840, 1, 101, 3, 4, 2, 1}; // SHA-256
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(components, oid);

  EXPECT_EQ(status, Status::Success);
  EXPECT_TRUE(oid.is_valid());
  EXPECT_EQ(oid.component_count(), 9);
}

// Test invalid root component
TEST_F(OIDTest, InvalidRootComponent) {
  std::vector<ComponentType> components = {3, 5, 4, 3};
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(components, oid);

  EXPECT_EQ(status, Status::InvalidRoot);
  EXPECT_FALSE(oid.is_valid());
}

// Test invalid second level for root 0
TEST_F(OIDTest, InvalidSecondLevelForRoot0) {
  std::vector<ComponentType> components = {0, 40};
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(components, oid);

  EXPECT_EQ(status, Status::InvalidSecondLevel);
  EXPECT_FALSE(oid.is_valid());
}

// Test invalid second level for root 1
TEST_F(OIDTest, InvalidSecondLevelForRoot1) {
  std::vector<ComponentType> components = {1, 40};
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(components, oid);

  EXPECT_EQ(status, Status::InvalidSecondLevel);
  EXPECT_FALSE(oid.is_valid());
}

// Test invalid second level for root 2
TEST_F(OIDTest, InvalidSecondLevelForRoot2) {
  std::vector<ComponentType> components = {2, 176};
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(components, oid);

  EXPECT_EQ(status, Status::InvalidSecondLevel);
  EXPECT_FALSE(oid.is_valid());
}

// Test too few components
TEST_F(OIDTest, TooFewComponents) {
  std::vector<ComponentType> components = {2};
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(components, oid);

  EXPECT_EQ(status, Status::InvalidRoot);
  EXPECT_FALSE(oid.is_valid());
}

// Test too many components
TEST_F(OIDTest, TooManyComponents) {
  std::vector<ComponentType> components = {2, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(components, oid);

  EXPECT_EQ(status, Status::InsufficientSpace);
  EXPECT_FALSE(oid.is_valid());
}

// Test round-trip: components -> OID -> components
TEST_F(OIDTest, RoundTripComponentConversion) {
  std::vector<ComponentType> original = {2, 5, 4, 3};
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(original, oid);
  ASSERT_EQ(status, Status::Success);

  std::array<ComponentType, MAX_COMPONENT_COUNT> result{};
  std::size_t count = oid.to_components(result);

  ASSERT_EQ(count, original.size());
  for (std::size_t i = 0; i < count; ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    EXPECT_EQ(result[i], original[i]);
  }
}

// Test OID to string conversion
TEST_F(OIDTest, OIDToString) {
  std::vector<ComponentType> components = {2, 5, 4, 3};
  std::string oid_str = oid_to_string(components);

  EXPECT_EQ(oid_str, "2.5.4.3");
}

// Test OID to string with large components
TEST_F(OIDTest, OIDToStringLargeComponents) {
  std::vector<ComponentType> components = {2, 16, 840, 1, 101, 3, 4, 2, 1};
  std::string oid_str = oid_to_string(components);

  EXPECT_EQ(oid_str, "2.16.840.1.101.3.4.2.1");
}

// Test string to OID conversion
TEST_F(OIDTest, StringToOID) {
  std::string oid_str = "2.5.4.3";
  std::vector<ComponentType> components;

  string_to_oid(oid_str, components);

  ASSERT_EQ(components.size(), 4);
  EXPECT_EQ(components[0], 2);
  EXPECT_EQ(components[1], 5);
  EXPECT_EQ(components[2], 4);
  EXPECT_EQ(components[3], 3);
}

// Test string to OID with large components
TEST_F(OIDTest, StringToOIDLargeComponents) {
  std::string oid_str = "2.16.840.1.101.3.4.2.1";
  std::vector<ComponentType> components;

  string_to_oid(oid_str, components);

  ASSERT_EQ(components.size(), 9);
  EXPECT_EQ(components[0], 2);
  EXPECT_EQ(components[1], 16);
  EXPECT_EQ(components[2], 840);
  EXPECT_EQ(components[3], 1);
  EXPECT_EQ(components[4], 101);
  EXPECT_EQ(components[5], 3);
  EXPECT_EQ(components[6], 4);
  EXPECT_EQ(components[7], 2);
  EXPECT_EQ(components[8], 1);
}

// Test round-trip: string -> OID -> string
TEST_F(OIDTest, RoundTripStringConversion) {
  std::string original = "2.5.4.3";
  std::vector<ComponentType> components;

  string_to_oid(original, components);
  std::string result = oid_to_string(components);

  EXPECT_EQ(result, original);
}

// Test DER encoding of OID
TEST_F(OIDTest, DEREncoding) {
  std::vector<ComponentType> components = {2, 5, 4, 3};
  ObjectIdentifier oid;
  auto status = ObjectIdentifier::from_components(components, oid);
  ASSERT_EQ(status, Status::Success);

  OctetArray encoded = der::put_oid_value(oid);

  // Expected encoding:
  // Tag: 0x06 (Universal, Primitive, ObjectIdentifier)
  // Length: 0x03 (3 octets)
  // Value: 0x55 (2*40+5=85), 0x04, 0x03
  ASSERT_EQ(encoded.size(), 5);
  EXPECT_EQ(encoded[0], 0x06); // Tag
  EXPECT_EQ(encoded[1], 0x03); // Length
  EXPECT_EQ(encoded[2], 0x55); // First two components: 2*40+5 = 85
  EXPECT_EQ(encoded[3], 0x04); // Third component
  EXPECT_EQ(encoded[4], 0x03); // Fourth component
}

// Test DER encoding of SHA-256 OID
TEST_F(OIDTest, DEREncodingSHA256) {
  std::vector<ComponentType> components = {2, 16, 840, 1, 101, 3, 4, 2, 1};
  ObjectIdentifier oid;
  auto status = ObjectIdentifier::from_components(components, oid);
  ASSERT_EQ(status, Status::Success);

  OctetArray encoded = der::put_oid_value(oid);

  // Expected encoding:
  // Tag: 0x06
  // Length: 0x09
  // Value: 0x60 (2*40+16=96), then base-128 encoding of remaining components
  ASSERT_GT(encoded.size(), 2);
  EXPECT_EQ(encoded[0], 0x06); // Tag
  EXPECT_EQ(encoded[1], 0x09); // Length = 9
  EXPECT_EQ(encoded[2], 0x60); // First two components: 2*40+16 = 96
}

// Test DER decoding of OID
TEST_F(OIDTest, DERDecoding) {
  // Encoded OID for 2.5.4.3
  OctetArray encoded = {0x06, 0x03, 0x55, 0x04, 0x03};

  ObjectIdentifier oid;
  std::size_t stop = 0;
  der::Status status = der::Status::Success;

  der::get_oid_value(encoded, 0, stop, oid, status);

  EXPECT_EQ(status, der::Status::Success);
  EXPECT_TRUE(oid.is_valid());
  EXPECT_EQ(oid.component_count(), 4);
  EXPECT_EQ(stop, 4);

  auto components = oid.components();
  EXPECT_EQ(components[0], 2);
  EXPECT_EQ(components[1], 5);
  EXPECT_EQ(components[2], 4);
  EXPECT_EQ(components[3], 3);
}

// Test DER decoding of SHA-256 OID
TEST_F(OIDTest, DERDecodingSHA256) {
  // Encoded OID for 2.16.840.1.101.3.4.2.1 (SHA-256)
  // 2*40+16=96=0x60, 840=0x86 0x48, 1=0x01, 101=0x65, 3=0x03, 4=0x04, 2=0x02, 1=0x01
  OctetArray encoded = {0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01};

  ObjectIdentifier oid;
  std::size_t stop = 0;
  der::Status status = der::Status::Success;

  der::get_oid_value(encoded, 0, stop, oid, status);

  EXPECT_EQ(status, der::Status::Success);
  EXPECT_TRUE(oid.is_valid());
  EXPECT_EQ(oid.component_count(), 9);

  auto components = oid.components();
  EXPECT_EQ(components[0], 2);
  EXPECT_EQ(components[1], 16);
  EXPECT_EQ(components[2], 840);
  EXPECT_EQ(components[3], 1);
  EXPECT_EQ(components[4], 101);
  EXPECT_EQ(components[5], 3);
  EXPECT_EQ(components[6], 4);
  EXPECT_EQ(components[7], 2);
  EXPECT_EQ(components[8], 1);
}

// Test round-trip DER encoding/decoding
TEST_F(OIDTest, RoundTripDEREncodingDecoding) {
  std::vector<ComponentType> original = {2, 5, 4, 3};
  ObjectIdentifier original_oid;
  auto status = ObjectIdentifier::from_components(original, original_oid);
  ASSERT_EQ(status, Status::Success);

  // Encode
  OctetArray encoded = der::put_oid_value(original_oid);

  // Decode
  ObjectIdentifier decoded_oid;
  std::size_t stop = 0;
  der::Status decode_status = der::Status::Success;
  der::get_oid_value(encoded, 0, stop, decoded_oid, decode_status);

  EXPECT_EQ(decode_status, der::Status::Success);
  EXPECT_TRUE(decoded_oid.is_valid());
  EXPECT_EQ(original_oid, decoded_oid);
}

// Test OID with large component requiring multi-byte base-128 encoding
TEST_F(OIDTest, LargeComponentEncoding) {
  // Component 16384 requires 3 bytes in base-128: 0x81 0x80 0x00
  std::vector<ComponentType> components = {2, 5, 16384};
  ObjectIdentifier oid;
  auto status = ObjectIdentifier::from_components(components, oid);
  ASSERT_EQ(status, Status::Success);

  OctetArray encoded = der::put_oid_value(oid);

  // Expected: Tag=0x06, Length=0x04, Value=0x55 (2*40+5), then base-128(16384)
  // 16384 = 0x4000 = 0b100000000000000
  // In base-128: 0b1000001 0b0000000 -> 0x81 0x80 0x00
  ASSERT_GE(encoded.size(), 2);
  EXPECT_EQ(encoded[0], 0x06); // Tag

  // Decode and verify
  ObjectIdentifier decoded_oid;
  std::size_t stop = 0;
  der::Status decode_status = der::Status::Success;
  der::get_oid_value(encoded, 0, stop, decoded_oid, decode_status);

  EXPECT_EQ(decode_status, der::Status::Success);
  EXPECT_EQ(oid, decoded_oid);
}

// Test OID equality
TEST_F(OIDTest, Equality) {
  std::vector<ComponentType> components1 = {2, 5, 4, 3};
  std::vector<ComponentType> components2 = {2, 5, 4, 3};
  std::vector<ComponentType> components3 = {2, 5, 4, 4};

  ObjectIdentifier oid1;
  ObjectIdentifier oid2;
  ObjectIdentifier oid3;
  ObjectIdentifier::from_components(components1, oid1);
  ObjectIdentifier::from_components(components2, oid2);
  ObjectIdentifier::from_components(components3, oid3);

  EXPECT_EQ(oid1, oid2);
  EXPECT_NE(oid1, oid3);
}

// Test empty OID
TEST_F(OIDTest, EmptyOID) {
  std::vector<ComponentType> components;
  ObjectIdentifier oid;

  auto status = ObjectIdentifier::from_components(components, oid);

  EXPECT_EQ(status, Status::InvalidRoot);
  EXPECT_FALSE(oid.is_valid());
  EXPECT_EQ(oid.component_count(), 0);
}

// Test validate_oid function
TEST_F(OIDTest, ValidateOID) {
  std::vector<ComponentType> valid_components = {2, 5, 4, 3};
  std::vector<ComponentType> invalid_components = {3, 5, 4, 3};

  EXPECT_EQ(validate_oid(valid_components), Status::Success);
  EXPECT_EQ(validate_oid(invalid_components), Status::InvalidRoot);
}
