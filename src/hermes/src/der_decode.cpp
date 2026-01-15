///////////////////////////////////////////////////////////////////////////
// FILE    : der_decode.cpp
// SUBJECT : Implementation of DER decoding functions
// AUTHOR  : (C) Copyright 2022 by Peter Chapin
//
// This file implements ASN.1 DER decoding functions that parse DER-encoded
// byte sequences back into primitive values.
//
// Maps from Ada package body: Hermes.DER.Decode (hermes-der-decode.adb)
//
///////////////////////////////////////////////////////////////////////////

#include "hermes/der_decode.hpp"
#include <algorithm>
#include <limits>

namespace hermes {
namespace der {
namespace decode {

    namespace {
        // Lookup table for tag numbers (maps 5-bit tag value to LeadingNumber enum)
        // Maps from Ada: Leading_Number_Lookup_Table array
        constexpr LeadingNumber tagLookupTable[32] = {
            LeadingNumber::Reserved,        // 0
            LeadingNumber::Boolean,         // 1
            LeadingNumber::Integer,         // 2
            LeadingNumber::BitString,       // 3
            LeadingNumber::OctetString,     // 4
            LeadingNumber::Null,            // 5
            LeadingNumber::ObjectIdentifier,// 6
            LeadingNumber::ObjectDescriptor,// 7
            LeadingNumber::InstanceOf,      // 8 (could also be External)
            LeadingNumber::Real,            // 9
            LeadingNumber::Enumerated,      // 10
            LeadingNumber::EmbeddedPDV,     // 11
            LeadingNumber::UTF8String,      // 12
            LeadingNumber::RelativeOID,     // 13
            LeadingNumber::Null,            // 14 - undefined
            LeadingNumber::Null,            // 15 - undefined
            LeadingNumber::Sequence,        // 16
            LeadingNumber::Set,             // 17
            LeadingNumber::NumericString,   // 18
            LeadingNumber::PrintableString, // 19
            LeadingNumber::TeletexString,   // 20
            LeadingNumber::VideotexString,  // 21
            LeadingNumber::IA5String,       // 22
            LeadingNumber::UTCTime,         // 23
            LeadingNumber::GeneralizedTime, // 24
            LeadingNumber::GraphicString,   // 25
            LeadingNumber::VisibleString,   // 26
            LeadingNumber::GeneralString,   // 27
            LeadingNumber::UniversalString, // 28
            LeadingNumber::CharacterString, // 29
            LeadingNumber::BMPString,       // 30
            LeadingNumber::ExtendedTag      // 31
        };
    }

    Status splitLeadingIdentifier(
        Octet value,
        TagClass& tagClass,
        StructuredFlag& structuredFlag,
        LeadingNumber& tag)
    {
        // Extract tag class from bits 7-6
        // Maps from Ada: Set_Tag_Class nested procedure
        switch (value & 0xC0) {
            case 0x00: tagClass = TagClass::Universal; break;
            case 0x40: tagClass = TagClass::Application; break;
            case 0x80: tagClass = TagClass::ContextSpecific; break;
            case 0xC0: tagClass = TagClass::Private; break;
            default: tagClass = TagClass::Universal; break; // Should never happen
        }

        // Extract structured flag from bit 5
        // Maps from Ada: Set_Structured_Flag nested procedure
        switch (value & 0x20) {
            case 0x00: structuredFlag = StructuredFlag::Primitive; break;
            case 0x20: structuredFlag = StructuredFlag::Constructed; break;
            default: structuredFlag = StructuredFlag::Primitive; break; // Should never happen
        }

        // Extract tag number from bits 4-0
        // Maps from Ada: Set_Tag nested procedure
        Octet tagValue = value & 0x1F;

        // Tag numbers 14 and 15 are undefined in ASN.1
        if (tagValue == 14 || tagValue == 15) {
            tag = LeadingNumber::Null;
            return Status::BadIdentifier;
        }

        tag = tagLookupTable[tagValue];
        return Status::Success;
    }

