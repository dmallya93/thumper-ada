#include <array>
#include <vector>

#include <gtest/gtest.h>

#include "thumper/crypto/cryptographic_services.hpp"
#include "thumper/types.hpp"

using namespace thumper;
using namespace thumper::crypto;

namespace {

// Helper function to create an OctetArray from an initializer list
OctetArray make_octets(std::initializer_list<Octet> bytes) {
  return {bytes};
}

// Helper function to create a SHA256Hash from an initializer list
SHA256Hash make_hash(std::initializer_list<Octet> bytes) {
  SHA256Hash hash{};
  size_t i = 0;
  for (Octet byte : bytes) {
    if (i >= 32) {
      break;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    hash.at(i++) = byte;
  }
  return hash;
}

} // anonymous namespace

// Test SHA-256 hash of "Hello"
TEST(CryptoTest, SHA256_Hello) {
  OctetArray input = make_octets({72, 101, 108, 108, 111}); // "Hello"
  SHA256Hash expected = make_hash({0x18, 0x5f, 0x8d, 0xb3, 0x22, 0x71, 0xfe, 0x25, 0xf5, 0x61, 0xa6,
                                   0xfc, 0x93, 0x8b, 0x2e, 0x26, 0x43, 0x06, 0xec, 0x30, 0x4e, 0xda,
                                   0x51, 0x80, 0x07, 0xd1, 0x76, 0x48, 0x26, 0x38, 0x19, 0x69});

  SHA256Context ctx;
  ctx.update(input);
  SHA256Hash hash = ctx.finalize();

  EXPECT_EQ(hash, expected) << "SHA256 hash of 'Hello' does not match";
}

// Test SHA-256 hash of "Hello, World!" (split across multiple updates)
TEST(CryptoTest, SHA256_HelloWorld_Split) {
  // "Hello, World!" = {72, 101, 108, 108, 111, 44, 32, 87, 111, 114, 108, 100,
  // 33}
  OctetArray input_part1 = make_octets({72, 101, 108, 108, 111, 44, 32, 87, 111, 114}); // First 10
  OctetArray input_part2 = make_octets({108, 100, 33});                                 // Last 3

  SHA256Hash expected = make_hash({0xdf, 0xfd, 0x60, 0x21, 0xbb, 0x2b, 0xd5, 0xb0, 0xaf, 0x67, 0x62,
                                   0x90, 0x80, 0x9e, 0xc3, 0xa5, 0x31, 0x91, 0xdd, 0x81, 0xc7, 0xf7,
                                   0x0a, 0x4b, 0x28, 0x68, 0x8a, 0x36, 0x21, 0x82, 0x98, 0x6f});

  SHA256Context ctx;
  ctx.update(input_part1);
  ctx.update(input_part2);
  SHA256Hash hash = ctx.finalize();

  EXPECT_EQ(hash, expected) << "SHA256 hash of 'Hello, World!' (split) does not match";
}

// Test SHA-256 hash of empty message
TEST(CryptoTest, SHA256_Empty) {
  SHA256Hash expected = make_hash({0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4,
                                   0xc8, 0x99, 0x6f, 0xb9, 0x24, 0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b,
                                   0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55});

  SHA256Context ctx;
  SHA256Hash hash = ctx.finalize();

  EXPECT_EQ(hash, expected) << "SHA256 hash of empty message does not match";
}

// Test SHA-256 hash of single character "s"
TEST(CryptoTest, SHA256_SingleChar) {
  OctetArray input = make_octets({115}); // "s"
  SHA256Hash expected = make_hash({0x04, 0x3a, 0x71, 0x87, 0x74, 0xc5, 0x72, 0xbd, 0x8a, 0x25, 0xad,
                                   0xbe, 0xb1, 0xbf, 0xcd, 0x5c, 0x02, 0x56, 0xae, 0x11, 0xce, 0xcf,
                                   0x9f, 0x9c, 0x3f, 0x92, 0x5d, 0x0e, 0x52, 0xbe, 0xaf, 0x89});

  SHA256Context ctx;
  ctx.update(input);
  SHA256Hash hash = ctx.finalize();

  EXPECT_EQ(hash, expected) << "SHA256 hash of 's' does not match";
}

// Test SHA-256 hash of "12345678901234567890123456789012345" (35 chars)
TEST(CryptoTest, SHA256_35Digits) {
  OctetArray input =
      make_octets({49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 49, 50, 51, 52, 53, 54, 55, 56,
                   57, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 49, 50, 51, 52, 53});

  SHA256Hash expected = make_hash({0x48, 0xd7, 0x38, 0xca, 0x1c, 0x30, 0xfd, 0xae, 0xfd, 0x88, 0xaa,
                                   0x83, 0x6a, 0x26, 0xf4, 0x89, 0x8d, 0xe8, 0xb9, 0x20, 0xe6, 0x47,
                                   0x2c, 0x74, 0xaf, 0xe8, 0xab, 0x44, 0xa2, 0xba, 0x16, 0x49});

  SHA256Context ctx;
  ctx.update(input);
  SHA256Hash hash = ctx.finalize();

  EXPECT_EQ(hash, expected) << "SHA256 hash of 35-digit string does not match";
}

// Test SHA-256 hash of "123456789012345678901234567890123456" (36 chars, split
// into 3 parts)
TEST(CryptoTest, SHA256_36Digits_Split) {
  OctetArray input_part1 = make_octets({49, 50, 51, 52, 53, 54, 55, 56, 57, 48}); // Chars 1-10
  OctetArray input_part2 = make_octets({49, 50, 51, 52, 53, 54, 55, 56, 57, 48}); // Chars 11-20
  OctetArray input_part3 =
      make_octets({49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 49, 50, 51, 52, 53, 54}); // Chars 21-36

  SHA256Hash expected = make_hash({0xde, 0x38, 0x46, 0x95, 0x4e, 0x38, 0xb3, 0x43, 0x9c, 0x32, 0xe7,
                                   0x14, 0x70, 0x31, 0xa4, 0x3a, 0x4b, 0x00, 0x7b, 0x26, 0x12, 0x20,
                                   0xf0, 0x36, 0x81, 0x3c, 0x34, 0x08, 0xda, 0x60, 0x78, 0xa3});

  SHA256Context ctx;
  ctx.update(input_part1);
  ctx.update(input_part2);
  ctx.update(input_part3);
  SHA256Hash hash = ctx.finalize();

  EXPECT_EQ(hash, expected) << "SHA256 hash of 36-digit string (split) does not match";
}

// Test SHA-256 hash of 1,000,000 'a's (stress test for large data)
TEST(CryptoTest, SHA256_Million_As) {
  std::vector<Octet> input(1'000'000, 97); // 1 million 'a's (ASCII 97)

  SHA256Hash expected = make_hash({0xcd, 0xc7, 0x6e, 0x5c, 0x99, 0x14, 0xfb, 0x92, 0x81, 0xa1, 0xc7,
                                   0xe2, 0x84, 0xd7, 0x3e, 0x67, 0xf1, 0x80, 0x9a, 0x48, 0xa4, 0x97,
                                   0x20, 0x0e, 0x04, 0x6d, 0x39, 0xcc, 0xc7, 0x11, 0x2c, 0xd0});

  SHA256Context ctx;
  ctx.update(input);
  SHA256Hash hash = ctx.finalize();

  EXPECT_EQ(hash, expected) << "SHA256 hash of 1 million 'a's does not match";
}

// Test that the convenience function produces the same result
TEST(CryptoTest, SHA256_ConvenienceFunction) {
  OctetArray input = make_octets({72, 101, 108, 108, 111}); // "Hello"
  SHA256Hash expected = make_hash({0x18, 0x5f, 0x8d, 0xb3, 0x22, 0x71, 0xfe, 0x25, 0xf5, 0x61, 0xa6,
                                   0xfc, 0x93, 0x8b, 0x2e, 0x26, 0x43, 0x06, 0xec, 0x30, 0x4e, 0xda,
                                   0x51, 0x80, 0x07, 0xd1, 0x76, 0x48, 0x26, 0x38, 0x19, 0x69});

  SHA256Hash hash = compute_sha256(input);

  EXPECT_EQ(hash, expected) << "compute_sha256() convenience function does not match";
}

// Test that context can be reused after finalize()
TEST(CryptoTest, SHA256_ContextReuse) {
  OctetArray input = make_octets({72, 101, 108, 108, 111}); // "Hello"
  SHA256Hash expected = make_hash({0x18, 0x5f, 0x8d, 0xb3, 0x22, 0x71, 0xfe, 0x25, 0xf5, 0x61, 0xa6,
                                   0xfc, 0x93, 0x8b, 0x2e, 0x26, 0x43, 0x06, 0xec, 0x30, 0x4e, 0xda,
                                   0x51, 0x80, 0x07, 0xd1, 0x76, 0x48, 0x26, 0x38, 0x19, 0x69});

  SHA256Context ctx;

  // First use
  ctx.update(input);
  SHA256Hash hash1 = ctx.finalize();
  EXPECT_EQ(hash1, expected);

  // Reuse the same context
  ctx.update(input);
  SHA256Hash hash2 = ctx.finalize();
  EXPECT_EQ(hash2, expected) << "Context reuse produces different result";
}

// Test update with empty data (should be a no-op)
TEST(CryptoTest, SHA256_EmptyUpdate) {
  OctetArray input = make_octets({72, 101, 108, 108, 111}); // "Hello"
  OctetArray empty;

  SHA256Hash expected = make_hash({0x18, 0x5f, 0x8d, 0xb3, 0x22, 0x71, 0xfe, 0x25, 0xf5, 0x61, 0xa6,
                                   0xfc, 0x93, 0x8b, 0x2e, 0x26, 0x43, 0x06, 0xec, 0x30, 0x4e, 0xda,
                                   0x51, 0x80, 0x07, 0xd1, 0x76, 0x48, 0x26, 0x38, 0x19, 0x69});

  SHA256Context ctx;
  ctx.update(empty); // Empty update
  ctx.update(input); // Actual data
  ctx.update(empty); // Another empty update
  SHA256Hash hash = ctx.finalize();

  EXPECT_EQ(hash, expected) << "Hash with empty updates does not match expected";
}

// Test RSA key initialization with non-existent file
TEST(CryptoTest, RSA_InitializeKey_BadFile) {
  Status status = initialize_key("/nonexistent/path/to/private_key_that_does_not_exist.pem");
  EXPECT_EQ(status, Status::BadKey) << "initialize_key should return BadKey for non-existent file";
}

// Test RSA key initialization with valid key file
TEST(CryptoTest, RSA_InitializeKey_Success) {
  // Use relative path from build directory (build/clang-debug -> ../../tests)
  Status status = initialize_key("../../tests/test_keys/test_private_key.pem");
  EXPECT_EQ(status, Status::Success) << "initialize_key should return Success for valid key file";
}

// Test making a signature and verifying it
TEST(CryptoTest, RSA_MakeSignature_And_Verify) {
  // Initialize with test private key
  Status status = initialize_key("../../tests/test_keys/test_private_key.pem");
  ASSERT_EQ(status, Status::Success) << "Failed to load private key";

  // Test data to sign
  OctetArray test_data = make_octets({72, 101, 108, 108, 111}); // "Hello"

  // Create signature
  OctetArray signature = make_signature(test_data);

  // Verify signature is not empty
  EXPECT_GT(signature.size(), 0) << "Signature should not be empty";

  // Verify the signature with the public key
  bool is_valid =
      verify_signature(test_data, signature, "../../tests/test_keys/test_public_key.pem");
  EXPECT_TRUE(is_valid) << "Signature verification should succeed";
}

// Test signature verification fails with wrong data
TEST(CryptoTest, RSA_Signature_Wrong_Data) {
  // Initialize with test private key
  Status status = initialize_key("../../tests/test_keys/test_private_key.pem");
  ASSERT_EQ(status, Status::Success) << "Failed to load private key";

  // Original data
  OctetArray original_data = make_octets({72, 101, 108, 108, 111}); // "Hello"

  // Create signature
  OctetArray signature = make_signature(original_data);

  // Try to verify with different data
  OctetArray wrong_data = make_octets({87, 111, 114, 108, 100}); // "World"

  bool is_valid =
      verify_signature(wrong_data, signature, "../../tests/test_keys/test_public_key.pem");
  EXPECT_FALSE(is_valid) << "Signature verification should fail with wrong data";
}

// Test signature verification fails with corrupted signature
TEST(CryptoTest, RSA_Signature_Corrupted) {
  // Initialize with test private key
  Status status = initialize_key("../../tests/test_keys/test_private_key.pem");
  ASSERT_EQ(status, Status::Success) << "Failed to load private key";

  // Test data
  OctetArray test_data = make_octets({72, 101, 108, 108, 111}); // "Hello"

  // Create signature
  OctetArray signature = make_signature(test_data);

  // Corrupt the signature
  if (!signature.empty()) {
    signature[0] ^= 0xFF; // Flip all bits in first byte
  }

  bool is_valid =
      verify_signature(test_data, signature, "../../tests/test_keys/test_public_key.pem");
  EXPECT_FALSE(is_valid) << "Signature verification should fail with corrupted signature";
}

// Test signing larger data
TEST(CryptoTest, RSA_Signature_Large_Data) {
  // Initialize with test private key
  Status status = initialize_key("../../tests/test_keys/test_private_key.pem");
  ASSERT_EQ(status, Status::Success) << "Failed to load private key";

  // Create larger test data (1KB)
  std::vector<Octet> large_data(1024);
  for (size_t i = 0; i < large_data.size(); ++i) {
    large_data[i] = static_cast<Octet>(i % 256);
  }

  // Create signature
  OctetArray signature = make_signature(large_data);

  // Verify signature
  bool is_valid =
      verify_signature(large_data, signature, "../../tests/test_keys/test_public_key.pem");
  EXPECT_TRUE(is_valid) << "Signature verification should succeed for large data";
}
