#include "thumper/hermes/hermes.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace thumper::hermes {

std::string octets_to_string(std::span<const Octet> octets) {
  if (octets.empty()) {
    return "";
  }

  // Lookup table for hex digits
  static constexpr std::string_view HEX_CHARS = "0123456789ABCDEF";

  // Calculate result string size: 2 hex digits per octet + spaces between
  // Format: "XX XX XX" -> 3 chars per octet minus 1 for no trailing space
  const std::size_t RESULT_SIZE = (octets.size() * 3) - 1;
  std::string result;
  result.reserve(RESULT_SIZE);

  for (std::size_t i = 0; i < octets.size(); ++i) {
    const Octet OCTET_VALUE = octets[i];

    // Convert high nibble
    result.push_back(HEX_CHARS[OCTET_VALUE >> 4]);
    // Convert low nibble
    result.push_back(HEX_CHARS[OCTET_VALUE & 0x0F]);

    // Add space separator (except after last octet)
    if (i < octets.size() - 1) {
      result.push_back(' ');
    }
  }

  return result;
}

void string_to_octet(std::string_view hex_str, Octet& result) {
  // Validate preconditions
  if (hex_str.length() != 2) {
    throw std::invalid_argument("string_to_octet: hex string must be exactly 2 characters");
  }

  if (!is_hex_digit(hex_str[0]) || !is_hex_digit(hex_str[1])) {
    throw std::invalid_argument("string_to_octet: string contains invalid hex digits");
  }

  // Convert two hex digits to octet value
  const Octet HIGH_NIBBLE = hex_digit_to_value(hex_str[0]);
  const Octet LOW_NIBBLE = hex_digit_to_value(hex_str[1]);
  result = static_cast<Octet>((HIGH_NIBBLE << 4) | LOW_NIBBLE);
}

void string_to_octet_array(std::string_view hex_str, OctetArray& result) {
  // Clear the output array
  result.clear();

  // Handle empty input
  if (hex_str.empty()) {
    return;
  }

  // Parse space-delimited hex pairs
  std::size_t pos = 0;
  while (pos < hex_str.length()) {
    // Skip leading spaces
    while (pos < hex_str.length() && hex_str[pos] == ' ') {
      ++pos;
    }

    // Check if we reached the end
    if (pos >= hex_str.length()) {
      break;
    }

    // Find the end of this hex pair (space or end of string)
    std::size_t token_start = pos;
    while (pos < hex_str.length() && hex_str[pos] != ' ') {
      ++pos;
    }

    // Extract the token
    std::string_view token = hex_str.substr(token_start, pos - token_start);

    // Validate that it's a valid hex pair
    if (token.length() != 2) {
      throw std::invalid_argument("string_to_octet_array: each hex pair must be exactly 2 digits");
    }

    // Convert and add to result
    Octet octet_value = 0;
    string_to_octet(token, octet_value);
    result.push_back(octet_value);
  }
}

} // namespace thumper::hermes
