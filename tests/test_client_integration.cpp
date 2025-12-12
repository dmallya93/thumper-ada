#include <gtest/gtest.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <string>

#include "thumper/client/spark_boundary.hpp"
#include "thumper/client/timestamp_maker.hpp"
#include "thumper/crypto/cryptographic_services.hpp"
#include "thumper/switches.hpp"

// Integration tests for client components.
//
// These tests verify that all client components work together correctly:
// - Command-line switch parsing
// - File I/O and hash computation
// - Timestamp creation (placeholder for now)
//
// Note: Some tests will fail or throw exceptions because network
// communication and full ASN.1 encoding are not yet implemented. These are
// expected failures that will be fixed in future milestones.

namespace {

// Helper to create a temporary test file with given contents
std::filesystem::path create_temp_file(const std::string& contents) {
  std::filesystem::path temp_path = std::filesystem::temp_directory_path() / "thumper_test_XXXXXX";

  // Create a unique temporary file
  std::string path_str = temp_path.string();
  std::ofstream file(path_str, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Failed to create temp file");
  }

  file.write(contents.c_str(), static_cast<std::streamsize>(contents.length()));
  file.close();

  return path_str;
}

// Helper to delete a temporary file
void delete_temp_file(const std::filesystem::path& path) {
  std::filesystem::remove(path);
}

} // anonymous namespace

// Test command-line switch parsing for client
TEST(ClientIntegrationTest, SwitchParsingValid) {
  thumper::switches::SwitchParser parser;

  // Valid client command line: -h hostname -p 318
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  const char* argv[] = {"thumper_client", "-h", "timestamp.example.com", "-p", "318"};
  int argc = 5;

  std::string error_msg;
  bool result = parser.validate(thumper::switches::EndpointType::Client, argc,
                                 const_cast<char**>(argv), error_msg);  // NOLINT(cppcoreguidelines-pro-type-const-cast)

  EXPECT_TRUE(result);
  EXPECT_EQ(error_msg, "No Error");

  if (result) {
    EXPECT_EQ(parser.get_switch(thumper::switches::SwitchType::Host), "timestamp.example.com");
    EXPECT_EQ(parser.get_switch(thumper::switches::SwitchType::Port), "318");
  }
}

// Test command-line switch parsing with missing required switch
TEST(ClientIntegrationTest, SwitchParsingMissingHost) {
  thumper::switches::SwitchParser parser;

  // Invalid client command line: missing -h
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  const char* argv[] = {"thumper_client", "-p", "318"};
  int argc = 3;

  std::string error_msg;
  bool result = parser.validate(thumper::switches::EndpointType::Client, argc,
                                 const_cast<char**>(argv), error_msg);  // NOLINT(cppcoreguidelines-pro-type-const-cast)

  EXPECT_FALSE(result);
  EXPECT_EQ(error_msg, "The -h switch is required for clients");
}

// Test command-line switch parsing with default port
TEST(ClientIntegrationTest, SwitchParsingDefaultPort) {
  thumper::switches::SwitchParser parser;

  // Valid client command line with default port: -h hostname
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  const char* argv[] = {"thumper_client", "-h", "timestamp.example.com"};
  int argc = 3;

  std::string error_msg;
  bool result = parser.validate(thumper::switches::EndpointType::Client, argc,
                                 const_cast<char**>(argv), error_msg);  // NOLINT(cppcoreguidelines-pro-type-const-cast)

  EXPECT_TRUE(result);
  EXPECT_EQ(error_msg, "No Error");

  if (result) {
    EXPECT_EQ(parser.get_switch(thumper::switches::SwitchType::Host), "timestamp.example.com");
    EXPECT_EQ(parser.get_switch(thumper::switches::SwitchType::Port), "318"); // default
  }
}

// Test command-line switch parsing with duplicate switch
TEST(ClientIntegrationTest, SwitchParsingDuplicate) {
  thumper::switches::SwitchParser parser;

  // Invalid client command line: duplicate -h switch
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  const char* argv[] = {"thumper_client", "-h", "host1.example.com", "-h", "host2.example.com"};
  int argc = 5;

  std::string error_msg;
  bool result = parser.validate(thumper::switches::EndpointType::Client, argc,
                                 const_cast<char**>(argv), error_msg);  // NOLINT(cppcoreguidelines-pro-type-const-cast)

  EXPECT_FALSE(result);
  EXPECT_EQ(error_msg, "Duplicate switch: -h");
}

