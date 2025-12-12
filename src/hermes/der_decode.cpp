#include "thumper/hermes/der_decode.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>

namespace thumper::hermes::der {

// NOLINTBEGIN(readability-identifier-naming,cppcoreguidelines-pro-bounds-constant-array-index)
// Local lookup tables and variables use snake_case for consistency with standard practice

void split_leading_identifier(Octet value, TagClass& tag_class, StructuredFlag& structured_flag,
                              LeadingNumberType& tag, Status& status) {
  status = Status::Success;

  // Extract tag class from bits 7-6
  switch (value & 0b11000000) {
  case 0b00000000:
    tag_class = TagClass::Universal;
    break;
  case 0b01000000:
    tag_class = TagClass::Application;
    break;
  case 0b10000000:
    tag_class = TagClass::ContextSpecific;
    break;
  case 0b11000000:
    tag_class = TagClass::Private;
    break;
  default:
    // Should never happen due to exhaustive case coverage
    tag_class = TagClass::Universal;
    break;
  }

  // Extract structured flag from bit 5
  switch (value & 0b00100000) {
  case 0b00000000:
    structured_flag = StructuredFlag::Primitive;
    break;
  case 0b00100000:
    structured_flag = StructuredFlag::Constructed;
    break;
  default:
    // Should never happen
    structured_flag = StructuredFlag::Primitive;
    break;
  }

  // Extract tag number from bits 4-0
  const auto tag_value = static_cast<Octet>(value & 0b00011111);

  // Tag numbers 14 and 15 are undefined in ASN.1
  if (tag_value == 14 || tag_value == 15) {
    status = Status::BadIdentifier;
  }

  // Lookup table for converting tag value to LeadingNumberType
  // This matches the Ada implementation in hermes-der-decode.adb
  constexpr std::array<LeadingNumberType, 32> kLeadingNumberLookup = {
      LeadingNumberType::TagReserved,         // 0
      LeadingNumberType::TagBoolean,          // 1
      LeadingNumberType::TagInteger,          // 2
      LeadingNumberType::TagBitString,        // 3
      LeadingNumberType::TagOctetString,      // 4
      LeadingNumberType::TagNull,             // 5
      LeadingNumberType::TagObjectIdentifier, // 6
      LeadingNumberType::TagObjectDescriptor, // 7
      LeadingNumberType::TagInstanceOf,       // 8
      LeadingNumberType::TagReal,             // 9
      LeadingNumberType::TagEnumerated,       // 10
      LeadingNumberType::TagEmbeddedPDV,      // 11
      LeadingNumberType::TagUTF8String,       // 12
      LeadingNumberType::TagRelativeOID,      // 13
      LeadingNumberType::TagNull,             // 14 (undefined)
      LeadingNumberType::TagNull,             // 15 (undefined)
      LeadingNumberType::TagSequence,         // 16
      LeadingNumberType::TagSet,              // 17
      LeadingNumberType::TagNumericString,    // 18
      LeadingNumberType::TagPrintableString,  // 19
      LeadingNumberType::TagTeletexString,    // 20
      LeadingNumberType::TagVideotexString,   // 21
      LeadingNumberType::TagIA5String,        // 22
      LeadingNumberType::TagUTCTime,          // 23
      LeadingNumberType::TagGeneralizedTime,  // 24
      LeadingNumberType::TagGraphicString,    // 25
      LeadingNumberType::TagVisibleString,    // 26
      LeadingNumberType::TagGeneralString,    // 27
      LeadingNumberType::TagUniversalString,  // 28
      LeadingNumberType::TagCharacterString,  // 29
      LeadingNumberType::TagBMPString,        // 30
      LeadingNumberType::TagExtendedTag       // 31
  };

  tag = kLeadingNumberLookup[tag_value];
}

void get_length_value(std::span<const Octet> message, std::size_t start, std::size_t& stop,
                      std::size_t& length, Status& status) {
  // Check preconditions
  assert(start < message.size());

  // Check for indefinite length (0x80)
  if (message[start] == 0b10000000) {
    stop = start;
    length = 0;
    status = Status::IndefiniteLength;
    return;
  }

  // Check for definite length, short form (bit 7 = 0)
  if ((message[start] & 0b10000000) == 0b00000000) {
    stop = start;
    length = static_cast<std::size_t>(message[start]);
    status = Status::Success;
    return;
  }

  // Check for definite length, long form, reserved value (0xFF)
  if (message[start] == 0b11111111) {
    stop = start;
    length = 0;
    status = Status::BadLength;
    return;
  }

  // We have definite length, long form, normal value
  // First octet = 0x80 | number_of_length_octets
  const auto length_octets = static_cast<std::size_t>(message[start] & 0b01111111);

  // Check that all length octets are in the array
  // We need start + length_octets to be within bounds
  if (start + length_octets >= message.size()) {
    stop = message.size() - 1;
    length = 0;
    status = Status::BadLength;
    return;
  }

  // Check that the value of the length is not too large
  // (assuming 64-bit size_t, we support up to 4 bytes of length encoding)
  if (length_octets > 4 || (length_octets == 4 && message[start + 1] >= 128)) {
    stop = start + length_octets;
    length = 0;
    status = Status::UnimplementedLength;
    return;
  }

  // Convert the length into a single size_t
  stop = start + length_octets;
  length = 0;
  for (std::size_t i = 1; i <= length_octets; ++i) {
    length = (length * 256) + static_cast<std::size_t>(message[start + i]);
  }
  status = Status::Success;
}

void get_boolean_value(std::span<const Octet> message, std::size_t start, std::size_t& stop,
                       bool& value, Status& status) {
  // Check preconditions
  assert(start < message.size());

  // Parse and validate the identifier octet
  TagClass tag_class = TagClass::Universal;
  StructuredFlag structured_flag = StructuredFlag::Primitive;
  LeadingNumberType tag = LeadingNumberType::TagBoolean;
  Status identifier_status = Status::Success;

  split_leading_identifier(message[start], tag_class, structured_flag, tag, identifier_status);

  // Validate that this is a proper Boolean identifier
  if (identifier_status != Status::Success || tag_class != TagClass::Universal ||
      structured_flag != StructuredFlag::Primitive || tag != LeadingNumberType::TagBoolean) {
    stop = start;
    value = false;
    status = Status::BadValue;
    return;
  }

  // Check that we have room for at least the length octet
  if (start >= message.size() - 1) {
    stop = start;
    value = false;
    status = Status::BadValue;
    return;
  }

  // Parse the length
  std::size_t length_stop = 0;
  std::size_t length = 0;
  Status length_status = Status::Success;

  get_length_value(message, start + 1, length_stop, length, length_status);

  if (length_status != Status::Success) {
    // We couldn't decode the length
    stop = length_stop;
    value = false;
    status = Status::BadValue;
    return;
  }

  if (length_stop >= message.size() - length) {
    // The value goes off the end of the message
    stop = message.size() - 1;
    value = false;
    status = Status::BadValue;
    return;
  }

  if (length != 1) {
    // Boolean values must have a length of exactly one
    stop = length_stop + length;
    value = false;
    status = Status::BadValue;
    return;
  }

  // Extract the Boolean value
  stop = length_stop + length;
  const auto value_octet = message[length_stop + 1];

  if (value_octet == 0xFF) {
    value = true;
    status = Status::Success;
  } else if (value_octet == 0x00) {
    value = false;
    status = Status::Success;
  } else {
    // Invalid Boolean encoding (DER requires 0xFF or 0x00)
    value = false;
    status = Status::BadValue;
  }
}

void get_integer_value(std::span<const Octet> message, std::size_t start, std::size_t& stop,
                       int& value, Status& status) {
  // Check preconditions
  assert(start < message.size());

  // Parse and validate the identifier octet
  TagClass tag_class = TagClass::Universal;
  StructuredFlag structured_flag = StructuredFlag::Primitive;
  LeadingNumberType tag = LeadingNumberType::TagInteger;
  Status identifier_status = Status::Success;

  split_leading_identifier(message[start], tag_class, structured_flag, tag, identifier_status);

  // Validate that this is a proper Integer identifier
  if (identifier_status != Status::Success || tag_class != TagClass::Universal ||
      structured_flag != StructuredFlag::Primitive || tag != LeadingNumberType::TagInteger) {
    stop = start;
    value = 0;
    status = Status::BadValue;
    return;
  }

  // Check that we have room for at least the length octet
  if (start >= message.size() - 1) {
    stop = start;
    value = 0;
    status = Status::BadValue;
    return;
  }

  // Parse the length
  std::size_t length_stop = 0;
  std::size_t length = 0;
  Status length_status = Status::Success;

  get_length_value(message, start + 1, length_stop, length, length_status);

  if (length_status != Status::Success) {
    // We couldn't decode the length
    stop = length_stop;
    value = 0;
    status = Status::BadValue;
    return;
  }

  if (length_stop >= message.size() - length) {
    // The value goes off the end of the message
    stop = message.size() - 1;
    value = 0;
    status = Status::BadValue;
    return;
  }

  if (length > 4) {
    // The length implies too large a value for 32-bit int
    stop = length_stop + length;
    value = 0;
    status = Status::UnimplementedValue;
    return;
  }

  // Check for unnecessary leading zeros (DER violation)
  if (length >= 2 && message[length_stop + 1] == 0x00 &&
      (message[length_stop + 2] & 0x80) == 0x00) {
    stop = length_stop + length;
    value = 0;
    status = Status::BadValue;
    return;
  }

  // Check for unnecessary leading ones (DER violation)
  if (length >= 2 && message[length_stop + 1] == 0xFF &&
      (message[length_stop + 2] | 0x7F) == 0xFF) {
    stop = length_stop + length;
    value = 0;
    status = Status::BadValue;
    return;
  }

  // Extract the integer value
  stop = length_stop + length;
  int result = 0;

  // Check if the value is positive (MSB = 0) or negative (MSB = 1)
  if ((message[length_stop + 1] & 0x80) == 0) {
    // Positive value
    for (std::size_t i = 1; i <= length; ++i) {
      result = 256 * result + static_cast<int>(message[length_stop + i]);
    }
    value = result;
  } else {
    // Negative value: use two's complement
    // First, invert all bits and accumulate
    for (std::size_t i = 1; i <= length; ++i) {
      result = 256 * result + static_cast<int>(message[length_stop + i] ^ 0xFF);
    }

    // Handle INT_MIN as a special case due to two's complement asymmetry
    if (result == std::numeric_limits<int>::max()) {
      value = std::numeric_limits<int>::min();
    } else {
      value = -(result + 1);
    }
  }

  status = Status::Success;
}

// NOLINTEND(readability-identifier-naming,cppcoreguidelines-pro-bounds-constant-array-index)

} // namespace thumper::hermes::der
