#include <gtest/gtest.h>
// NOLINTBEGIN(readability-identifier-naming)
// Test local constants use snake_case for readability
// NOLINTBEGIN(modernize-use-designated-initializers,cppcoreguidelines-pro-bounds-constant-array-index,cppcoreguidelines-init-variables)

#include <array>
#include <vector>

#include "thumper/hermes/der_decode.hpp"
#include "thumper/hermes/der_encode.hpp"
#include "thumper/types.hpp"

using namespace thumper;
using namespace thumper::hermes::der;

// Test fixture for DER decoding tests
class DERDecodeTest : public ::testing::Test {
protected:
  // Helper function to run a test case and compare outputs
  template <typename T>
  static void
  RunTestCase(const std::vector<Octet>& data, std::size_t start, std::size_t expected_stop,
              const T& expected_value, Status expected_status, const std::string& test_name,
              void (*decode_fn)(std::span<const Octet>, std::size_t, std::size_t&, T&, Status&)) {
    std::size_t stop;
    T value;
    Status status;

    decode_fn(data, start, stop, value, status);

    EXPECT_EQ(stop, expected_stop) << test_name << ": stop mismatch";
    EXPECT_EQ(value, expected_value) << test_name << ": value mismatch";
    EXPECT_EQ(status, expected_status) << test_name << ": status mismatch";
  }
};

// Test Split_Leading_Identifier
TEST_F(DERDecodeTest, SplitLeadingIdentifier) {
  TagClass tag_class;
  StructuredFlag structured_flag;
  LeadingNumberType tag;
  Status status;

  // Test Universal, Primitive, Boolean (0x01)
  split_leading_identifier(0x01, tag_class, structured_flag, tag, status);
  EXPECT_EQ(tag_class, TagClass::Universal);
  EXPECT_EQ(structured_flag, StructuredFlag::Primitive);
  EXPECT_EQ(tag, LeadingNumberType::TagBoolean);
  EXPECT_EQ(status, Status::Success);

  // Test Universal, Primitive, Integer (0x02)
  split_leading_identifier(0x02, tag_class, structured_flag, tag, status);
  EXPECT_EQ(tag_class, TagClass::Universal);
  EXPECT_EQ(structured_flag, StructuredFlag::Primitive);
  EXPECT_EQ(tag, LeadingNumberType::TagInteger);
  EXPECT_EQ(status, Status::Success);

  // Test Universal, Constructed, Sequence (0x30)
  split_leading_identifier(0x30, tag_class, structured_flag, tag, status);
  EXPECT_EQ(tag_class, TagClass::Universal);
  EXPECT_EQ(structured_flag, StructuredFlag::Constructed);
  EXPECT_EQ(tag, LeadingNumberType::TagSequence);
  EXPECT_EQ(status, Status::Success);

  // Test Application, Primitive, Boolean (0x41)
  split_leading_identifier(0x41, tag_class, structured_flag, tag, status);
  EXPECT_EQ(tag_class, TagClass::Application);
  EXPECT_EQ(structured_flag, StructuredFlag::Primitive);
  EXPECT_EQ(tag, LeadingNumberType::TagBoolean);
  EXPECT_EQ(status, Status::Success);

  // Test ContextSpecific, Constructed, Sequence (0xB0)
  split_leading_identifier(0xB0, tag_class, structured_flag, tag, status);
  EXPECT_EQ(tag_class, TagClass::ContextSpecific);
  EXPECT_EQ(structured_flag, StructuredFlag::Constructed);
  EXPECT_EQ(tag, LeadingNumberType::TagSequence);
  EXPECT_EQ(status, Status::Success);

  // Test invalid tag number (14)
  split_leading_identifier(0x0E, tag_class, structured_flag, tag, status);
  EXPECT_EQ(status, Status::BadIdentifier);

  // Test invalid tag number (15)
  split_leading_identifier(0x0F, tag_class, structured_flag, tag, status);
  EXPECT_EQ(status, Status::BadIdentifier);
}

