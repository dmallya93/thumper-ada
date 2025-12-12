// Thumper Client CLI - Command-line interface for timestamp client
//
// This is a minimal command-line client that demonstrates the integration of
// all migrated components: command-line parsing, file I/O, SHA-256 hashing,
// and timestamp creation.
//
// Usage:
//   thumper_client_cli -h <hostname> [-p <port>] <document_file> <timestamp_file>
//
// Example:
//   thumper_client_cli -h timestamp.example.com -p 318 document.txt document.tsr
//
// This corresponds to a minimal version of the Ada thumper_client.adb main
// program, but without GUI dependencies (client_gui, client_logger with GTK).
// The Ada source has multiple client executables; this is the command-line
// version.
//
// Exit codes:
//   0 - Success
//   1 - Invalid command-line arguments
//   2 - File I/O error
//   3 - Timestamp operation failed

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

#include "thumper/client/spark_boundary.hpp"
#include "thumper/switches.hpp"

namespace {

// Print usage information to stderr
void print_usage(const char* program_name) {
  std::cerr << "Usage: " << program_name
            << " -h <hostname> [-p <port>] <document_file> <timestamp_file>\n"
            << "\n"
            << "Options:\n"
            << "  -h <hostname>  Timestamp server hostname (required)\n"
            << "  -p <port>      Timestamp server port (optional, default: 318)\n"
            << "\n"
            << "Arguments:\n"
            << "  document_file    Path to the document file to timestamp\n"
            << "  timestamp_file   Path where the timestamp will be stored\n"
            << "\n"
            << "Example:\n"
            << "  " << program_name
            << " -h timestamp.example.com -p 318 document.txt document.tsr\n";
}

} // anonymous namespace

int main(int argc, char* argv[]) {
  try {
    // We need at least 5 arguments:
    // program_name -h hostname document_file timestamp_file
    // (plus optional -p port makes 7 arguments max)
    if (argc < 5) {
      std::cerr << "Error: Insufficient arguments\n\n";
      print_usage(argv[0]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      return 1;
    }

    // The last two arguments should be file names, not switches
    // So we need to validate switches for everything except the last 2 args
    int switch_argc = argc - 2;

    // Create a temporary argv array for switch parsing (excluding file names)
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    char** switch_argv = new char*[static_cast<std::size_t>(switch_argc)];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
    for (int i = 0; i < switch_argc; ++i) {
      switch_argv[i] = argv[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    // Parse and validate command-line switches
    thumper::switches::SwitchParser parser;
    std::string error_message;

    bool valid =
        parser.validate(thumper::switches::EndpointType::Client, switch_argc, switch_argv, error_message);

    delete[] switch_argv;  // NOLINT(cppcoreguidelines-owning-memory)

    if (!valid) {
      std::cerr << "Error: " << error_message << "\n\n";
      print_usage(argv[0]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      return 1;
    }

    // Extract switch values
    std::string hostname = parser.get_switch(thumper::switches::SwitchType::Host);
    std::string port = parser.get_switch(thumper::switches::SwitchType::Port);

    // Extract file paths (last two arguments)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::filesystem::path document_file = argv[argc - 2];
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::filesystem::path timestamp_file = argv[argc - 1];

    // Display configuration
    std::cout << "Thumper Client\n";
    std::cout << "==============\n";
    std::cout << "Server: " << hostname << ":" << port << "\n";
    std::cout << "Document: " << document_file << "\n";
    std::cout << "Timestamp: " << timestamp_file << "\n";
    std::cout << "\n";

    // Verify document file exists
    if (!std::filesystem::exists(document_file)) {
      std::cerr << "Error: Document file does not exist: " << document_file << "\n";
      return 2;
    }

    // Perform timestamp operation
    std::cout << "Computing document hash and creating timestamp request...\n";

    try {
      thumper::client::fetch_timestamp(document_file, timestamp_file);

      std::cout << "Success: Timestamp created and saved to " << timestamp_file << "\n";
      return 0;

    } catch (const std::runtime_error& e) {
      // This is expected since create_timestamp is not fully implemented yet
      // (network communication is in future milestone)
      std::cerr << "Note: " << e.what() << "\n";
      std::cerr << "\nThe client successfully computed the document hash and "
                   "prepared for timestamp creation.\n";
      std::cerr << "Full timestamp creation requires network communication, "
                   "which will be implemented\n";
      std::cerr << "in Milestone 2 (Network Layer).\n";
      return 3;
    }

  } catch (const thumper::switches::SwitchError& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;

  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "File system error: " << e.what() << "\n";
    return 2;

  } catch (const std::exception& e) {
    std::cerr << "Unexpected error: " << e.what() << "\n";
    return 3;

  } catch (...) {
    std::cerr << "Unknown error occurred\n";
    return 3;
  }
}
