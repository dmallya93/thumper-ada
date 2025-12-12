#ifndef THUMPER_HERMES_HERMES_HPP
#define THUMPER_HERMES_HERMES_HPP

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

#include "thumper/types.hpp"

namespace thumper::hermes {

// Converts an array of octets to a space-delimited collection of hex values.
// Example: [0x48, 0x65, 0x6C, 0x6C, 0x6F] -> "48 65 6C 6C 6F"
//
// This function is used throughout the Hermes library for debugging, testing,
// and human-readable representation of binary ASN.1 data.
//
// @param octets The array of octets to convert
// @return A string with space-delimited uppercase hex digits
std::string octets_to_string(std::span<const Octet> octets);

// Converts a string of two hex digits to an octet.
// The hex digits 'A' through 'F' must be uppercase.
//
// Preconditions:
// - hex_str.length() must equal 2
// - hex_str must contain only valid hex digits ('0'-'9', 'A'-'F')
//
// @param hex_str A string view containing exactly two hex digits
// @param result Output parameter receiving the converted octet value
// @throws std::invalid_argument if preconditions are violated
void string_to_octet(std::string_view hex_str, Octet& result);

// Converts a string of space-delimited octet values expressed in hex into an
// octet array.
//
// Example: "48 65 6C 6C 6F" -> [0x48, 0x65, 0x6C, 0x6C, 0x6F]
//
// This function parses a string containing hex digit pairs separated by spaces.
// The output array must be pre-sized to accommodate all octets. The function
// will populate the array starting at index 0.
//
// Preconditions:
// - hex_str must contain only hex digits ('0'-'9', 'A'-'F') and spaces
// - result must be sized to accommodate all octets in the string
//
// @param hex_str A string view containing space-delimited hex octets
// @param result Output array that will be populated with decoded octets
// @throws std::invalid_argument if hex_str contains invalid characters
// @throws std::out_of_range if result is too small for the decoded data
void string_to_octet_array(std::string_view hex_str, OctetArray& result);

// Helper function to check if a character is a valid hex digit.
// Valid hex digits are '0'-'9' and 'A'-'F' (uppercase only).
//
// @param ch The character to check
// @return true if ch is a valid hex digit, false otherwise
constexpr bool is_hex_digit(char ch) noexcept {
  return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F');
}

// Helper function to convert a hex digit character to its numeric value.
// Precondition: ch must be a valid hex digit (as determined by is_hex_digit).
//
// @param ch A valid hex digit character
// @return The numeric value of the hex digit (0-15)
constexpr Octet hex_digit_to_value(char ch) noexcept {
  if (ch >= '0' && ch <= '9') {
    return static_cast<Octet>(ch - '0');
  }
  return static_cast<Octet>(ch - 'A' + 10);
}

} // namespace thumper::hermes

#endif // THUMPER_HERMES_HERMES_HPP