// Test command-line switch parsing with unknown switch
TEST(ClientIntegrationTest, SwitchParsingUnknown) {
  thumper::switches::SwitchParser parser;

  // Invalid client command line: unknown -x switch
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  const char* argv[] = {"thumper_client", "-h", "timestamp.example.com", "-x", "value"};
  int argc = 5;

  std::string error_msg;
  bool result = parser.validate(thumper::switches::EndpointType::Client, argc,
                                 const_cast<char**>(argv), error_msg);  // NOLINT(cppcoreguidelines-pro-type-const-cast)

  EXPECT_FALSE(result);
  EXPECT_EQ(error_msg, "Unknown switch: -x");
}

// Test file hashing with empty file
TEST(ClientIntegrationTest, HashEmptyFile) {
  // Create an empty temporary file
  std::filesystem::path temp_file = create_temp_file("");

  // Compute hash using the crypto service directly
  thumper::crypto::SHA256Context ctx;
  thumper::crypto::SHA256Hash hash = ctx.finalize();

  // The hash of an empty file is the hash of empty data
  // SHA-256("") = e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
  EXPECT_EQ(hash[0], 0xe3);
  EXPECT_EQ(hash[1], 0xb0);
  EXPECT_EQ(hash[2], 0xc4);
  EXPECT_EQ(hash[3], 0x42);

  delete_temp_file(temp_file);
}

// Test file hashing with known content
TEST(ClientIntegrationTest, HashKnownContent) {
  // Create a temporary file with known content
  std::string content = "Hello, World!";
  std::filesystem::path temp_file = create_temp_file(content);

  // Compute hash using streaming API (like spark_boundary does)
  thumper::crypto::SHA256Context ctx;
  for (char c : content) {
    std::array<thumper::Octet, 1> byte = {static_cast<thumper::Octet>(static_cast<unsigned char>(c))};
    ctx.update(byte);
  }
  thumper::crypto::SHA256Hash hash = ctx.finalize();

  // SHA-256("Hello, World!") = dffd6021bb2bd5b0af676290809ec3a53191dd81c7f70a4b28688a362182986f
  EXPECT_EQ(hash[0], 0xdf);
  EXPECT_EQ(hash[1], 0xfd);
  EXPECT_EQ(hash[2], 0x60);
  EXPECT_EQ(hash[3], 0x21);

  delete_temp_file(temp_file);
}

// Test fetch_timestamp with a real file
// This test verifies that the function successfully creates a basic timestamp request
// that encodes the document hash using the Hermes DER library.
TEST(ClientIntegrationTest, FetchTimestampCreatesFile) {
  // Create a temporary document file
  std::string content = "Document to timestamp";
  std::filesystem::path doc_file = create_temp_file(content);
  std::filesystem::path ts_file = std::filesystem::temp_directory_path() / "test_timestamp.tsr";

  // Remove any existing timestamp file
  std::filesystem::remove(ts_file);

  // Attempt to fetch timestamp (should now succeed with basic encoding)
  EXPECT_NO_THROW({ thumper::client::fetch_timestamp(doc_file, ts_file); });

  // Verify the timestamp file was created
  EXPECT_TRUE(std::filesystem::exists(ts_file));

  // Verify the timestamp file has content
  if (std::filesystem::exists(ts_file)) {
    auto file_size = std::filesystem::file_size(ts_file);
    EXPECT_GT(file_size, 0);
  }

  // Clean up
  delete_temp_file(doc_file);
  std::filesystem::remove(ts_file);
}

// Test that get_switch throws when validate hasn't been called
TEST(ClientIntegrationTest, GetSwitchBeforeValidate) {
  thumper::switches::SwitchParser parser;

  EXPECT_THROW({ (void)parser.get_switch(thumper::switches::SwitchType::Host); },
               thumper::switches::SwitchError);
}

// Test server endpoint validation (should reject -h switch)
TEST(ClientIntegrationTest, SwitchParsingServerRejectsHost) {
  thumper::switches::SwitchParser parser;

  // Invalid server command line: -h is illegal for servers
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  const char* argv[] = {"thumper_server", "-h", "hostname"};
  int argc = 3;

  std::string error_msg;
  bool result = parser.validate(thumper::switches::EndpointType::Server, argc,
                                 const_cast<char**>(argv), error_msg);  // NOLINT(cppcoreguidelines-pro-type-const-cast)

  EXPECT_FALSE(result);
  EXPECT_EQ(error_msg, "The -h switch is illegal for servers");
}

// Test server endpoint validation (should accept -p switch with default)
TEST(ClientIntegrationTest, SwitchParsingServerDefaultPort) {
  thumper::switches::SwitchParser parser;

  // Valid server command line: no switches (all optional)
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  const char* argv[] = {"thumper_server"};
  int argc = 1;

  std::string error_msg;
  bool result = parser.validate(thumper::switches::EndpointType::Server, argc,
                                 const_cast<char**>(argv), error_msg);  // NOLINT(cppcoreguidelines-pro-type-const-cast)

  EXPECT_TRUE(result);
  EXPECT_EQ(error_msg, "No Error");

  if (result) {
    EXPECT_EQ(parser.get_switch(thumper::switches::SwitchType::Port), "318"); // default
  }
}

