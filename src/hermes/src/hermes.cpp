///////////////////////////////////////////////////////////////////////////
// FILE    : hermes.cpp
// SUBJECT : Implementation of core Hermes utilities
// AUTHOR  : (C) Copyright 2022 by Peter C. Chapin
//
// This file contains implementations of hex string conversion utilities
// that are used throughout the Hermes library, particularly for testing
// and debugging ASN.1 encoded data.
//
///////////////////////////////////////////////////////////////////////////

#include "hermes/hermes.hpp"
#include <stdexcept>
#include <cctype>

namespace hermes {

    namespace {
        // Helper: Convert a hex character to its numeric value
        // Maps from Ada's Lookup_Octet nested function
        Octet hexCharToValue(char ch) {
            if (ch >= '0' && ch <= '9') {
                return static_cast<Octet>(ch - '0');
            }
            if (ch >= 'A' && ch <= 'F') {
                return static_cast<Octet>(ch - 'A' + 10);
            }
            throw std::invalid_argument("Invalid hex character");
        }

        // Helper: Convert a 4-bit value to uppercase hex character
        char valueToHexChar(Octet value) {
            static const char lookup[] = "0123456789ABCDEF";
            return lookup[value & 0x0F];
        }
    }

    std::string octetsToString(const OctetArray& octets) {
        // Maps from Ada: function Octets_To_String(Octets : in Octet_Array) return String

        if (octets.empty()) {
            return "";
        }

        // Calculate result size: 2 chars per octet + (n-1) spaces
        // Ada: Returned_String : String(1 .. 3 * Octets'Length - 1)
        std::string result;
        result.reserve(3 * octets.size() - 1);

        for (size_t i = 0; i < octets.size(); ++i) {
            if (i > 0) {
                result += ' ';
            }

            Octet value = octets[i];
            // High nibble
            result += valueToHexChar(value >> 4);
            // Low nibble
            result += valueToHexChar(value & 0x0F);
        }

        return result;
    }

    Octet stringToOctet(const std::string& text) {
        // Maps from Ada: procedure String_To_Octet(Text : in Hex_String; Octet_Value : out Octet)
        //               with Pre => Text'Length = 2;

        if (text.length() != 2) {
            throw std::invalid_argument("Hex string must be exactly 2 characters");
        }

        // Ada: Octet_Value := 16 * Lookup_Octet(Text(Text'First)) + Lookup_Octet(Text(Text'Last));
        return static_cast<Octet>(16 * hexCharToValue(text[0]) + hexCharToValue(text[1]));
    }

    OctetArray stringToOctetArray(const std::string& text) {
        // Maps from Ada: procedure String_To_Octet_Array(Text : in Readible_Hex_String;
        //                                                  Result : out Octet_Array)

        OctetArray result;
        std::string hexPair;
        hexPair.reserve(2);

        // Parse space-delimited hex values
        // Ada uses Find_Token with character set for spaces
        for (size_t i = 0; i < text.length(); ++i) {
            char ch = text[i];

            if (ch == ' ') {
                // Process accumulated hex pair if we have one
                if (!hexPair.empty()) {
                    if (hexPair.length() != 2) {
                        throw std::invalid_argument("Incomplete hex pair in string");
                    }
                    result.push_back(stringToOctet(hexPair));
                    hexPair.clear();
                }
                continue;
            }

            // Validate hex character
            if (!((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F'))) {
                throw std::invalid_argument("Invalid character in hex string (must be 0-9 or A-F)");
            }

            hexPair += ch;

            // Process pair when we have 2 characters
            if (hexPair.length() == 2) {
                result.push_back(stringToOctet(hexPair));
                hexPair.clear();
            }
        }

        // Process any remaining hex pair at end
        if (!hexPair.empty()) {
            if (hexPair.length() != 2) {
                throw std::invalid_argument("Incomplete hex pair at end of string");
            }
            result.push_back(stringToOctet(hexPair));
        }

        return result;
    }

} // namespace hermes