    Status getLengthValue(
        const OctetArray& message,
        std::size_t start,
        std::size_t& stop,
        std::size_t& length)
    {
        // Precondition: start must be valid index
        if (start >= message.size()) {
            stop = start;
            length = 0;
            return Status::BadLength;
        }

        Octet firstByte = message[start];

        // Check for indefinite length (0x80) - not allowed in DER
        if (firstByte == 0x80) {
            stop = start;
            length = 0;
            return Status::IndefiniteLength;
        }

        // Check for definite length, short form (high bit clear)
        if ((firstByte & 0x80) == 0x00) {
            stop = start;
            length = static_cast<std::size_t>(firstByte);
            return Status::Success;
        }

        // Check for reserved value (0xFF)
        if (firstByte == 0xFF) {
            stop = start;
            length = 0;
            return Status::BadLength;
        }

        // Definite length, long form: low 7 bits = number of length octets
        std::size_t lengthOfLength = static_cast<std::size_t>(firstByte & 0x7F);

        // Check that all length octets are available in the message
        // Ada: Start > Message'Last - Length_Of_Length
        // C++ with 0-based indexing: start + lengthOfLength >= message.size()
        if (start + lengthOfLength >= message.size()) {
            stop = message.size() - 1;
            length = 0;
            return Status::BadLength;
        }

        // Check that the length value is not too large (limit to 4 bytes for 32-bit size_t)
        // Note: On 64-bit systems, size_t can hold more, but we keep the Ada limit
        if (lengthOfLength > 4 || (lengthOfLength == 4 && message[start + 1] >= 128)) {
            stop = start + lengthOfLength;
            length = 0;
            return Status::UnimplementedLength;
        }

        // Convert length octets to a single value (big-endian)
        // Maps from Ada: Convert_Length nested function with loop invariants
        std::size_t result = 0;
        for (std::size_t i = 1; i <= lengthOfLength; ++i) {
            // Ada loop invariant: Result stays within bounds to prevent overflow
            result = (result * 256) + static_cast<std::size_t>(message[start + i]);
        }

        stop = start + lengthOfLength;
        length = result;
        return Status::Success;
    }

    Status getBooleanValue(
        const OctetArray& message,
        std::size_t start,
        std::size_t& stop,
        bool& value)
    {
        // Precondition: start must be valid index
        if (start >= message.size()) {
            stop = start;
            value = false;
            return Status::BadValue;
        }

        // Stage 1: Validate identifier byte
        // Must be 0x01: Universal (00), Primitive (0), Boolean (00001)
        TagClass tagClass;
        StructuredFlag structuredFlag;
        LeadingNumber tag;
        Status identifierStatus = splitLeadingIdentifier(
            message[start], tagClass, structuredFlag, tag);

        if (identifierStatus != Status::Success ||
            tagClass != TagClass::Universal ||
            structuredFlag != StructuredFlag::Primitive ||
            tag != LeadingNumber::Boolean) {
            stop = start;
            value = false;
            return Status::BadValue;
        }

        // Check we have room for at least length byte
        if (start > message.size() - 2) {
            stop = start;
            value = false;
            return Status::BadValue;
        }

        // Stage 2: Decode and validate length
        // Maps from Ada: Identifier_Ok nested procedure
        std::size_t lengthStop;
        std::size_t lengthValue;
        Status lengthStatus = getLengthValue(message, start + 1, lengthStop, lengthValue);

        if (lengthStatus != Status::Success) {
            stop = lengthStop;
            value = false;
            return Status::BadValue;
        }

        // Check that value doesn't go off end of message
        if (lengthStop > message.size() - 1 - lengthValue) {
            stop = message.size() - 1;
            value = false;
            return Status::BadValue;
        }

        // Boolean must have length exactly 1
        if (lengthValue != 1) {
            stop = lengthStop + lengthValue;
            value = false;
            return Status::BadValue;
        }

        // Stage 3: Extract and validate value byte
        // Maps from Ada: Identifier_And_Length_Ok nested procedure
        stop = lengthStop + lengthValue;
        Octet valueByte = message[lengthStop + 1];

        if (valueByte == 0xFF) {
            value = true;
            return Status::Success;
        } else if (valueByte == 0x00) {
            value = false;
            return Status::Success;
        } else {
            // DER only allows 0x00 (false) and 0xFF (true)
            // Reject 0x01 and other non-zero values
            value = false;
            return Status::BadValue;
        }
    }

