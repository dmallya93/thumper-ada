///////////////////////////////////////////////////////////////////////////
// FILE    : der_encode.hpp
// SUBJECT : Specification of DER encoding functions
// AUTHOR  : (C) Copyright 2022 by Peter Chapin
//
// This header provides functions for encoding ASN.1 primitive values into
// DER (Distinguished Encoding Rules) format. DER is a canonical encoding
// that produces exactly one byte sequence for each value - critical for
// digital signatures.
//
// Maps from Ada package: Hermes.DER.Encode (hermes-der-encode.ads/adb)
//
// DER encoding follows the TLV (Tag-Length-Value) structure:
// - Tag: Identifies the type (1 byte for short form)
// - Length: Encodes the length of the value (1-5 bytes)
// - Value: The actual data being encoded
//
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "hermes.hpp"
#include "der.hpp"
#include "oid.hpp"

namespace hermes {
namespace der {
namespace encode {

    /// Constructs an identifier octet from its constituent parts
    ///
    /// Maps from Ada:
    /// ```ada
    /// function Make_Leading_Identifier
    ///   (Tag_Class       : Tag_Class_Type;
    ///    Structured_Flag : Structured_Flag_Type;
    ///    Tag             : Leading_Number_Type) return Octet;
    /// ```
    ///
    /// DER tag octet format (1 byte):
    /// - Bits 7-6: Tag class (00=Universal, 01=Application, 10=Context, 11=Private)
    /// - Bit 5: Structured flag (0=Primitive, 1=Constructed)
    /// - Bits 4-0: Tag number (0-30 for short form, 31 indicates extended form)
    ///
    /// @param tagClass The tag class
    /// @param structuredFlag Whether the value is primitive or constructed
    /// @param tag The tag number
    /// @return The constructed tag byte
    Octet makeLeadingIdentifier(
        TagClass tagClass,
        StructuredFlag structuredFlag,
        LeadingNumber tag);

    /// Returns the DER encoded length
    ///
    /// Maps from Ada:
    /// ```ada
    /// function Put_Length_Value(Length : Natural) return Hermes.Octet_Array
    ///   with Post => Put_Length_Value'Result'Length in 1 .. 5;
    /// ```
    ///
    /// DER length encoding:
    /// - 0-127: Single byte (short form) - the value itself
    /// - 128-255: Two bytes - 0x81 followed by value
    /// - 256-65535: Three bytes - 0x82 followed by value in big-endian
    /// - 65536-16777215: Four bytes - 0x83 followed by value in big-endian
    /// - 16777216+: Five bytes - 0x84 followed by value in big-endian
    ///
    /// @param length The length value to encode
    /// @return DER encoded length (1-5 bytes)
    OctetArray putLengthValue(std::uint32_t length);

    /// Returns the DER encoded TLV triple of a Boolean value
    ///
    /// Maps from Ada:
    /// ```ada
    /// function Put_Boolean_Value(Value : Boolean) return Hermes.Octet_Array
    ///   with Post => Put_Boolean_Value'Result'Length = 3;
    /// ```
    ///
    /// DER Boolean encoding:
    /// - Tag: 0x01 (Universal, Primitive, Boolean)
    /// - Length: 0x01 (1 byte)
    /// - Value: 0xFF (true) or 0x00 (false)
    ///
    /// @param value The boolean value to encode
    /// @return 3-byte DER encoded boolean
    OctetArray putBooleanValue(bool value);

    /// Returns the DER encoded TLV triple of an integer value
    ///
    /// Maps from Ada:
    /// ```ada
    /// function Put_Integer_Value(Value : Integer) return Hermes.Octet_Array;
    /// ```
    ///
    /// DER Integer encoding uses minimum number of bytes:
    /// - Tag: 0x02 (Universal, Primitive, Integer)
    /// - Length: 1-4 bytes depending on value magnitude
    /// - Value: Big-endian two's complement representation
    ///   - If high bit of first content byte is 1, prepend 0x00 to distinguish from negative
    ///
    /// Current implementation handles non-negative integers only.
    ///
    /// @param value The integer value to encode (must be non-negative)
    /// @return DER encoded integer (3-6 bytes)
    OctetArray putIntegerValue(std::int32_t value);

    /// Returns the DER encoded TLV triple of an octet string
    ///
    /// Maps from Ada:
    /// ```ada
    /// function Put_Octet_String_Value(Value : Hermes.Octet_Array) return Hermes.Octet_Array;
    /// ```
    ///
    /// DER Octet String encoding:
    /// - Tag: 0x04 (Universal, Primitive, OctetString)
    /// - Length: DER encoded length of the value
    /// - Value: The octets themselves (unmodified)
    ///
    /// @param value The octet array to encode
    /// @return DER encoded octet string
    OctetArray putOctetStringValue(const OctetArray& value);

    /// Returns the DER encoded TLV triple of a null value
    ///
    /// Maps from Ada:
    /// ```ada
    /// function Put_Null_Value return Hermes.Octet_Array;
    /// ```
    ///
    /// DER Null encoding:
    /// - Tag: 0x05 (Universal, Primitive, Null)
    /// - Length: 0x00 (0 bytes)
    /// - Value: (none)
    ///
    /// @return 2-byte DER encoded null
    OctetArray putNullValue();

    /// Returns the DER encoded TLV triple of an object identifier value
    ///
    /// Maps from Ada:
    /// ```ada
    /// function Put_OID_Value(Value : Hermes.OID.Object_Identifier) return Hermes.Octet_Array;
    /// ```
    ///
    /// DER OID encoding (complex):
    /// - Tag: 0x06 (Universal, Primitive, ObjectIdentifier)
    /// - Length: DER encoded length of the encoded component bytes
    /// - Value: Encoded components:
    ///   - First byte: 40*first_component + second_component
    ///   - Remaining components: Each encoded in base-128 with continuation bits
    ///     - Split into 7-bit chunks
    ///     - MSB set to 1 for all but last chunk
    ///     - Big-endian order (most significant chunk first)
    ///
    /// Example: OID 2.16.840 encodes as:
    /// - First byte: 40*2 + 16 = 96 (0x60)
    /// - 840 = 0x348 = 0b110_1001000 -> [0x86, 0x48] (0b1000_0110, 0b0100_1000)
    ///
    /// @param value The OID to encode
    /// @return DER encoded OID
    OctetArray putOIDValue(const oid::ObjectIdentifier& value);

} // namespace encode
} // namespace der
} // namespace hermes