// Test Get_Length_Value with various length encodings
// Corresponds to Test_Get_Length in check_der_decode.adb
TEST_F(DERDecodeTest, GetLengthValue) {
  struct TestCase {
    std::vector<Octet> data;
    std::size_t start;
    std::size_t expected_stop;
    std::size_t expected_length;
    Status expected_status;
  };

  const std::array<TestCase, 16> test_cases = {{
      // Correctly formatted definite form using short encoding
      {{0x00}, 0, 0, 0, Status::Success},
      {{0x01}, 0, 0, 1, Status::Success},
      {{0x7F}, 0, 0, 127, Status::Success},

      // Indefinite length
      {{0x80}, 0, 0, 0, Status::IndefiniteLength},

      // Reserved encoding
      {{0xFF}, 0, 0, 0, Status::BadLength},

      // Correctly formatted definite form using long encoding
      {{0x81, 0x00}, 0, 1, 0, Status::Success},
      {{0x81, 0x01}, 0, 1, 1, Status::Success},
      {{0x81, 0xFF}, 0, 1, 255, Status::Success},
      {{0x82, 0x00, 0x01}, 0, 2, 1, Status::Success},
      {{0x82, 0x01, 0x00}, 0, 2, 256, Status::Success},
      {{0x82, 0xFF, 0xFF}, 0, 2, 65535, Status::Success},
      {{0x83, 0xFF, 0xFF, 0xFF}, 0, 3, 16777215, Status::Success},
      {{0x84, 0x7F, 0xFF, 0xFF, 0xFF}, 0, 4, 2147483647, Status::Success},

      // Unimplemented lengths
      {{0x84, 0x80, 0x00, 0x00, 0x00}, 0, 4, 0, Status::UnimplementedLength},
      {{0x85, 0x00, 0x00, 0x00, 0x00, 0x01}, 0, 5, 0, Status::UnimplementedLength},

      // Error case: not enough octets
      {{0x85, 0x00, 0x00, 0x00, 0x00}, 0, 4, 0, Status::BadLength},
  }};

  for (std::size_t i = 0; i < test_cases.size(); ++i) {
    std::size_t stop;
    std::size_t length;
    Status status;

    get_length_value(test_cases[i].data, test_cases[i].start, stop, length, status);

    EXPECT_EQ(stop, test_cases[i].expected_stop) << "Test case #" << (i + 1) << ": stop mismatch";
    EXPECT_EQ(length, test_cases[i].expected_length)
        << "Test case #" << (i + 1) << ": length mismatch";
    EXPECT_EQ(status, test_cases[i].expected_status)
        << "Test case #" << (i + 1) << ": status mismatch";
  }
}

// Test Get_Boolean_Value
// Corresponds to Test_Get_Boolean in check_der_decode.adb
TEST_F(DERDecodeTest, GetBooleanValue) {
  struct TestCase {
    std::vector<Octet> data;
    std::size_t start;
    std::size_t expected_stop;
    bool expected_value;
    Status expected_status;
  };

  const std::array<TestCase, 3> test_cases = {{
      // Correctly formatted Boolean encodings
      {{0x01, 0x01, 0x00}, 0, 2, false, Status::Success},
      {{0x01, 0x01, 0xFF}, 0, 2, true, Status::Success},

      // Invalid encoding (DER requires 0x00 or 0xFF only)
      {{0x01, 0x01, 0x01}, 0, 2, false, Status::BadValue},
  }};

  for (std::size_t i = 0; i < test_cases.size(); ++i) {
    std::size_t stop;
    bool value;
    Status status;

    get_boolean_value(test_cases[i].data, test_cases[i].start, stop, value, status);

    EXPECT_EQ(stop, test_cases[i].expected_stop) << "Test case #" << (i + 1) << ": stop mismatch";
    EXPECT_EQ(value, test_cases[i].expected_value)
        << "Test case #" << (i + 1) << ": value mismatch";
    EXPECT_EQ(status, test_cases[i].expected_status)
        << "Test case #" << (i + 1) << ": status mismatch";
  }
}