    Status getIntegerValue(
        const OctetArray& message,
        std::size_t start,
        std::size_t& stop,
        int32_t& value)
    {
        // Precondition: start must be valid index
        if (start >= message.size()) {
            stop = start;
            value = 0;
            return Status::BadValue;
        }

        // Stage 1: Validate identifier byte
        // Must be 0x02: Universal (00), Primitive (0), Integer (00010)
        TagClass tagClass;
        StructuredFlag structuredFlag;
        LeadingNumber tag;
        Status identifierStatus = splitLeadingIdentifier(
            message[start], tagClass, structuredFlag, tag);

        if (identifierStatus != Status::Success ||
            tagClass != TagClass::Universal ||
            structuredFlag != StructuredFlag::Primitive ||
            tag != LeadingNumber::Integer) {
            stop = start;
            value = 0;
            return Status::BadValue;
        }

        // Check we have room for at least length byte
        if (start > message.size() - 2) {
            stop = start;
            value = 0;
            return Status::BadValue;
        }

        // Stage 2: Decode and validate length
        // Maps from Ada: Identifier_Ok nested procedure
        std::size_t lengthStop;
        std::size_t lengthValue;
        Status lengthStatus = getLengthValue(message, start + 1, lengthStop, lengthValue);

        if (lengthStatus != Status::Success) {
            stop = lengthStop;
            value = 0;
            return Status::BadValue;
        }

        // Check that value doesn't go off end of message
        if (lengthStop > message.size() - 1 - lengthValue) {
            stop = message.size() - 1;
            value = 0;
            return Status::BadValue;
        }

        // Length must be 1-4 bytes (limit for 32-bit integer)
        if (lengthValue > 4) {
            stop = lengthStop + lengthValue;
            value = 0;
            return Status::UnimplementedValue;
        }

        // Validate canonical form: check for unnecessary leading zeros
        // If length >= 2 and first byte is 0x00 and second byte has high bit clear
        if (lengthValue >= 2 &&
            message[lengthStop + 1] == 0x00 &&
            (message[lengthStop + 2] & 0x80) == 0x00) {
            stop = lengthStop + lengthValue;
            value = 0;
            return Status::BadValue;
        }

        // Validate canonical form: check for unnecessary leading ones
        // If length >= 2 and first byte is 0xFF and second byte has all bits except high bit
        // Note: The Ada check is: (Message(Length_Stop + 2) or 16#7F#) = 16#FF#
        // This means: if ORing with 0x7F gives 0xFF, then all bits except bit 7 were already 1
        // which means the high bit (bit 7) must also be 1, making 0xFF redundant
        if (lengthValue >= 2 &&
            message[lengthStop + 1] == 0xFF &&
            (message[lengthStop + 2] | 0x7F) == 0xFF) {
            stop = lengthStop + lengthValue;
            value = 0;
            return Status::BadValue;
        }

        // Stage 3: Extract integer value
        // Maps from Ada: Identifier_And_Length_Ok nested procedure
        stop = lengthStop + lengthValue;

        // If high bit of first byte is clear, value is positive
        if ((message[lengthStop + 1] & 0x80) == 0) {
            int32_t result = 0;
            for (std::size_t i = 1; i <= lengthValue; ++i) {
                result = (result * 256) + static_cast<int32_t>(message[lengthStop + i]);
            }
            value = result;
            return Status::Success;
        }

        // High bit is set: negative value, use two's complement
        // Invert bits, then negate
        int32_t result = 0;
        for (std::size_t i = 1; i <= lengthValue; ++i) {
            result = (result * 256) + static_cast<int32_t>(message[lengthStop + i] ^ 0xFF);
        }

        // Special case: handle most negative value due to two's complement asymmetry
        // Integer'First in Ada is -2^31, which can't be represented as -(positive)
        if (result == std::numeric_limits<int32_t>::max()) {
            value = std::numeric_limits<int32_t>::min();
        } else {
            value = -(result + 1);
        }

        return Status::Success;
    }

