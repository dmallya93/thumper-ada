///////////////////////////////////////////////////////////////////////////
// FILE    : der_encode.cpp
// SUBJECT : Implementation of DER encoding functions
// AUTHOR  : (C) Copyright 2022 by Peter Chapin
//
// This file implements DER (Distinguished Encoding Rules) encoding for
// ASN.1 primitive types. The implementation closely follows the Ada/SPARK
// reference implementation to ensure binary compatibility.
//
// Maps from Ada package body: Hermes.DER.Encode (hermes-der-encode.adb)
//
///////////////////////////////////////////////////////////////////////////

#include "hermes/der_encode.hpp"
#include <algorithm>

namespace hermes {
namespace der {
namespace encode {

    Octet makeLeadingIdentifier(
        TagClass tagClass,
        StructuredFlag structuredFlag,
        LeadingNumber tag)
    {
        // Lookup tables mapping enum values to bit patterns
        // Maps from Ada lookup tables in Make_Leading_Identifier

        // Tag class occupies bits 7-6
        constexpr Octet tagClassLookup[] = {
            0b0000'0000,  // Universal
            0b0100'0000,  // Application
            0b1000'0000,  // ContextSpecific
            0b1100'0000   // Private
        };

        // Structured flag occupies bit 5
        constexpr Octet structuredFlagLookup[] = {
            0b0000'0000,  // Primitive
            0b0010'0000   // Constructed
        };

        // Tag number occupies bits 4-0 for short form (0-30)
        // LeadingNumber enum values directly correspond to tag numbers
        const Octet tagNumber = static_cast<Octet>(tag);

        // Combine the three components with bitwise OR
        return tagClassLookup[static_cast<std::size_t>(tagClass)] |
               structuredFlagLookup[static_cast<std::size_t>(structuredFlag)] |
               tagNumber;
    }

