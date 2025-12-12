#ifndef THUMPER_HERMES_DER_DECODE_HPP
#define THUMPER_HERMES_DER_DECODE_HPP

#include <cstddef>
#include <span>

#include "thumper/hermes/der.hpp"
#include "thumper/types.hpp"

namespace thumper::hermes::der {

// Splits a leading identifier octet into its constituent parts.
//
// This function parses the first octet of a DER-encoded value (the identifier/tag octet)
// and extracts the tag class, structured flag, and tag number.
//
// The function fails with Status::BadIdentifier if the value is invalid (e.g., tag
// numbers 14 or 15 which are undefined in the universal class).
//
// This function corresponds to Split_Leading_Identifier in hermes-der-decode.ads.
//
// @param value The identifier octet to parse
// @param tag_class Output: The extracted tag class
// @param structured_flag Output: Whether the type is primitive or constructed
// @param tag Output: The extracted tag number
// @param status Output: Success or BadIdentifier
void split_leading_identifier(Octet value, TagClass& tag_class, StructuredFlag& structured_flag,
                              LeadingNumberType& tag, Status& status);

// Decodes a DER-encoded length from a message.
//
// DER length encoding:
// - Short form (0-127): Single octet with bit 7 = 0, bits 6-0 = length
// - Long form (128+): First octet = 0x80 | num_octets, followed by length in big-endian
// - Indefinite form (0x80): Not allowed in DER, returns Status::IndefiniteLength
//
// On success:
// - stop is set to the index of the last octet of the encoded length
// - length contains the decoded length value
//
// On failure:
// - Status::IndefiniteLength: Indefinite form encountered (length = 0, stop = start)
// - Status::BadLength: Malformed encoding (length = 0, stop undefined)
// - Status::UnimplementedLength: Length too large (length = 0, stop = last octet of
// length)
//
// This function corresponds to Get_Length_Value in hermes-der-decode.ads.
//
// @param message The message buffer to decode from
// @param start The starting index where the length encoding begins
// @param stop Output: The index of the last octet of the encoded length
// @param length Output: The decoded length value
// @param status Output: Success or error status
void get_length_value(std::span<const Octet> message, std::size_t start, std::size_t& stop,
                      std::size_t& length, Status& status);

// Decodes a DER-encoded Boolean value from a message.
//
// DER Boolean encoding:
// - Identifier: 0x01 (Universal, Primitive, Boolean)
// - Length: 0x01
// - Value: 0xFF (true) or 0x00 (false)
//
// On success:
// - stop is set to the index of the last octet of the encoded Boolean
// - value contains the decoded boolean
//
// On failure (Status::BadValue):
// - Invalid identifier, length, or value encoding
// - value is set to false, stop may be undefined
//
// This function corresponds to Get_Boolean_Value in hermes-der-decode.ads.
//
// @param message The message buffer to decode from
// @param start The starting index of the Boolean encoding
// @param stop Output: The index of the last octet of the encoded Boolean
// @param value Output: The decoded boolean value
// @param status Output: Success or BadValue
void get_boolean_value(std::span<const Octet> message, std::size_t start, std::size_t& stop,
                       bool& value, Status& status);

// Decodes a DER-encoded integer value from a message.
//
// DER Integer encoding uses two's complement representation with minimal octets.
// The most significant bit of the first content octet is the sign bit.
//
// On success:
// - stop is set to the index of the last octet of the encoded integer
// - value contains the decoded integer
//
// On failure:
// - Status::BadValue: Invalid identifier, length, or value encoding
// - Status::UnimplementedValue: Integer too large for 32-bit int
// - value is set to 0, stop may be undefined
//
// This function corresponds to Get_Integer_Value in hermes-der-decode.ads.
//
// @param message The message buffer to decode from
// @param start The starting index of the integer encoding
// @param stop Output: The index of the last octet of the encoded integer
// @param value Output: The decoded integer value
// @param status Output: Success, BadValue, or UnimplementedValue
void get_integer_value(std::span<const Octet> message, std::size_t start, std::size_t& stop,
                       int& value, Status& status);

// Note: get_oid_value is intentionally NOT included here.
// Per the task specification, OID decoding will be co-migrated with the OID type
// in Task 4 to ensure API boundary correctness.

} // namespace thumper::hermes::der

#endif // THUMPER_HERMES_DER_DECODE_HPP
