#include <gtest/gtest.h>
// NOLINTBEGIN(readability-identifier-naming)
// Test local constants use snake_case for readability
// NOLINTBEGIN(modernize-use-designated-initializers,cppcoreguidelines-pro-bounds-constant-array-index)

#include <array>
#include <vector>

#include "thumper/hermes/der_encode.hpp"
#include "thumper/types.hpp"

using namespace thumper;
using namespace thumper::hermes::der;

// Test fixture for DER encoding tests
class DEREncodeTest : public ::testing::Test {
protected:
  // Helper function to compare OctetArray with expected values
  static void ExpectOctetArrayEqual(const OctetArray& actual, const std::vector<Octet>& expected,
                                    const std::string& test_name) {
    EXPECT_EQ(actual.size(), expected.size()) << test_name << ": length mismatch";
    EXPECT_EQ(actual, expected) << test_name << ": value mismatch";
  }
};

// Test Make_Leading_Identifier
TEST_F(DEREncodeTest, MakeLeadingIdentifier) {
  // Test Universal, Primitive, Boolean (0x01)
  Octet result = make_leading_identifier(TagClass::Universal, StructuredFlag::Primitive,
                                         LeadingNumberType::TagBoolean);
  EXPECT_EQ(result, 0x01);

  // Test Universal, Primitive, Integer (0x02)
  result = make_leading_identifier(TagClass::Universal, StructuredFlag::Primitive,
                                   LeadingNumberType::TagInteger);
  EXPECT_EQ(result, 0x02);

  // Test Universal, Constructed, Sequence (0x30)
  result = make_leading_identifier(TagClass::Universal, StructuredFlag::Constructed,
                                   LeadingNumberType::TagSequence);
  EXPECT_EQ(result, 0x30);

  // Test Application, Primitive, Boolean (0x41)
  result = make_leading_identifier(TagClass::Application, StructuredFlag::Primitive,
                                   LeadingNumberType::TagBoolean);
  EXPECT_EQ(result, 0x41);

  // Test ContextSpecific, Constructed, Sequence (0xB0)
  result = make_leading_identifier(TagClass::ContextSpecific, StructuredFlag::Constructed,
                                   LeadingNumberType::TagSequence);
  EXPECT_EQ(result, 0xB0);
}

// Test Put_Length_Value with various length values
// Corresponds to Test_Put_Length in check_der_encode.adb
TEST_F(DEREncodeTest, PutLengthValue) {
  struct TestCase {
    std::size_t input;
    std::vector<Octet> expected;
  };

  const std::array<TestCase, 10> test_cases = {{
      {0, {0x00}},
      {1, {0x01}},
      {127, {0x7F}},
      {128, {0x81, 0x80}},
      {255, {0x81, 0xFF}},
      {256, {0x82, 0x01, 0x00}},
      {0xFFFF, {0x82, 0xFF, 0xFF}},
      {0x10000, {0x83, 0x01, 0x00, 0x00}},
      {0xFF0000, {0x83, 0xFF, 0x00, 0x00}},
      {0x7FFFFFFF, {0x84, 0x7F, 0xFF, 0xFF, 0xFF}},
  }};

  for (std::size_t i = 0; i < test_cases.size(); ++i) {
    OctetArray result = put_length_value(test_cases[i].input);
    ExpectOctetArrayEqual(result, test_cases[i].expected, "Test case #" + std::to_string(i + 1));
  }
}

// Test Put_Boolean_Value
// Corresponds to Test_Put_Boolean in check_der_encode.adb
TEST_F(DEREncodeTest, PutBooleanValue) {
  const std::vector<Octet> boolean_false = {0x01, 0x01, 0x00};
  const std::vector<Octet> boolean_true = {0x01, 0x01, 0xFF};

  ExpectOctetArrayEqual(put_boolean_value(false), boolean_false, "False case");
  ExpectOctetArrayEqual(put_boolean_value(true), boolean_true, "True case");
}

