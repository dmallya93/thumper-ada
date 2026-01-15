///////////////////////////////////////////////////////////////////////////
// FILE    : der_decode.hpp
// SUBJECT : DER (Distinguished Encoding Rules) decoding functions
// AUTHOR  : (C) Copyright 2022 by Peter Chapin
//
// This header provides functions to decode ASN.1 DER-encoded data back
// into primitive values. DER is a canonical subset of BER (Basic Encoding
// Rules) used in cryptographic protocols like RFC-3161.
//
// The decoding functions perform extensive validation to reject non-canonical
// DER encodings and malformed data. This is critical for security as it
// prevents ambiguous message parsing that could lead to signature forgery.
//
// Maps from Ada package: Hermes.DER.Decode (hermes-der-decode.ads/adb)
//
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "hermes/hermes.hpp"
#include "hermes/der.hpp"
#include "hermes/oid.hpp"
#include <cstddef>

namespace hermes {
namespace der {
namespace decode {

    /// Splits a leading identifier octet into its constituent parts
    ///
    /// Maps from Ada:
    /// ```ada
    /// procedure Split_Leading_Identifier
    ///   (Value           : in  Octet;
    ///    Tag_Class       : out Tag_Class_Type;
    ///    Structured_Flag : out Structured_Flag_Type;
    ///    Tag             : out Leading_Number_Type;
    ///    Status          : out Status_Type)
    ///   with Depends => ( (Tag_Class, Structured_Flag, Tag, Status) => Value);
    /// ```
    ///
    /// The identifier octet encodes:
    /// - Bits 7-6: Tag class (Universal, Application, ContextSpecific, Private)
    /// - Bit 5: Structured flag (Primitive or Constructed)
    /// - Bits 4-0: Tag number (0-30 for standard tags, 31 indicates long form)
    ///
    /// Fails with Status::BadIdentifier if the value is an invalid first
    /// identifier octet (tag numbers 14 and 15 are undefined in ASN.1).
    ///
    /// @param value The identifier octet to parse
    /// @param tagClass Output: extracted tag class
    /// @param structuredFlag Output: extracted structured flag
    /// @param tag Output: extracted tag number
    /// @return Status code (Success or BadIdentifier)
    Status splitLeadingIdentifier(
        Octet value,
        TagClass& tagClass,
        StructuredFlag& structuredFlag,
        LeadingNumber& tag);

    /// Decodes a DER-encoded length from a message
    ///
    /// Maps from Ada:
    /// ```ada
    /// procedure Get_Length_Value
    ///   (Message : in  Octet_Array;
    ///    Start   : in  Natural;
    ///    Stop    : out Natural;
    ///    Length  : out Natural;
    ///    Status  : out Status_Type)
    ///   with
    ///     Depends => ( (Stop, Length, Status) => (Message, Start) ),
    ///     Pre => Start in Message'Range,
    ///     Post => Stop in Start .. Message'Last;
    /// ```
    ///
    /// DER length encoding has three forms:
    /// - Short form: High bit clear (0-127), length is the byte value itself
    /// - Indefinite form: 0x80 (NOT ALLOWED in DER, returns IndefiniteLength)
    /// - Long form: High bit set, low 7 bits = number of length bytes that follow
    ///   - Reserved value 0xFF returns BadLength
    ///   - Limited to 4 bytes (32-bit Natural) - longer returns UnimplementedLength
    ///
    /// @param message The message to examine
    /// @param start Starting position in message where length encoding begins
    /// @param stop Output: last position used by the encoded length
    /// @param length Output: the decoded length value
    /// @return Status code (Success, IndefiniteLength, BadLength, or UnimplementedLength)
    ///
    /// Precondition: start must be a valid index in message
    /// Postcondition: stop is in range [start, message.size()-1]
    Status getLengthValue(
        const OctetArray& message,
        std::size_t start,
        std::size_t& stop,
        std::size_t& length);

    /// Decodes a DER-encoded Boolean value
    ///
    /// Maps from Ada:
    /// ```ada
    /// procedure Get_Boolean_Value
    ///   (Message : in  Octet_Array;
    ///    Start   : in  Natural;
    ///    Stop    : out Natural;
    ///    Value   : out Boolean;
    ///    Status  : out Status_Type)
    ///   with
    ///     Depends => ( (Stop, Value, Status) => (Message, Start) ),
    ///     Pre => Start in Message'Range,
    ///     Post => Stop in Start .. Message'Last;
    /// ```
    ///
    /// DER Boolean encoding: Tag (0x01) + Length (0x01) + Value (0x00 or 0xFF)
    /// - Only 0x00 (false) and 0xFF (true) are valid DER encodings
    /// - 0x01 and other non-zero values are rejected as BadValue
    ///
    /// Validation stages:
    /// 1. Identifier byte must be 0x01 (Universal, Primitive, Boolean)
    /// 2. Length must be 1
    /// 3. Value byte must be 0x00 or 0xFF
    ///
    /// @param message The message to examine
    /// @param start Starting position in message where Boolean TLV begins
    /// @param stop Output: last position used by the encoded Boolean
    /// @param value Output: the decoded Boolean value
    /// @return Status code (Success or BadValue)
    ///
    /// Precondition: start must be a valid index in message
    /// Postcondition: stop is in range [start, message.size()-1]
    Status getBooleanValue(
        const OctetArray& message,
        std::size_t start,
        std::size_t& stop,
        bool& value);

