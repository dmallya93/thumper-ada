#include <stdexcept>

#include <gtest/gtest.h>

#include "thumper/hermes/hermes.hpp"

using namespace thumper;
using namespace thumper::hermes;

// Test fixture for Hermes core functionality
class HermesCoreTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Common setup if needed
  }

  void TearDown() override {
    // Common cleanup if needed
  }
};

// ============================================================================
// Tests for is_hex_digit
// ============================================================================

TEST_F(HermesCoreTest, IsHexDigit_ValidDigits) {
  // Test decimal digits
  for (char ch = '0'; ch <= '9'; ++ch) {
    EXPECT_TRUE(is_hex_digit(ch)) << "Digit '" << ch << "' should be valid";
  }

  // Test uppercase hex letters
  for (char ch = 'A'; ch <= 'F'; ++ch) {
    EXPECT_TRUE(is_hex_digit(ch)) << "Letter '" << ch << "' should be valid";
  }
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
TEST_F(HermesCoreTest, IsHexDigit_InvalidCharacters) {
  // Test lowercase letters (not allowed per spec)
  for (char ch = 'a'; ch <= 'f'; ++ch) {
    EXPECT_FALSE(is_hex_digit(ch)) << "Lowercase letter '" << ch << "' should be invalid";
  }

  // Test other characters
  EXPECT_FALSE(is_hex_digit(' '));
  EXPECT_FALSE(is_hex_digit('G'));
  EXPECT_FALSE(is_hex_digit('Z'));
  EXPECT_FALSE(is_hex_digit('/'));
  EXPECT_FALSE(is_hex_digit(':'));
  EXPECT_FALSE(is_hex_digit('@'));
  EXPECT_FALSE(is_hex_digit('['));
}
// NOLINTEND(readability-function-cognitive-complexity)

// ============================================================================
// Tests for hex_digit_to_value
// ============================================================================

TEST_F(HermesCoreTest, HexDigitToValue_DecimalDigits) {
  EXPECT_EQ(hex_digit_to_value('0'), 0);
  EXPECT_EQ(hex_digit_to_value('1'), 1);
  EXPECT_EQ(hex_digit_to_value('2'), 2);
  EXPECT_EQ(hex_digit_to_value('3'), 3);
  EXPECT_EQ(hex_digit_to_value('4'), 4);
  EXPECT_EQ(hex_digit_to_value('5'), 5);
  EXPECT_EQ(hex_digit_to_value('6'), 6);
  EXPECT_EQ(hex_digit_to_value('7'), 7);
  EXPECT_EQ(hex_digit_to_value('8'), 8);
  EXPECT_EQ(hex_digit_to_value('9'), 9);
}

TEST_F(HermesCoreTest, HexDigitToValue_UppercaseLetters) {
  EXPECT_EQ(hex_digit_to_value('A'), 10);
  EXPECT_EQ(hex_digit_to_value('B'), 11);
  EXPECT_EQ(hex_digit_to_value('C'), 12);
  EXPECT_EQ(hex_digit_to_value('D'), 13);
  EXPECT_EQ(hex_digit_to_value('E'), 14);
  EXPECT_EQ(hex_digit_to_value('F'), 15);
}

// ============================================================================
// Tests for octets_to_string
// ============================================================================

TEST_F(HermesCoreTest, OctetsToString_EmptyArray) {
  OctetArray empty;
  std::string result = octets_to_string(empty);
  EXPECT_EQ(result, "");
}

TEST_F(HermesCoreTest, OctetsToString_SingleOctet) {
  OctetArray octets = {0x42};
  std::string result = octets_to_string(octets);
  EXPECT_EQ(result, "42");
}

TEST_F(HermesCoreTest, OctetsToString_MultipleOctets) {
  OctetArray octets = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello" in ASCII
  std::string result = octets_to_string(octets);
  EXPECT_EQ(result, "48 65 6C 6C 6F");
}

TEST_F(HermesCoreTest, OctetsToString_AllZeros) {
  OctetArray octets = {0x00, 0x00, 0x00};
  std::string result = octets_to_string(octets);
  EXPECT_EQ(result, "00 00 00");
}

TEST_F(HermesCoreTest, OctetsToString_AllOnes) {
  OctetArray octets = {0xFF, 0xFF, 0xFF};
  std::string result = octets_to_string(octets);
  EXPECT_EQ(result, "FF FF FF");
}

TEST_F(HermesCoreTest, OctetsToString_MixedValues) {
  OctetArray octets = {0x00, 0x0F, 0xF0, 0xFF, 0xAB, 0xCD, 0xEF};
  std::string result = octets_to_string(octets);
  EXPECT_EQ(result, "00 0F F0 FF AB CD EF");
}

TEST_F(HermesCoreTest, OctetsToString_UppercaseHex) {
  // Verify that hex letters are uppercase
  OctetArray octets = {0xAB, 0xCD, 0xEF};
  std::string result = octets_to_string(octets);
  EXPECT_EQ(result, "AB CD EF");
  // Ensure no lowercase
  EXPECT_EQ(result.find('a'), std::string::npos);
  EXPECT_EQ(result.find('b'), std::string::npos);
  EXPECT_EQ(result.find('c'), std::string::npos);
  EXPECT_EQ(result.find('d'), std::string::npos);
  EXPECT_EQ(result.find('e'), std::string::npos);
  EXPECT_EQ(result.find('f'), std::string::npos);
}

// ============================================================================
// Tests for string_to_octet
// ============================================================================

TEST_F(HermesCoreTest, StringToOctet_ValidDecimal) {
  Octet result = 0;
  string_to_octet("42", result);
  EXPECT_EQ(result, 0x42);
}

TEST_F(HermesCoreTest, StringToOctet_ValidUppercaseHex) {
  Octet result = 0;
  string_to_octet("AB", result);
  EXPECT_EQ(result, 0xAB);

  string_to_octet("FF", result);
  EXPECT_EQ(result, 0xFF);

  string_to_octet("CD", result);
  EXPECT_EQ(result, 0xCD);
}

TEST_F(HermesCoreTest, StringToOctet_ZeroValue) {
  Octet result = 0xFF; // Initialize to non-zero
  string_to_octet("00", result);
  EXPECT_EQ(result, 0x00);
}

TEST_F(HermesCoreTest, StringToOctet_MixedDigits) {
  Octet result = 0;
  string_to_octet("A5", result);
  EXPECT_EQ(result, 0xA5);

  string_to_octet("5A", result);
  EXPECT_EQ(result, 0x5A);
}

TEST_F(HermesCoreTest, StringToOctet_InvalidLength) {
  Octet result = 0;

  // Too short
  EXPECT_THROW(string_to_octet("A", result), std::invalid_argument);

  // Too long
  EXPECT_THROW(string_to_octet("ABC", result), std::invalid_argument);

  // Empty
  EXPECT_THROW(string_to_octet("", result), std::invalid_argument);
}

TEST_F(HermesCoreTest, StringToOctet_InvalidCharacters) {
  Octet result = 0;

  // Lowercase (not allowed per spec)
  EXPECT_THROW(string_to_octet("ab", result), std::invalid_argument);

  // Non-hex characters
  EXPECT_THROW(string_to_octet("XY", result), std::invalid_argument);
  EXPECT_THROW(string_to_octet("G0", result), std::invalid_argument);
  EXPECT_THROW(string_to_octet("0Z", result), std::invalid_argument);

  // Special characters
  EXPECT_THROW(string_to_octet("!@", result), std::invalid_argument);
  EXPECT_THROW(string_to_octet(" 0", result), std::invalid_argument);
  EXPECT_THROW(string_to_octet("0 ", result), std::invalid_argument);
}

// ============================================================================
// Tests for string_to_octet_array
// ============================================================================

TEST_F(HermesCoreTest, StringToOctetArray_EmptyString) {
  OctetArray result;
  string_to_octet_array("", result);
  EXPECT_TRUE(result.empty());
}

TEST_F(HermesCoreTest, StringToOctetArray_SingleOctet) {
  OctetArray result;
  string_to_octet_array("42", result);
  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], 0x42);
}