// Test Get_Integer_Value
// Corresponds to Test_Get_Integer in check_der_decode.adb
TEST_F(DERDecodeTest, GetIntegerValue) {
  struct TestCase {
    std::vector<Octet> data;
    std::size_t start;
    std::size_t expected_stop;
    int expected_value;
    Status expected_status;
  };

  const std::array<TestCase, 18> test_cases = {{
      // Correctly formatted integer encodings
      {{0x02, 0x01, 0x00}, 0, 2, 0, Status::Success},
      {{0x02, 0x01, 0x01}, 0, 2, 1, Status::Success},
      {{0x02, 0x01, 0x7F}, 0, 2, 127, Status::Success},
      {{0x02, 0x01, 0xFF}, 0, 2, -1, Status::Success},
      {{0x02, 0x02, 0x00, 0xFF}, 0, 3, 255, Status::Success},
      {{0x02, 0x02, 0x01, 0x00}, 0, 3, 256, Status::Success},
      {{0x02, 0x02, 0x01, 0xFF}, 0, 3, 511, Status::Success},
      {{0x02, 0x04, 0x7F, 0xFF, 0xFF, 0xFF}, 0, 5, 2147483647, Status::Success},
      {{0x02, 0x04, 0x80, 0x00, 0x00, 0x00}, 0, 5, -2147483648, Status::Success},

      // Invalid encodings
      {{0x22, 0x01, 0x01}, 0, 0, 0, Status::BadValue},                   // Wrong tag (constructed)
      {{0x42, 0x01, 0x01}, 0, 0, 0, Status::BadValue},                   // Wrong tag class
      {{0x1F, 0x01, 0x01}, 0, 0, 0, Status::BadValue},                   // Extended tag
      {{0x02, 0x02, 0x01}, 0, 2, 0, Status::BadValue},                   // Value goes off end
      {{0x02, 0x02, 0x00, 0x01}, 0, 3, 0, Status::BadValue},             // Unnecessary leading zero
      {{0x02, 0x02, 0xFF, 0x80}, 0, 3, 0, Status::BadValue},             // Unnecessary leading ones
      {{0x02, 0x04, 0xFF, 0xFF, 0xFF, 0xFF}, 0, 5, 0, Status::BadValue}, // All ones (would be -1
                                                                         // but uses 4 bytes)

      // Unimplemented values (more than 4 bytes)
      {{0x02, 0x05, 0x00, 0x80, 0x00, 0x00, 0x00}, 0, 6, 0, Status::UnimplementedValue},
      {{0x02, 0x05, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF}, 0, 6, 0, Status::UnimplementedValue},
  }};

  for (std::size_t i = 0; i < test_cases.size(); ++i) {
    std::size_t stop;
    int value;
    Status status;

    get_integer_value(test_cases[i].data, test_cases[i].start, stop, value, status);

    EXPECT_EQ(stop, test_cases[i].expected_stop) << "Test case #" << (i + 1) << ": stop mismatch";
    EXPECT_EQ(value, test_cases[i].expected_value)
        << "Test case #" << (i + 1) << ": value mismatch";
    EXPECT_EQ(status, test_cases[i].expected_status)
        << "Test case #" << (i + 1) << ": status mismatch";
  }
}

// Test round-trip encoding/decoding for Boolean values
TEST_F(DERDecodeTest, BooleanRoundTrip) {
  // Test false
  {
    OctetArray encoded = put_boolean_value(false);
    std::size_t stop;
    bool value;
    Status status;

    get_boolean_value(encoded, 0, stop, value, status);

    EXPECT_EQ(status, Status::Success);
    EXPECT_EQ(value, false);
    EXPECT_EQ(stop, encoded.size() - 1);
  }

  // Test true
  {
    OctetArray encoded = put_boolean_value(true);
    std::size_t stop;
    bool value;
    Status status;

    get_boolean_value(encoded, 0, stop, value, status);

    EXPECT_EQ(status, Status::Success);
    EXPECT_EQ(value, true);
    EXPECT_EQ(stop, encoded.size() - 1);
  }
}

// Test round-trip encoding/decoding for integer values
TEST_F(DERDecodeTest, IntegerRoundTrip) {
  const std::array<int, 15> test_values = {
      {0, 1, 127, 128, 255, 256, 32767, 32768, 65535, -1, -127, -128, -129, -32768, -32769}};

  for (int test_value : test_values) {
    OctetArray encoded = put_integer_value(test_value);
    std::size_t stop;
    int value;
    Status status;

    get_integer_value(encoded, 0, stop, value, status);

    EXPECT_EQ(status, Status::Success) << "Failed for value " << test_value;
    EXPECT_EQ(value, test_value) << "Failed for value " << test_value;
    EXPECT_EQ(stop, encoded.size() - 1) << "Failed for value " << test_value;
  }
}
// NOLINTEND(modernize-use-designated-initializers,cppcoreguidelines-pro-bounds-constant-array-index,cppcoreguidelines-init-variables)
// NOLINTEND(readability-identifier-naming)
