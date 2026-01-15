///////////////////////////////////////////////////////////////////////////
// FILE    : hermes.hpp
// SUBJECT : Core types and utilities for the Hermes ASN.1 library
// AUTHOR  : (C) Copyright 2022 by Peter C. Chapin
//
// This is the top-level header for the Hermes ASN.1 library. It provides
// fundamental octet types and hex string conversion utilities used throughout
// the library.
//
// Design Decision #2 (Buffer Strategy): Using std::vector<uint8_t> for
// OctetArray to provide dynamic sizing with automatic memory management.
// This gives flexibility for variable-length ASN.1 structures while
// maintaining RAII safety.
//
// Design Decision #4 (Error Handling): Using enum class Status for explicit
// error handling, mirroring the Ada implementation. Functions that can fail
// return Status, while conversion functions may throw std::invalid_argument
// for truly exceptional cases (invalid input).
//
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace hermes {

    /// Basic octet type (8-bit unsigned byte)
    /// Maps from Ada: type Octet is mod 2**8;
    using Octet = std::uint8_t;

    /// Dynamic array of octets
    /// Maps from Ada: type Octet_Array is array(Natural range <>) of Octet;
    ///
    /// Using std::vector for automatic memory management and dynamic sizing.
    /// This supports variable-length ASN.1 structures naturally.
    using OctetArray = std::vector<Octet>;

    /// Converts an array of octets to a space-delimited string of hex values
    ///
    /// Maps from Ada: function Octets_To_String(Octets : in Octet_Array) return String;
    ///
    /// Example: {0x01, 0xFF, 0xA3} -> "01 FF A3"
    ///
    /// @param octets The octet array to convert
    /// @return Space-delimited hex string with uppercase A-F
    std::string octetsToString(const OctetArray& octets);

    /// Converts a string of two hex digits to an octet
    ///
    /// Maps from Ada: procedure String_To_Octet(Text : in Hex_String; Octet_Value : out Octet)
    ///               with Pre => Text'Length = 2;
    ///
    /// The digits A-F must be uppercase.
    ///
    /// @param text Two-character hex string (e.g., "1A", "FF")
    /// @return The corresponding octet value
    /// @throws std::invalid_argument if text length != 2 or contains invalid hex chars
    Octet stringToOctet(const std::string& text);

    /// Converts a space-delimited hex string into an octet array
    ///
    /// Maps from Ada: procedure String_To_Octet_Array(Text : in Readible_Hex_String;
    ///                                                  Result : out Octet_Array);
    ///
    /// Example: "01 FF A3" -> {0x01, 0xFF, 0xA3}
    ///
    /// Leading/trailing spaces are handled. Multiple spaces between octets are allowed.
    /// Valid characters: '0'-'9', 'A'-'F', space
    ///
    /// @param text Space-delimited hex string
    /// @return Vector of octets parsed from the string
    /// @throws std::invalid_argument if invalid hex characters are encountered
    OctetArray stringToOctetArray(const std::string& text);

} // namespace hermes

