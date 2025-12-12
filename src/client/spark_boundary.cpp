#include "thumper/client/spark_boundary.hpp"

#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "thumper/client/timestamp_maker.hpp"
#include "thumper/crypto/cryptographic_services.hpp"
#include "thumper/hermes/hermes.hpp"

namespace thumper::client {

// Helper function to read a document file and compute its SHA-256 hash.
//
// This function corresponds to the Ada function Read_Document_File in
// Client_SPARK_Boundary. It reads the file byte-by-byte using streaming
// hash computation, just like the Ada version uses Sequential_IO.
//
// @param document_file_name Path to the document file
// @return The SHA-256 hash of the file contents
// @throws std::runtime_error if the file cannot be opened or read
static crypto::SHA256Hash read_document_file(const std::filesystem::path& document_file_name) {
  // Open file in binary mode
  std::ifstream file(document_file_name, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Failed to open document file: " + document_file_name.string());
  }

  // Initialize hash context (corresponds to Initialize_Hash)
  crypto::SHA256Context hash_ctx;

  // Read file byte-by-byte and update hash
  // This matches the Ada implementation which uses Sequential_IO to read
  // one octet at a time
  std::array<Octet, 1> read_octet_array{};
  char byte = 0;
  while (file.get(byte)) {
    read_octet_array[0] = static_cast<Octet>(static_cast<unsigned char>(byte));
    hash_ctx.update(read_octet_array);
  }

  // Check if we stopped due to an error (not just EOF)
  if (file.bad()) {
    throw std::runtime_error("Error reading document file: " + document_file_name.string());
  }

  // Finalize hash (corresponds to Finalize_Hash)
  return hash_ctx.finalize();
}

void fetch_timestamp(const std::filesystem::path& document_file_name,
                     const std::filesystem::path& timestamp_file_name) {
  // Compute SHA-256 hash of the document file
  crypto::SHA256Hash file_hash = read_document_file(document_file_name);

  // Convert hash to hex string for logging (corresponds to Octets_To_String)
  std::string printed_hash = hermes::octets_to_string(file_hash);

  // Log the file name and hash (corresponds to Client_Logger.Write_Information)
  // In the Ada version, this uses Client_Logger. In C++, we write to stderr
  // for debugging.
  std::cerr << "Verify file name: " << document_file_name.string() << "; SHA256 hash: " << printed_hash
            << '\n';

  // Create timestamp request
  // For now, this will throw an exception because create_timestamp is not
  // fully implemented (network communication is in future milestone)
  std::array<Octet, 4096> timestamp_buffer{};
  std::size_t timestamp_size = 0;

  try {
    create_timestamp(file_hash, timestamp_buffer, timestamp_size);

    // If we reach here, write the timestamp to file
    std::ofstream ts_file(timestamp_file_name, std::ios::binary);
    if (!ts_file) {
      throw std::runtime_error("Failed to open timestamp file for writing: " +
                               timestamp_file_name.string());
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    ts_file.write(reinterpret_cast<const char*>(timestamp_buffer.data()),
                  static_cast<std::streamsize>(timestamp_size));

    if (!ts_file) {
      throw std::runtime_error("Failed to write timestamp file: " + timestamp_file_name.string());
    }
  } catch (const std::runtime_error& e) {
    // Re-throw with additional context
    throw std::runtime_error(std::string("fetch_timestamp failed: ") + e.what());
  }
}

bool check_timestamp(const std::filesystem::path& document_file_name,
                     const std::filesystem::path& timestamp_file_name) {
  // Compute SHA-256 hash of the document file
  crypto::SHA256Hash file_hash = read_document_file(document_file_name);

  // Convert hash to hex string for logging
  std::string printed_hash = hermes::octets_to_string(file_hash);

  // Log the file name and hash (corresponds to Client_Logger.Write_Information)
  std::cerr << "Check file name: " << document_file_name.string() << "; SHA256 hash: " << printed_hash
            << '\n';

  // Read the timestamp file
  std::ifstream ts_file(timestamp_file_name, std::ios::binary);
  if (!ts_file) {
    throw std::runtime_error("Failed to open timestamp file: " + timestamp_file_name.string());
  }

  // Read entire timestamp file into buffer
  std::vector<Octet> timestamp_buffer;
  char byte = 0;
  while (ts_file.get(byte)) {
    timestamp_buffer.push_back(static_cast<Octet>(static_cast<unsigned char>(byte)));
  }

  if (ts_file.bad()) {
    throw std::runtime_error("Error reading timestamp file: " + timestamp_file_name.string());
  }

  // Verify the timestamp
  try {
    return verify_timestamp(file_hash, timestamp_buffer);
  } catch (const std::runtime_error& e) {
    // Re-throw with additional context
    throw std::runtime_error(std::string("check_timestamp failed: ") + e.what());
  }
}

} // namespace thumper::client
