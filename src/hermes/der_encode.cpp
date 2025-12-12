#include "thumper/hermes/der_encode.hpp"

#include <array>
#include <cassert>

namespace thumper::hermes::der {

// NOLINTBEGIN(readability-identifier-naming,cppcoreguidelines-pro-bounds-constant-array-index)
// Local lookup tables use snake_case for consistency with standard practice

Octet make_leading_identifier(TagClass tag_class, StructuredFlag structured_flag,
                              LeadingNumberType tag) {
  // Lookup tables for bit patterns corresponding to each enum value.
  // These match the Ada implementation in hermes-der-encode.adb.

  // Tag class occupies bits 7-6
  constexpr std::array<Octet, 4> kTagClassLookup = {
      0b00000000, // Universal
      0b01000000, // Application
      0b10000000, // ContextSpecific
      0b11000000  // Private
  };

  // Structured flag occupies bit 5
  constexpr std::array<Octet, 2> kStructuredFlagLookup = {
      0b00000000, // Primitive
      0b00100000  // Constructed
  };

  // Leading number occupies bits 4-0
  // Note: This uses the underlying numeric value of the LeadingNumberType enum,
  // which is already defined to match ASN.1 tag numbers.
  const auto tag_number = static_cast<Octet>(tag);

  // Combine all three fields using bitwise OR
  return kTagClassLookup[static_cast<std::size_t>(tag_class)] |
         kStructuredFlagLookup[static_cast<std::size_t>(structured_flag)] | tag_number;
}

OctetArray put_length_value(std::size_t length) {
  OctetArray result;

  // Short form: 0-127 encoded as single octet
  if (length <= 127) {
    result.push_back(static_cast<Octet>(length));
    return result;
  }

  // Long form: First octet = 0x80 | number_of_length_octets,
  // followed by length in big-endian format

  if (length <= 0xFF) {
    // 1 length octet
    result.push_back(0b10000001);
    result.push_back(static_cast<Octet>(length));
  } else if (length <= 0xFFFF) {
    // 2 length octets
    result.push_back(0b10000010);
    result.push_back(static_cast<Octet>((length >> 8) & 0xFF));
    result.push_back(static_cast<Octet>(length & 0xFF));
  } else if (length <= 0xFFFFFF) {
    // 3 length octets
    result.push_back(0b10000011);
    result.push_back(static_cast<Octet>((length >> 16) & 0xFF));
    result.push_back(static_cast<Octet>((length >> 8) & 0xFF));
    result.push_back(static_cast<Octet>(length & 0xFF));
  } else {
    // 4 length octets (handles up to 2^32-1)
    result.push_back(0b10000100);
    result.push_back(static_cast<Octet>((length >> 24) & 0xFF));
    result.push_back(static_cast<Octet>((length >> 16) & 0xFF));
    result.push_back(static_cast<Octet>((length >> 8) & 0xFF));
    result.push_back(static_cast<Octet>(length & 0xFF));
  }

  return result;
}

OctetArray put_boolean_value(bool value) {
  OctetArray result;

  // Identifier: Universal, Primitive, Boolean
  result.push_back(make_leading_identifier(TagClass::Universal, StructuredFlag::Primitive,
                                           LeadingNumberType::TagBoolean));

  // Length: Always 1 octet for Boolean
  result.push_back(0x01);

  // Value: 0xFF for true, 0x00 for false (DER specification)
  result.push_back(value ? 0xFF : 0x00);

  return result;
}

// NOLINTBEGIN(bugprone-branch-clone)
// Different branches encode different byte lengths - clones are intentional
OctetArray put_integer_value(int value) {
  OctetArray result;

  // Identifier: Universal, Primitive, Integer
  result.push_back(make_leading_identifier(TagClass::Universal, StructuredFlag::Primitive,
                                           LeadingNumberType::TagInteger));

  // Determine the minimum number of octets needed to encode the value.
  // DER requires the most significant bit of the first content octet to represent
  // the sign, and no unnecessary leading 0x00 or 0xFF octets.

  if (value >= 0 && value <= 0x7F) {
    // 1 octet (0x00 to 0x7F)
    result.push_back(0x01); // Length
    result.push_back(static_cast<Octet>(value));
  } else if (value >= 0 && value <= 0x7FFF) {
    // 2 octets (0x0080 to 0x7FFF)
    result.push_back(0x02); // Length
    result.push_back(static_cast<Octet>((value >> 8) & 0xFF));
    result.push_back(static_cast<Octet>(value & 0xFF));
  } else if (value >= 0 && value <= 0x7FFFFF) {
    // 3 octets (0x008000 to 0x7FFFFF)
    result.push_back(0x03); // Length
    result.push_back(static_cast<Octet>((value >> 16) & 0xFF));
    result.push_back(static_cast<Octet>((value >> 8) & 0xFF));
    result.push_back(static_cast<Octet>(value & 0xFF));
  } else if (value >= 0) {
    // 4 octets (0x00800000 to 0x7FFFFFFF)
    result.push_back(0x04); // Length
    result.push_back(static_cast<Octet>((value >> 24) & 0xFF));
    result.push_back(static_cast<Octet>((value >> 16) & 0xFF));
    result.push_back(static_cast<Octet>((value >> 8) & 0xFF));
    result.push_back(static_cast<Octet>(value & 0xFF));
  } else if (value >= -0x80) {
    // Negative value, 1 octet (-128 to -1)
    result.push_back(0x01); // Length
    result.push_back(static_cast<Octet>(value & 0xFF));
  } else if (value >= -0x8000) {
    // Negative value, 2 octets (-32768 to -129)
    result.push_back(0x02); // Length
    result.push_back(static_cast<Octet>((value >> 8) & 0xFF));
    result.push_back(static_cast<Octet>(value & 0xFF));
  } else if (value >= -0x800000) {
    // Negative value, 3 octets (-8388608 to -32769)
    result.push_back(0x03); // Length
    result.push_back(static_cast<Octet>((value >> 16) & 0xFF));
    result.push_back(static_cast<Octet>((value >> 8) & 0xFF));
    result.push_back(static_cast<Octet>(value & 0xFF));
  } else {
    // Negative value, 4 octets (-2147483648 to -8388609)
    result.push_back(0x04); // Length
    result.push_back(static_cast<Octet>((value >> 24) & 0xFF));
    result.push_back(static_cast<Octet>((value >> 16) & 0xFF));
    result.push_back(static_cast<Octet>((value >> 8) & 0xFF));
    result.push_back(static_cast<Octet>(value & 0xFF));
  }

  return result;
}

OctetArray put_octet_string_value(std::span<const Octet> value) {
  OctetArray result;

  // Identifier: Universal, Primitive, OctetString
  result.push_back(make_leading_identifier(TagClass::Universal, StructuredFlag::Primitive,
                                           LeadingNumberType::TagOctetString));

  // Length
  OctetArray length_encoding = put_length_value(value.size());
  result.insert(result.end(), length_encoding.begin(), length_encoding.end());

  // Value
  result.insert(result.end(), value.begin(), value.end());

  return result;
}

OctetArray put_null_value() {
  OctetArray result;

  // Identifier: Universal, Primitive, Null
  result.push_back(make_leading_identifier(TagClass::Universal, StructuredFlag::Primitive,
                                           LeadingNumberType::TagNull));

  // Length: Always 0 for NULL
  result.push_back(0x00);

  return result;
}
// NOLINTEND(bugprone-branch-clone)

// NOLINTEND(readability-identifier-naming,cppcoreguidelines-pro-bounds-constant-array-index)

} // namespace thumper::hermes::der
