#ifndef THUMPER_HERMES_DER_HPP
#define THUMPER_HERMES_DER_HPP

#include <cstdint>

namespace thumper::hermes::der {

// Status codes returned by DER encoding/decoding operations.
// These match the Ada Status_Type enumeration from hermes-der.ads.
enum class Status {
  Success,             // Operation completed successfully
  BadIdentifier,       // Invalid identifier octet (e.g., undefined tag 14/15)
  IndefiniteLength,    // Indefinite length encoding (not supported in DER)
  UnimplementedLength, // Length too large for this implementation
  BadLength,           // Malformed length encoding
  UnimplementedValue,  // Value too large for this implementation
  BadValue             // Malformed value encoding
};

// ASN.1 tag class.
// Corresponds to Ada's Tag_Class_Type from hermes-der.ads.
enum class TagClass {
  Universal,       // 0b00 - Standard ASN.1 types
  Application,     // 0b01 - Application-specific types
  ContextSpecific, // 0b10 - Context-specific types (most common in protocols)
  Private          // 0b11 - Private types
};

// ASN.1 structured flag.
// Indicates whether a type is primitive (atomic) or constructed (contains other types).
// Corresponds to Ada's Structured_Flag_Type from hermes-der.ads.
enum class StructuredFlag {
  Primitive,  // 0b0 - Simple types (INTEGER, BOOLEAN, etc.)
  Constructed // 0b1 - Container types (SEQUENCE, SET, etc.)
};

// ASN.1 universal tag numbers.
// Corresponds to Ada's Leading_Number_Type from hermes-der.ads.
// Note: Not all ASN.1 tags are implemented in this migration;
// only those actually used by the Thumper timestamp protocol.
enum class LeadingNumberType : std::uint8_t {
  TagReserved = 0,
  TagBoolean = 1,
  TagInteger = 2,
  TagBitString = 3,
  TagOctetString = 4,
  TagNull = 5,
  TagObjectIdentifier = 6,
  TagObjectDescriptor = 7,
  TagInstanceOf = 8,
  TagExternal = 8, // Same as InstanceOf
  TagReal = 9,
  TagEnumerated = 10,
  TagEmbeddedPDV = 11,
  TagUTF8String = 12,
  TagRelativeOID = 13,
  // 14 and 15 are undefined/reserved
  TagSequence = 16,
  TagSequenceOf = 16, // Same as Sequence
  TagSet = 17,
  TagSetOf = 17, // Same as Set
  TagNumericString = 18,
  TagPrintableString = 19,
  TagTeletexString = 20,
  TagT61String = 20, // Same as TeletexString
  TagVideotexString = 21,
  TagIA5String = 22,
  TagUTCTime = 23,
  TagGeneralizedTime = 24,
  TagGraphicString = 25,
  TagVisibleString = 26,
  TagISO646String = 26, // Same as VisibleString
  TagGeneralString = 27,
  TagUniversalString = 28,
  TagCharacterString = 29,
  TagBMPString = 30,
  TagExtendedTag = 31
};

} // namespace thumper::hermes::der

#endif // THUMPER_HERMES_DER_HPP
