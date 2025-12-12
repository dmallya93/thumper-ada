#ifndef THUMPER_HERMES_DER_ENCODE_HPP
#define THUMPER_HERMES_DER_ENCODE_HPP

#include <span>

#include "thumper/hermes/der.hpp"
#include "thumper/hermes/oid.hpp"
#include "thumper/types.hpp"

namespace thumper::hermes::der {

// Constructs an identifier octet from its constituent parts.
//
// DER identifier octets have the following structure:
// - Bits 7-6: Tag class (Universal, Application, ContextSpecific, Private)
// - Bit 5: Structured flag (Primitive or Constructed)
// - Bits 4-0: Tag number
//
// This function corresponds to Make_Leading_Identifier in hermes-der-encode.ads.
//
// @param tag_class The tag class for this identifier
// @param structured_flag Whether the type is primitive or constructed
// @param tag The tag number (must be < 31 for single-octet form)
// @return The constructed identifier octet
Octet make_leading_identifier(TagClass tag_class, StructuredFlag structured_flag,
                              LeadingNumberType tag);

// Encodes a length value in DER format.
//
// DER length encoding has two forms:
// - Short form (0-127): Single octet with bit 7 = 0
// - Long form (128+): First octet = 0x80 | num_bytes, followed by length in big-endian
//
// This function corresponds to Put_Length_Value in hermes-der-encode.ads.
//
// @param length The length value to encode (must be non-negative)
// @return An OctetArray containing the encoded length (1-5 octets)
OctetArray put_length_value(std::size_t length);

// Encodes a Boolean value in DER TLV format.
//
// DER Boolean encoding:
// - Tag: 0x01 (Universal, Primitive, Boolean)
// - Length: 0x01 (always 1 octet)
// - Value: 0xFF (true) or 0x00 (false)
//
// This function corresponds to Put_Boolean_Value in hermes-der-encode.ads.
//
// @param value The boolean value to encode
// @return An OctetArray containing the TLV-encoded Boolean (3 octets)
OctetArray put_boolean_value(bool value);

// Encodes an integer value in DER TLV format.
//
// DER Integer encoding uses the minimum number of octets needed to represent
// the value in two's complement form with a sign bit. The most significant bit
// of the first content octet determines the sign (0 = positive, 1 = negative).
//
// This function corresponds to Put_Integer_Value in hermes-der-encode.ads.
//
// @param value The integer value to encode
// @return An OctetArray containing the TLV-encoded integer (3-6 octets)
OctetArray put_integer_value(int value);

// Encodes an octet string in DER TLV format.
//
// DER Octet String encoding:
// - Tag: 0x04 (Universal, Primitive, OctetString)
// - Length: Number of octets in the value
// - Value: Raw octet data
//
// This function corresponds to Put_Octet_String_Value in hermes-der-encode.ads.
//
// @param value The octet array to encode
// @return An OctetArray containing the TLV-encoded octet string
OctetArray put_octet_string_value(std::span<const Octet> value);

// Encodes a NULL value in DER TLV format.
//
// DER NULL encoding:
// - Tag: 0x05 (Universal, Primitive, Null)
// - Length: 0x00 (always 0 octets)
//
// This function corresponds to Put_Null_Value in hermes-der-encode.ads.
//
// @return An OctetArray containing the TLV-encoded NULL (2 octets)
OctetArray put_null_value();

// Encodes an Object Identifier (OID) in DER TLV format.
//
// DER OID encoding:
// - Tag: 0x06 (Universal, Primitive, ObjectIdentifier)
// - Length: Number of octets in the encoded OID value
// - Value: Encoded OID components using base-128 encoding
//
// The first two components are combined: first_octet = (component[0] * 40) + component[1]
// Subsequent components are encoded in base-128 with MSB continuation bits.
//
// This function corresponds to Put_OID_Value in hermes-der-encode.adb.
//
// @param value The object identifier to encode
// @return An OctetArray containing the TLV-encoded OID
OctetArray put_oid_value(const thumper::hermes::oid::ObjectIdentifier& value);

} // namespace thumper::hermes::der

#endif // THUMPER_HERMES_DER_ENCODE_HPP