    /// Decodes a DER-encoded integer value
    ///
    /// Maps from Ada:
    /// ```ada
    /// procedure Get_Integer_Value
    ///   (Message : in  Octet_Array;
    ///    Start   : in  Natural;
    ///    Stop    : out Natural;
    ///    Value   : out Integer;
    ///    Status  : out Status_Type)
    ///   with
    ///     Depends => ( (Stop, Value, Status) => (Message, Start) ),
    ///     Pre => Start in Message'Range,
    ///     Post => Stop in Start .. Message'Last;
    /// ```
    ///
    /// DER Integer encoding: Tag (0x02) + Length + Value (big-endian two's complement)
    /// - Length must be 1-4 bytes (5+ bytes returns UnimplementedValue)
    /// - Two's complement encoding: high bit set indicates negative
    ///
    /// DER canonical form validation (rejects non-canonical encodings):
    /// - No unnecessary leading zeros: If length >= 2 and first byte is 0x00
    ///   and second byte has high bit clear, this is an unnecessary leading zero
    /// - No unnecessary leading ones: If length >= 2 and first byte is 0xFF
    ///   and second byte has bit 7 set (and other bits not all 1), unnecessary leading one
    ///
    /// Validation stages:
    /// 1. Identifier byte must be 0x02 (Universal, Primitive, Integer)
    /// 2. Length must be 1-4 bytes
    /// 3. Value must be in canonical form (no unnecessary leading zeros/ones)
    /// 4. Extract value using two's complement for negative numbers
    ///
    /// @param message The message to examine
    /// @param start Starting position in message where Integer TLV begins
    /// @param stop Output: last position used by the encoded integer
    /// @param value Output: the decoded integer value
    /// @return Status code (Success, BadValue, or UnimplementedValue)
    ///
    /// Precondition: start must be a valid index in message
    /// Postcondition: stop is in range [start, message.size()-1]
    Status getIntegerValue(
        const OctetArray& message,
        std::size_t start,
        std::size_t& stop,
        int32_t& value);

    /// Decodes a DER-encoded Object Identifier value
    ///
    /// Maps from Ada:
    /// ```ada
    /// procedure Get_OID_Value
    ///   (Message : in  Octet_Array;
    ///    Start   : in  Natural;
    ///    Stop    : out Natural;
    ///    Value   : out Hermes.OID.Object_Identifier;
    ///    Status  : out Status_Type)
    ///   with
    ///     Depends => ( (Stop, Value, Status) => (Message, Start) ),
    ///     Pre => Start in Message'Range,
    ///     Post => Stop in Start .. Message'Last;
    /// ```
    ///
    /// DER OID encoding: Tag (0x06) + Length + Value
    /// - First octet encodes first two components: value = (first * 40) + second
    ///   - first must be 0, 1, or 2
    ///   - If first is 0 or 1, second must be 0-39
    ///   - If first is 2, second can be 0-175
    /// - Remaining octets encode subsequent components using base-128 encoding:
    ///   - High bit (0x80) set means more bytes follow
    ///   - Low 7 bits contribute to the component value
    ///   - Continue until byte with high bit clear
    ///
    /// Example: SHA-256 OID {2, 16, 840, 1, 101, 3, 4, 2, 1}
    /// - First byte: 2*40 + 16 = 96 = 0x60
    /// - 840 encodes as: 0x86 0x48 (840 = 6*128 + 72)
    /// - 1 encodes as: 0x01
    /// - 101 encodes as: 0x65
    /// - Remaining components are small and encode as single bytes
    ///
    /// NOTE: This function was a stub (TODO) in the Ada implementation.
    /// This C++ version provides a complete implementation.
    ///
    /// @param message The message to examine
    /// @param start Starting position in message where OID TLV begins
    /// @param stop Output: last position used by the encoded OID
    /// @param value Output: the decoded Object Identifier
    /// @return Status code (Success or BadValue)
    ///
    /// Precondition: start must be a valid index in message
    /// Postcondition: stop is in range [start, message.size()-1]
    Status getOIDValue(
        const OctetArray& message,
        std::size_t start,
        std::size_t& stop,
        oid::ObjectIdentifier& value);

} // namespace decode
} // namespace der
} // namespace hermes