    OctetArray putLengthValue(std::uint32_t length)
    {
        // Maps from Ada: Put_Length_Value in hermes-der-encode.adb
        // Returns 1-5 bytes depending on length value
        OctetArray result;

        if (length <= 127) {
            // Short form: single byte containing the length value
            result.push_back(static_cast<Octet>(length));
        }
        else if (length <= 255) {
            // Long form: 2 bytes
            // First byte: 0x81 (0b1000_0001 = bit 7 set, lower 7 bits = 1 byte follows)
            result.push_back(0b1000'0001);
            result.push_back(static_cast<Octet>(length));
        }
        else if (length <= 65535) {
            // Long form: 3 bytes
            // First byte: 0x82 (2 bytes follow)
            // Next 2 bytes: length in big-endian
            result.push_back(0b1000'0010);
            result.push_back(static_cast<Octet>((length >> 8) & 0xFF));
            result.push_back(static_cast<Octet>(length & 0xFF));
        }
        else if (length <= 16777215) {
            // Long form: 4 bytes
            // First byte: 0x83 (3 bytes follow)
            // Next 3 bytes: length in big-endian
            result.push_back(0b1000'0011);
            result.push_back(static_cast<Octet>((length >> 16) & 0xFF));
            result.push_back(static_cast<Octet>((length >> 8) & 0xFF));
            result.push_back(static_cast<Octet>(length & 0xFF));
        }
        else {
            // Long form: 5 bytes
            // First byte: 0x84 (4 bytes follow)
            // Next 4 bytes: length in big-endian
            result.push_back(0b1000'0100);
            result.push_back(static_cast<Octet>((length >> 24) & 0xFF));
            result.push_back(static_cast<Octet>((length >> 16) & 0xFF));
            result.push_back(static_cast<Octet>((length >> 8) & 0xFF));
            result.push_back(static_cast<Octet>(length & 0xFF));
        }

        return result;
    }

    OctetArray putBooleanValue(bool value)
    {
        // Maps from Ada: Put_Boolean_Value in hermes-der-encode.adb
        // Returns exactly 3 bytes: Tag, Length, Value
        OctetArray result;

        // Tag: Universal class, Primitive, Boolean
        result.push_back(makeLeadingIdentifier(
            TagClass::Universal,
            StructuredFlag::Primitive,
            LeadingNumber::Boolean));

        // Length: Always 1 byte for boolean
        result.push_back(0x01);

        // Value: 0xFF for true, 0x00 for false
        result.push_back(value ? 0xFF : 0x00);

        return result;
    }

    OctetArray putIntegerValue(std::int32_t value)
    {
        // Maps from Ada: Put_Integer_Value in hermes-der-encode.adb
        // Note: Ada implementation only handles non-negative integers
        // This implementation follows the same pattern

        OctetArray result;

        // Tag: Universal class, Primitive, Integer
        result.push_back(makeLeadingIdentifier(
            TagClass::Universal,
            StructuredFlag::Primitive,
            LeadingNumber::Integer));

        // Determine the minimum number of bytes needed and encode the value
        if (value <= 0x7F) {
            // 1 content byte
            result.push_back(0x01);  // Length
            result.push_back(static_cast<Octet>(value & 0xFF));
        }
        else if (value <= 0x7FFF) {
            // 2 content bytes
            result.push_back(0x02);  // Length
            result.push_back(static_cast<Octet>((value >> 8) & 0xFF));
            result.push_back(static_cast<Octet>(value & 0xFF));
        }
        else if (value <= 0x7FFFFF) {
            // 3 content bytes
            result.push_back(0x03);  // Length
            result.push_back(static_cast<Octet>((value >> 16) & 0xFF));
            result.push_back(static_cast<Octet>((value >> 8) & 0xFF));
            result.push_back(static_cast<Octet>(value & 0xFF));
        }
        else {
            // 4 content bytes
            result.push_back(0x04);  // Length
            result.push_back(static_cast<Octet>((value >> 24) & 0xFF));
            result.push_back(static_cast<Octet>((value >> 16) & 0xFF));
            result.push_back(static_cast<Octet>((value >> 8) & 0xFF));
            result.push_back(static_cast<Octet>(value & 0xFF));
        }

        return result;
    }

    OctetArray putOctetStringValue(const OctetArray& value)
    {
        // Maps from Ada: Put_Octet_String_Value in hermes-der-encode.adb
        // This is a simple concatenation in Ada using expression function

        OctetArray result;

        // Tag: Universal class, Primitive, OctetString
        result.push_back(makeLeadingIdentifier(
            TagClass::Universal,
            StructuredFlag::Primitive,
            LeadingNumber::OctetString));

        // Length: DER encoded length of the value
        OctetArray lengthBytes = putLengthValue(static_cast<std::uint32_t>(value.size()));
        result.insert(result.end(), lengthBytes.begin(), lengthBytes.end());

        // Value: The octets themselves
        result.insert(result.end(), value.begin(), value.end());

        return result;
    }

    OctetArray putNullValue()
    {
        // Maps from Ada: Put_Null_Value in hermes-der-encode.adb
        // Returns exactly 2 bytes: Tag and zero Length

        OctetArray result;

        // Tag: Universal class, Primitive, Null
        result.push_back(makeLeadingIdentifier(
            TagClass::Universal,
            StructuredFlag::Primitive,
            LeadingNumber::Null));

        // Length: Always 0 for null
        result.push_back(0x00);

        return result;
    }

    // Helper function for encoding OID components in base-128 with continuation bits
    // Maps from Ada: nested procedure To_Octet_Array in Put_OID_Value
    namespace {
        void toOctetArray(
            const oid::ObjectIdentifier& identifier,
            OctetArray& result,
            std::size_t& octetCount)
        {
            // Maps from Ada nested procedure To_Octet_Array in hermes-der-encode.adb:179-276

            result.clear();
            octetCount = 0;

            // Get the component array from the OID
            oid::ComponentArray separates = oid::toSeparates(identifier);
            const std::size_t numberOfComponents = separates.size();

            if (numberOfComponents < 2) {
                // Invalid OID - need at least 2 components
                return;
            }

            // First byte: Encode first two components as (40 * first + second)
            // This is mandated by ASN.1 encoding rules
            const Octet firstByte = static_cast<Octet>(
                (separates[0] * 40) + separates[1]);
            result.push_back(firstByte);
            octetCount = 1;

            // Encode remaining components (index 2 onwards)
            for (std::size_t otherIndex = 2; otherIndex < numberOfComponents; ++otherIndex) {
                oid::ComponentType currentComponent = separates[otherIndex];

                // Break the current component into 7-bit units
                // Store in little-endian order initially (will reverse later)
                std::vector<Octet> tempBuffer;

                do {
                    tempBuffer.push_back(static_cast<Octet>(currentComponent & 0x7F));
                    currentComponent >>= 7;
                } while (currentComponent > 0);

                // Reverse to big-endian order
                std::reverse(tempBuffer.begin(), tempBuffer.end());

                // Set MSB (continuation bit) on all bytes except the last
                for (std::size_t i = 0; i < tempBuffer.size() - 1; ++i) {
                    tempBuffer[i] |= 0x80;
                }

                // Append to result
                result.insert(result.end(), tempBuffer.begin(), tempBuffer.end());
                octetCount += tempBuffer.size();
            }
        }
    }

    OctetArray putOIDValue(const oid::ObjectIdentifier& value)
    {
        // Maps from Ada: Put_OID_Value in hermes-der-encode.adb:179-290

        // Encode the OID components to octets
        OctetArray encoded;
        std::size_t count;
        toOctetArray(value, encoded, count);

        // Build the TLV structure
        OctetArray result;

        // Tag: Universal class, Primitive, ObjectIdentifier
        result.push_back(makeLeadingIdentifier(
            TagClass::Universal,
            StructuredFlag::Primitive,
            LeadingNumber::ObjectIdentifier));

        // Length: DER encoded length of the encoded components
        OctetArray lengthBytes = putLengthValue(static_cast<std::uint32_t>(count));
        result.insert(result.end(), lengthBytes.begin(), lengthBytes.end());

        // Value: The encoded component bytes
        result.insert(result.end(), encoded.begin(), encoded.end());

        return result;
    }

} // namespace encode
} // namespace der
} // namespace hermes
