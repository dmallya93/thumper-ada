#include "thumper/switches.hpp"

#include <algorithm>
#include <cctype>

namespace thumper::switches {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
bool SwitchParser::validate(EndpointType endpoint, int argc, char* argv[],
                             std::string& error_message) {
  // Clear any previous state
  switch_map_.clear();
  is_validated_ = false;

  // Optimistically assume everything will work
  error_message = "No Error";

  // Note: argc includes the program name at argv[0], so we start from index 1
  // Ada's Argument_Count excludes the program name, so Ada's check is:
  // "Argument_Count rem 2 /= 0"
  // In C++, we need (argc - 1) to get the actual argument count
  int arg_count = argc - 1;

  // There must be an even number of arguments (could be zero)
  if (arg_count % 2 != 0) {
    error_message = "Invalid command line syntax";
    return false;
  }

  // Build the switch map associating switch names to their values
  // Start at argv[1] (skip program name at argv[0])
  for (int i = 1; i < argc; i += 2) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::string current_arg = argv[i];

    // Check if argument is in the form "-X" where X is a single character
    if (current_arg.length() != 2 || current_arg[0] != '-') {
      error_message = "Invalid switch: " + current_arg;
      return false;
    }

    char switch_char = current_arg[1];

    // Check for duplicate switches
    if (switch_map_.contains(switch_char)) {
      error_message = "Duplicate switch: " + current_arg;
      return false;
    }

    // Insert switch and its value (value is at i+1)
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    switch_map_[switch_char] = argv[i + 1];
  }

  // Set default values for optional switches that apply to both client and
  // server
  if (!switch_map_.contains('p')) {
    switch_map_['p'] = "318"; // RFC-3161 standard timestamp port
  }

  // Check for unknown switches
  if (check_unknown(error_message)) {
    return false;
  }

  // Check endpoint-specific switch semantics
  if (check_switch_semantics(endpoint, error_message)) {
    return false;
  }

  // All validation passed
  is_validated_ = true;
  return true;
}

std::string SwitchParser::get_switch(SwitchType switch_type) const {
  if (!is_validated_) {
    throw SwitchError("get_switch() called before successful validate()");
  }

  char switch_char = '\0';
  switch (switch_type) {
  case SwitchType::Host:
    switch_char = 'h';
    break;
  case SwitchType::Port:
    switch_char = 'p';
    break;
  }

  auto it = switch_map_.find(switch_char);
  if (it == switch_map_.end()) {
    throw SwitchError("Switch not found in map (should not happen after "
                      "successful validation)");
  }

  return it->second;
}

void SwitchParser::clear() {
  switch_map_.clear();
  is_validated_ = false;
}

bool SwitchParser::check_unknown(std::string& error_message) const {
  for (const auto& [switch_char, value] : switch_map_) {
    // Known switches: 'h' (host), 'p' (port)
    if (switch_char != 'h' && switch_char != 'p') {
      error_message = "Unknown switch: -";
      error_message += switch_char;
      return true;
    }
  }
  return false;
}

bool SwitchParser::check_switch_semantics(EndpointType endpoint,
                                          std::string& error_message) const {
  switch (endpoint) {
  case EndpointType::Client:
    // The 'h' switch is required for clients
    if (!switch_map_.contains('h')) {
      error_message = "The -h switch is required for clients";
      return true;
    }
    break;

  case EndpointType::Server:
    // The 'h' switch is illegal for servers
    if (switch_map_.contains('h')) {
      error_message = "The -h switch is illegal for servers";
      return true;
    }
    break;
  }

  return false;
}

} // namespace thumper::switches
