#ifndef THUMPER_SWITCHES_HPP
#define THUMPER_SWITCHES_HPP

#include <map>
#include <stdexcept>
#include <string>
#include <string_view>

// Command-line switch parsing and validation for Thumper client and server.
//
// This module provides functionality to parse command-line arguments in the
// form of (name, value) pairs (e.g., "-h hostname -p port") and validate them
// according to the endpoint type (Client or Server).
//
// Usage:
//   1. Create a SwitchParser instance
//   2. Call validate() with the endpoint type and command-line arguments
//   3. If validation succeeds, use get_switch() to retrieve switch values
//
// Example:
//   SwitchParser parser;
//   std::string error_msg;
//   if (parser.validate(EndpointType::Client, argc, argv, error_msg)) {
//     std::string host = parser.get_switch(SwitchType::Host);
//     std::string port = parser.get_switch(SwitchType::Port);
//     // Use host and port...
//   } else {
//     std::cerr << "Error: " << error_msg << std::endl;
//   }
//
// Design corresponds to Ada's Thumper_Switches package which uses Ada.Command_Line
// and Ada.Containers.Ordered_Maps.

namespace thumper::switches {

// Exception thrown when get_switch() is called before successful validation.
class SwitchError : public std::runtime_error {
public:
  explicit SwitchError(const std::string& message) : std::runtime_error(message) {}
};

// Endpoint type determines which switches are required/allowed.
// - Client: requires -h switch, -p is optional (defaults to 318)
// - Server: -h is illegal, -p is optional (defaults to 318)
enum class EndpointType { Client, Server };

// Switch types supported by Thumper.
// - Host: Server hostname or IP address (required for client, illegal for
// server)
// - Port: Port number (optional, defaults to RFC-3161 standard port 318)
enum class SwitchType { Host, Port };

// SwitchParser manages command-line switch parsing and validation.
//
// This class encapsulates the state of parsed command-line switches and
// provides methods to validate and retrieve switch values. It corresponds
// to the Ada Thumper_Switches package which uses an internal Switch_Map.
//
// Thread Safety: Not thread-safe. Each thread should use its own parser.
class SwitchParser {
public:
  SwitchParser() = default;

  // Validates command-line switches for the specified endpoint type.
  //
  // This method parses command-line arguments and performs the following
  // checks:
  //   a) Arguments are in the form of multiple (name, value) pairs
  //   b) No switch names are duplicated
  //   c) Required switches are present (-h for client)
  //   d) Illegal switches are not present (-h for server)
  //   e) No unknown switches are present
  //   f) Optional switches are given default values if necessary (-p defaults
  //   to 318)
  //
  // The syntax of switch values is NOT checked by this method. Validation
  // of value formats (e.g., valid hostname, numeric port) should be done
  // by the caller after retrieving switch values.
  //
  // This method must be called successfully before get_switch() can be used.
  //
  // @param endpoint The endpoint type (Client or Server)
  // @param argc Argument count from main()
  // @param argv Argument vector from main()
  // @param error_message Output parameter containing error message on failure,
  //                      or "No Error" on success
  // @return true if validation succeeds, false otherwise
  //
  // Corresponds to Ada's Validate procedure.
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  bool validate(EndpointType endpoint, int argc, char* argv[], std::string& error_message);

  // Returns the value associated with the given switch.
  //
  // If the switch was not specified on the command line, a suitable default
  // is returned (currently only -p defaults to "318").
  //
  // @param switch_type The switch to retrieve
  // @return The switch value as a string
  // @throws SwitchError if validate() was not called successfully before this
  // method
  //
  // Corresponds to Ada's Get_Switch function.
  [[nodiscard]] std::string get_switch(SwitchType switch_type) const;

  // Clears all parsed switches and resets validation state.
  // After calling this method, validate() must be called again before
  // get_switch().
  void clear();

private:
  // Internal map from switch character to switch value.
  // Corresponds to Ada's Switch_Maps.Map type.
  std::map<char, std::string> switch_map_;

  // Validation state: true if validate() succeeded, false otherwise.
  bool is_validated_{false};

  // Checks if the switch map contains any unknown switches.
  // Known switches: 'h' (host), 'p' (port)
  // Returns true and sets error_message if unknown switches are found.
  bool check_unknown(std::string& error_message) const;

  // Checks endpoint-specific switch semantics.
  // - Client: requires 'h' switch
  // - Server: 'h' switch is illegal
  // Returns true and sets error_message if semantic errors are found.
  bool check_switch_semantics(EndpointType endpoint, std::string& error_message) const;
};

} // namespace thumper::switches

#endif // THUMPER_SWITCHES_HPP