// Test Put_Integer_Value
// Corresponds to Test_Put_Integer in check_der_encode.adb
TEST_F(DEREncodeTest, PutIntegerValue) {
  struct TestCase {
    int input;
    std::vector<Octet> expected;
  };

  const std::array<TestCase, 10> test_cases = {{
      {0, {0x02, 0x01, 0x00}},
      {1, {0x02, 0x01, 0x01}},
      {127, {0x02, 0x01, 0x7F}},
      {128, {0x02, 0x02, 0x00, 0x80}},
      {255, {0x02, 0x02, 0x00, 0xFF}},
      {256, {0x02, 0x02, 0x01, 0x00}},
      {0xFFFF, {0x02, 0x03, 0x00, 0xFF, 0xFF}},
      {0x10000, {0x02, 0x03, 0x01, 0x00, 0x00}},
      {0xFF0000, {0x02, 0x04, 0x00, 0xFF, 0x00, 0x00}},
      {0x7FFFFFFF, {0x02, 0x04, 0x7F, 0xFF, 0xFF, 0xFF}},
  }};

  for (std::size_t i = 0; i < test_cases.size(); ++i) {
    OctetArray result = put_integer_value(test_cases[i].input);
    ExpectOctetArrayEqual(result, test_cases[i].expected, "Test case #" + std::to_string(i + 1));
  }
}

// Test Put_Integer_Value with negative values
TEST_F(DEREncodeTest, PutIntegerValueNegative) {
  struct TestCase {
    int input;
    std::vector<Octet> expected;
  };

  const std::array<TestCase, 5> test_cases = {{
      {-1, {0x02, 0x01, 0xFF}},
      {-128, {0x02, 0x01, 0x80}},
      {-129, {0x02, 0x02, 0xFF, 0x7F}},
      {-32768, {0x02, 0x02, 0x80, 0x00}},
      {-32769, {0x02, 0x03, 0xFF, 0x7F, 0xFF}},
  }};

  for (std::size_t i = 0; i < test_cases.size(); ++i) {
    OctetArray result = put_integer_value(test_cases[i].input);
    ExpectOctetArrayEqual(result, test_cases[i].expected,
                          "Negative test case #" + std::to_string(i + 1));
  }
}

// Test Put_Octet_String_Value
TEST_F(DEREncodeTest, PutOctetStringValue) {
  // Empty octet string
  {
    const std::vector<Octet> empty;
    OctetArray result = put_octet_string_value(empty);
    const std::vector<Octet> expected = {0x04, 0x00};
    ExpectOctetArrayEqual(result, expected, "Empty octet string");
  }

  // Short octet string
  {
    const std::vector<Octet> short_string = {0x01, 0x02, 0x03};
    OctetArray result = put_octet_string_value(short_string);
    const std::vector<Octet> expected = {0x04, 0x03, 0x01, 0x02, 0x03};
    ExpectOctetArrayEqual(result, expected, "Short octet string");
  }

  // Longer octet string (tests long-form length encoding)
  {
    std::vector<Octet> long_string(200);
    for (std::size_t i = 0; i < long_string.size(); ++i) {
      long_string[i] = static_cast<Octet>(i % 256);
    }
    OctetArray result = put_octet_string_value(long_string);

    // Expected: 0x04 (tag) + 0x81 0xC8 (length = 200) + 200 bytes of data
    EXPECT_EQ(result.size(), 203);
    EXPECT_EQ(result[0], 0x04);
    EXPECT_EQ(result[1], 0x81);
    EXPECT_EQ(result[2], 0xC8);
    for (std::size_t i = 0; i < long_string.size(); ++i) {
      EXPECT_EQ(result[3 + i], long_string[i]);
    }
  }
}

// Test Put_Null_Value
TEST_F(DEREncodeTest, PutNullValue) {
  const std::vector<Octet> expected = {0x05, 0x00};
  ExpectOctetArrayEqual(put_null_value(), expected, "Null value");
}

// Note: Test_Put_OID is intentionally not migrated here, as OID encoding
// will be co-migrated with the OID type in Task 4 per the task specification.
// NOLINTEND(modernize-use-designated-initializers,cppcoreguidelines-pro-bounds-constant-array-index)
// NOLINTEND(readability-identifier-naming)