    Status getOIDValue(
        const OctetArray& message,
        std::size_t start,
        std::size_t& stop,
        oid::ObjectIdentifier& value)
    {
        // NOTE: This was a stub (TODO) in the Ada implementation
        // This C++ version provides a complete implementation

        // Precondition: start must be valid index
        if (start >= message.size()) {
            stop = start;
            value = oid::ObjectIdentifier();
            return Status::BadValue;
        }

        // Stage 1: Validate identifier byte
        // Must be 0x06: Universal (00), Primitive (0), ObjectIdentifier (00110)
        TagClass tagClass;
        StructuredFlag structuredFlag;
        LeadingNumber tag;
        Status identifierStatus = splitLeadingIdentifier(
            message[start], tagClass, structuredFlag, tag);

        if (identifierStatus != Status::Success ||
            tagClass != TagClass::Universal ||
            structuredFlag != StructuredFlag::Primitive ||
            tag != LeadingNumber::ObjectIdentifier) {
            stop = start;
            value = oid::ObjectIdentifier();
            return Status::BadValue;
        }

        // Check we have room for at least length byte
        if (start > message.size() - 2) {
            stop = start;
            value = oid::ObjectIdentifier();
            return Status::BadValue;
        }

        // Stage 2: Decode and validate length
        std::size_t lengthStop;
        std::size_t lengthValue;
        Status lengthStatus = getLengthValue(message, start + 1, lengthStop, lengthValue);

        if (lengthStatus != Status::Success) {
            stop = lengthStop;
            value = oid::ObjectIdentifier();
            return Status::BadValue;
        }

        // Check that value doesn't go off end of message
        if (lengthStop > message.size() - 1 - lengthValue || lengthValue == 0) {
            stop = message.size() - 1;
            value = oid::ObjectIdentifier();
            return Status::BadValue;
        }

        // Stage 3: Decode OID components
        // First byte encodes first two components: value = (first * 40) + second
        Octet firstByte = message[lengthStop + 1];
        oid::ComponentType firstComponent = static_cast<oid::ComponentType>(firstByte / 40);
        oid::ComponentType secondComponent = static_cast<oid::ComponentType>(firstByte % 40);

        // Validate first component (must be 0, 1, or 2)
        if (firstComponent > 2) {
            stop = lengthStop + lengthValue;
            value = oid::ObjectIdentifier();
            return Status::BadValue;
        }

        // Validate second component constraints
        if ((firstComponent == 0 || firstComponent == 1) && secondComponent >= 40) {
            stop = lengthStop + lengthValue;
            value = oid::ObjectIdentifier();
            return Status::BadValue;
        }
        if (firstComponent == 2 && secondComponent > 175) {
            stop = lengthStop + lengthValue;
            value = oid::ObjectIdentifier();
            return Status::BadValue;
        }

        // Build component array starting with first two components
        oid::ComponentArray components;
        components.push_back(firstComponent);
        components.push_back(secondComponent);

        // Decode remaining components using base-128 encoding
        std::size_t pos = lengthStop + 2;
        std::size_t endPos = lengthStop + lengthValue + 1;

        while (pos < endPos) {
            oid::ComponentType component = 0;
            bool continueBit;

            do {
                if (pos >= endPos) {
                    stop = lengthStop + lengthValue;
                    value = oid::ObjectIdentifier();
                    return Status::BadValue;
                }

                Octet byte = message[pos++];
                continueBit = (byte & 0x80) != 0;

                // Check for overflow before shifting and adding
                if (component > (std::numeric_limits<oid::ComponentType>::max() >> 7)) {
                    stop = lengthStop + lengthValue;
                    value = oid::ObjectIdentifier();
                    return Status::BadValue;
                }

                component = (component << 7) | (byte & 0x7F);
            } while (continueBit);

            components.push_back(component);
        }

        // Construct ObjectIdentifier from components
        oid::Status oidStatus = oid::toObjectIdentifier(components, value);
        if (oidStatus != oid::Status::Success) {
            stop = lengthStop + lengthValue;
            value = oid::ObjectIdentifier();
            return Status::BadValue;
        }

        stop = lengthStop + lengthValue;
        return Status::Success;
    }

} // namespace decode
} // namespace der
} // namespace hermes