TEST_F(HermesCoreTest, StringToOctetArray_MultipleOctets) {
  OctetArray result;
  string_to_octet_array("48 65 6C 6C 6F", result);
  ASSERT_EQ(result.size(), 5);
  EXPECT_EQ(result[0], 0x48); // 'H'
  EXPECT_EQ(result[1], 0x65); // 'e'
  EXPECT_EQ(result[2], 0x6C); // 'l'
  EXPECT_EQ(result[3], 0x6C); // 'l'
  EXPECT_EQ(result[4], 0x6F); // 'o'
}

TEST_F(HermesCoreTest, StringToOctetArray_LeadingSpaces) {
  OctetArray result;
  string_to_octet_array("  AB CD", result);
  ASSERT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], 0xAB);
  EXPECT_EQ(result[1], 0xCD);
}

TEST_F(HermesCoreTest, StringToOctetArray_TrailingSpaces) {
  OctetArray result;
  string_to_octet_array("AB CD  ", result);
  ASSERT_EQ(result.size(), 2);
  EXPECT_EQ(result[0], 0xAB);
  EXPECT_EQ(result[1], 0xCD);
}

TEST_F(HermesCoreTest, StringToOctetArray_MultipleSpaces) {
  OctetArray result;
  string_to_octet_array("AB   CD   EF", result);
  ASSERT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], 0xAB);
  EXPECT_EQ(result[1], 0xCD);
  EXPECT_EQ(result[2], 0xEF);
}

