#ifndef THUMPER_CLIENT_SPARK_BOUNDARY_HPP
#define THUMPER_CLIENT_SPARK_BOUNDARY_HPP

#include <filesystem>
#include <string>

// Client SPARK boundary - interface between SPARK-verified code and OS
// operations.
//
// This module provides the boundary between SPARK-verified timestamp logic
// and operating system operations like file I/O. In the Ada version, SPARK
// contracts enforce that file I/O happens only at this boundary.
//
// In C++, we maintain this architectural separation even though we don't have
// formal SPARK verification. This boundary isolates system-dependent
// operations from the core cryptographic and encoding logic.
//
// Design Note: This corresponds to the Ada package Client_SPARK_Boundary.
// The Ada implementation uses Ada.Sequential_IO for file reading. The C++
// version uses std::filesystem and std::fstream for cross-platform file
// handling (Design Decision #6: Ada String to C++ String Representation).
//
// Thread Safety: Functions are not thread-safe when operating on the same
// files. Callers should ensure exclusive access to files being read/written.

namespace thumper::client {

// Obtains a timestamp for a document and stores it.
//
// This function reads a document file, computes its SHA-256 hash, creates
// a timestamp request, sends it to the timestamp server, receives the
// response, and writes the timestamp to a file.
//
// @param document_file_name Path to the document file to timestamp
// @param timestamp_file_name Path where the timestamp will be stored
//
// @throws std::runtime_error if file I/O fails or timestamp creation fails
// @throws std::filesystem::filesystem_error if file paths are invalid
//
// Implementation notes:
// - Uses streaming hash computation to handle large files efficiently
// - Reads document file byte-by-byte (like Ada's Sequential_IO)
// - Creates timestamp request by calling create_timestamp()
// - (Future) Will send request to server and receive response
// - Writes timestamp to output file
//
// The Ada version logs the computed hash using Client_Logger. The C++ version
// can optionally print hash to stderr for debugging.
//
// Corresponds to Ada's Fetch_Timestamp procedure.
void fetch_timestamp(const std::filesystem::path& document_file_name,
                     const std::filesystem::path& timestamp_file_name);

// Verifies that a timestamp is valid for a document.
//
// This function reads a document file and its corresponding timestamp file,
// then verifies that the timestamp correctly corresponds to the document.
//
// @param document_file_name Path to the document file
// @param timestamp_file_name Path to the timestamp file
// @return true if the timestamp is valid for the document, false otherwise
//
// @throws std::runtime_error if file I/O fails or verification fails
// @throws std::filesystem::filesystem_error if file paths are invalid
//
// Implementation notes:
// - Computes SHA-256 hash of the document file
// - Reads the timestamp file
// - Calls verify_timestamp() to validate the timestamp
//
// The Ada version logs the computed hash using Client_Logger. The C++ version
// can optionally print hash to stderr for debugging.
//
// Corresponds to Ada's Check_Timestamp function.
bool check_timestamp(const std::filesystem::path& document_file_name,
                     const std::filesystem::path& timestamp_file_name);

} // namespace thumper::client

#endif // THUMPER_CLIENT_SPARK_BOUNDARY_HPP