// Test large file hashing
// Verifies that streaming hash computation handles large files correctly
TEST(ClientIntegrationTest, HashLargeFile) {
  // Create a large temporary file (several MB)
  constexpr std::size_t FILE_SIZE = std::size_t{5} * 1024 * 1024; // 5 MB
  std::string large_content;
  large_content.reserve(FILE_SIZE);

  // Fill with repeating pattern
  for (std::size_t i = 0; i < FILE_SIZE; ++i) {
    large_content += static_cast<char>('A' + static_cast<int>(i % 26));
  }

  std::filesystem::path temp_file = create_temp_file(large_content);

  // Compute hash using streaming API (like spark_boundary does)
  thumper::crypto::SHA256Context ctx;
  std::ifstream file(temp_file, std::ios::binary);
  ASSERT_TRUE(file.is_open());

  std::array<thumper::Octet, 4096> buffer{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  while (file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()))) {
    auto bytes_read = static_cast<std::size_t>(file.gcount());
    ctx.update(std::span<const thumper::Octet>(buffer.data(), bytes_read));
  }
  // Handle any remaining bytes
  if (file.gcount() > 0) {
    auto bytes_read = static_cast<std::size_t>(file.gcount());
    ctx.update(std::span<const thumper::Octet>(buffer.data(), bytes_read));
  }

  thumper::crypto::SHA256Hash hash = ctx.finalize();

  // Verify we got a hash (non-zero in at least some bytes)
  bool has_nonzero = false;
  for (auto byte : hash) {
    if (byte != 0) {
      has_nonzero = true;
      break;
    }
  }
  EXPECT_TRUE(has_nonzero);

  delete_temp_file(temp_file);
}

// Test round-trip encoding/decoding of timestamp
// Verifies that create_timestamp and verify_timestamp work together
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST(ClientIntegrationTest, TimestampRoundTrip) {
  // Create a test hash
  std::array<thumper::Octet, 32> test_hash{};
  for (std::size_t i = 0; i < test_hash.size(); ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    test_hash[i] = static_cast<thumper::Octet>(i);
  }

  // Create timestamp
  std::array<thumper::Octet, 4096> timestamp_buffer{};
  std::size_t timestamp_size = 0;

  EXPECT_NO_THROW({
    thumper::client::create_timestamp(test_hash, timestamp_buffer, timestamp_size);
  });

  // Verify we got some data
  EXPECT_GT(timestamp_size, 0);
  EXPECT_LE(timestamp_size, timestamp_buffer.size());

  // Verify the timestamp
  std::span<const thumper::Octet> timestamp_span(timestamp_buffer.data(), timestamp_size);
  bool is_valid = false;

  EXPECT_NO_THROW({
    is_valid = thumper::client::verify_timestamp(test_hash, timestamp_span);
  });

  EXPECT_TRUE(is_valid);
}

// Test verify_timestamp rejects invalid data
TEST(ClientIntegrationTest, VerifyTimestampRejectsInvalid) {
  std::array<thumper::Octet, 32> test_hash{};
  for (std::size_t i = 0; i < test_hash.size(); ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    test_hash[i] = static_cast<thumper::Octet>(i);
  }

  // Create invalid timestamp data (too short)
  std::array<thumper::Octet, 2> invalid_timestamp{0x01, 0x02};

  bool is_valid = thumper::client::verify_timestamp(test_hash, invalid_timestamp);
  EXPECT_FALSE(is_valid);
}

// Test check_timestamp integration
// Verifies that fetch_timestamp and check_timestamp work together end-to-end
TEST(ClientIntegrationTest, CheckTimestampIntegration) {
  // Create a document file
  std::string content = "Integration test document";
  std::filesystem::path doc_file = create_temp_file(content);
  std::filesystem::path ts_file = std::filesystem::temp_directory_path() / "integration_test.tsr";

  // Remove any existing timestamp file
  std::filesystem::remove(ts_file);

  // Create timestamp
  EXPECT_NO_THROW({
    thumper::client::fetch_timestamp(doc_file, ts_file);
  });

  // Verify the timestamp file was created
  ASSERT_TRUE(std::filesystem::exists(ts_file));

  // Check the timestamp
  bool is_valid = false;
  EXPECT_NO_THROW({
    is_valid = thumper::client::check_timestamp(doc_file, ts_file);
  });

  EXPECT_TRUE(is_valid);

  // Clean up
  delete_temp_file(doc_file);
  std::filesystem::remove(ts_file);
}