TEST_F(HermesCoreTest, StringToOctetArray_AllZeros) {
  OctetArray result;
  string_to_octet_array("00 00 00", result);
  ASSERT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], 0x00);
  EXPECT_EQ(result[1], 0x00);
  EXPECT_EQ(result[2], 0x00);
}

TEST_F(HermesCoreTest, StringToOctetArray_AllOnes) {
  OctetArray result;
  string_to_octet_array("FF FF FF", result);
  ASSERT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], 0xFF);
  EXPECT_EQ(result[1], 0xFF);
  EXPECT_EQ(result[2], 0xFF);
}

TEST_F(HermesCoreTest, StringToOctetArray_InvalidOddLength) {
  OctetArray result;
  // Each hex pair must be exactly 2 digits
  EXPECT_THROW(string_to_octet_array("A", result), std::invalid_argument);
  EXPECT_THROW(string_to_octet_array("AB C", result), std::invalid_argument);
  EXPECT_THROW(string_to_octet_array("AB CDE", result), std::invalid_argument);
}

TEST_F(HermesCoreTest, StringToOctetArray_InvalidCharacters) {
  OctetArray result;

  // Lowercase not allowed
  EXPECT_THROW(string_to_octet_array("ab cd", result), std::invalid_argument);

  // Non-hex characters
  EXPECT_THROW(string_to_octet_array("GG HH", result), std::invalid_argument);
  EXPECT_THROW(string_to_octet_array("XY ZZ", result), std::invalid_argument);
}

// ============================================================================
// Round-trip tests: verify that encoding and decoding are inverse operations
// ============================================================================

TEST_F(HermesCoreTest, RoundTrip_EmptyArray) {
  OctetArray original;
  std::string hex_str = octets_to_string(original);
  OctetArray decoded;
  string_to_octet_array(hex_str, decoded);
  EXPECT_EQ(original, decoded);
}

TEST_F(HermesCoreTest, RoundTrip_SingleOctet) {
  OctetArray original = {0x42};
  std::string hex_str = octets_to_string(original);
  OctetArray decoded;
  string_to_octet_array(hex_str, decoded);
  EXPECT_EQ(original, decoded);
}

TEST_F(HermesCoreTest, RoundTrip_MultipleOctets) {
  OctetArray original = {0x00, 0x01, 0x0F, 0x10, 0x7F, 0x80, 0xFF};
  std::string hex_str = octets_to_string(original);
  OctetArray decoded;
  string_to_octet_array(hex_str, decoded);
  EXPECT_EQ(original, decoded);
}

TEST_F(HermesCoreTest, RoundTrip_HelloWorld) {
  // "Hello" in ASCII
  OctetArray original = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
  std::string hex_str = octets_to_string(original);
  EXPECT_EQ(hex_str, "48 65 6C 6C 6F");
  OctetArray decoded;
  string_to_octet_array(hex_str, decoded);
  EXPECT_EQ(original, decoded);
}

TEST_F(HermesCoreTest, RoundTrip_AllByteValues) {
  // Test all possible byte values
  OctetArray original;
  original.reserve(256);
  for (int i = 0; i < 256; ++i) {
    original.push_back(static_cast<Octet>(i));
  }

  std::string hex_str = octets_to_string(original);
  OctetArray decoded;
  string_to_octet_array(hex_str, decoded);
  EXPECT_EQ(original, decoded);
}
