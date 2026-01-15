///////////////////////////////////////////////////////////////////////////
// FILE    : der.hpp
// SUBJECT : DER (Distinguished Encoding Rules) type definitions
// AUTHOR  : (C) Copyright 2015 by Peter Chapin
//
// This header provides common type definitions for ASN.1 DER encoding
// and decoding operations. These types are used throughout the Hermes
// library to represent tag classes, status codes, and other DER-specific
// structures.
//
// Maps from Ada package: Hermes.DER
//
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>

namespace hermes {
namespace der {

    /// Status codes for DER encoding/decoding operations
    ///
    /// Maps from Ada: type Status_Type is (Success, Bad_Identifier, ...)
    ///
    /// Used to indicate success or the nature of errors in DER operations.
    /// Following the Ada pattern of explicit error handling via return codes.
    enum class Status : std::uint8_t {
        Success,                // Operation completed successfully
        BadIdentifier,          // Invalid or unexpected tag identifier
        IndefiniteLength,       // Indefinite length encoding (not allowed in DER)
        UnimplementedLength,    // Length encoding format not yet implemented
        BadLength,              // Invalid length field
        UnimplementedValue,     // Value type not yet implemented
        BadValue                // Invalid value encoding
    };

    /// ASN.1 tag class types
    ///
    /// Maps from Ada: type Tag_Class_Type is (Class_Universal, ...)
    ///
    /// Defines the four tag classes in ASN.1:
    /// - Universal: Standard ASN.1 types (INTEGER, BOOLEAN, etc.)
    /// - Application: Application-specific types
    /// - ContextSpecific: Context-specific tags (used in CHOICE, etc.)
    /// - Private: Private use tags
    enum class TagClass : std::uint8_t {
        Universal = 0,
        Application = 1,
        ContextSpecific = 2,
        Private = 3
    };

    /// Structured/primitive flag for ASN.1 tags
    ///
    /// Maps from Ada: type Structured_Flag_Type is (Primitive, Constructed)
    ///
    /// Indicates whether the value is:
    /// - Primitive: Simple value (e.g., INTEGER, BOOLEAN)
    /// - Constructed: Composed of other values (e.g., SEQUENCE, SET)
    enum class StructuredFlag : std::uint8_t {
        Primitive = 0,
        Constructed = 1
    };

    /// Universal tag numbers for common ASN.1 types
    ///
    /// Maps from Ada: type Leading_Number_Type is (Tag_Reserved, Tag_Boolean, ...)
    ///
    /// These are the standard tag numbers defined in the ASN.1 specification
    /// for universal class types. Each primitive ASN.1 type has an assigned number.
    enum class LeadingNumber : std::uint8_t {
        Reserved = 0,
        Boolean = 1,
        Integer = 2,
        BitString = 3,
        OctetString = 4,
        Null = 5,
        ObjectIdentifier = 6,
        ObjectDescriptor = 7,
        InstanceOf = 8,
        External = 9,
        Real = 10,
        Enumerated = 11,
        EmbeddedPDV = 12,
        UTF8String = 13,
        RelativeOID = 14,
        // 15 is reserved
        Sequence = 16,
        SequenceOf = 16,  // Same as Sequence
        Set = 17,
        SetOf = 17,       // Same as Set
        NumericString = 18,
        PrintableString = 19,
        TeletexString = 20,
        T61String = 20,   // Same as TeletexString
        VideotexString = 21,
        IA5String = 22,
        UTCTime = 23,
        GeneralizedTime = 24,
        GraphicString = 25,
        VisibleString = 26,
        ISO646String = 26,  // Same as VisibleString
        GeneralString = 27,
        UniversalString = 28,
        CharacterString = 29,
        BMPString = 30,
        ExtendedTag = 31   // Indicates long form tag
    };

} // namespace der
} // namespace hermes

