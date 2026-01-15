///////////////////////////////////////////////////////////////////////////
// FILE    : oid.cpp
// SUBJECT : Implementation of Object Identifier support
// AUTHOR  : (C) Copyright 2022 by Peter C. Chapin
//
// Object Identifier implementation with validation of ASN.1 constraints.
///////////////////////////////////////////////////////////////////////////

#include "hermes/oid.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace hermes {
namespace oid {

    // Helper function to validate root component
    // Maps from Ada nested function: Bad_First_Level
    static bool isBadFirstLevel(ComponentType root) {
        return !(root == 0 || root == 1 || root == 2);
    }

    // Helper function to validate second level component based on root
    // Maps from Ada nested function: Bad_Second_Level
    static bool isBadSecondLevel(ComponentType root, ComponentType second) {
        switch (root) {
            case 0:
            case 1:
                return second >= 40;  // Must be < 40
            case 2:
                return second > 175;  // Must be <= 175
            default:
                return true;  // Invalid root
        }
    }

    // ObjectIdentifier implementation

    ObjectIdentifier::ObjectIdentifier() noexcept
        : rootComponent_(0)
        , secondLevelComponent_(0)
        , otherComponents_{}
        , otherComponentCount_(0)
    {
        // Default constructor creates an effectively empty OID
    }

    std::size_t ObjectIdentifier::componentCount() const noexcept {
        // Maps from Ada: return Identifier.Other_Component_Count + 2;
        return otherComponentCount_ + 2;
    }

    // Free function implementations

    Status toObjectIdentifier(const ComponentArray& separates, ObjectIdentifier& result) {
        // Maps from Ada procedure To_Object_Identifier

        // Initialize result to default state
        // Maps from Ada: Result := Object_Identifier'(0, 0, Other_Component_Array'(others => 0), 0);
        result.rootComponent_ = 0;
        result.secondLevelComponent_ = 0;
        std::fill(result.otherComponents_.begin(), result.otherComponents_.end(), 0);
        result.otherComponentCount_ = 0;

        // Validate we have at least a root component
        // Maps from Ada: if Separates'Length < 1 or else Bad_First_Level(Separates(Separates'First))
        if (separates.empty() || isBadFirstLevel(separates[0])) {
            return Status::InvalidRoot;
        }

        result.rootComponent_ = static_cast<std::uint8_t>(separates[0]);

        // Validate we have a second level component
        // Maps from Ada: if Separates'Length < 2 or else Bad_Second_Level(...)
        if (separates.size() < 2 || isBadSecondLevel(separates[0], separates[1])) {
            return Status::InvalidSecondLevel;
        }

        result.secondLevelComponent_ = static_cast<std::uint8_t>(separates[1]);

        // Copy remaining components
        // Maps from Ada: for I in Component_Index_Type range Separates'First + 2 .. Separates'Last
        const std::size_t otherCount = separates.size() - 2;

        // Check if we have space for all components
        if (otherCount > result.otherComponents_.size()) {
            return Status::InsufficientSpace;
        }

        for (std::size_t i = 0; i < otherCount; ++i) {
            result.otherComponents_[i] = separates[i + 2];
        }
        result.otherComponentCount_ = otherCount;

        return Status::Success;
    }

    Status toSeparates(const ObjectIdentifier& identifier,
                      ComponentArray& result,
                      std::size_t& numberOfComponents) {
        // Maps from Ada procedure To_Separates

        // Initialize output
        // Maps from Ada: Result := (others => 0); Number_Of_Components := 0;
        result.clear();
        numberOfComponents = 0;

        // Check if we have sufficient space
        const std::size_t totalComponents = identifier.componentCount();
        // In C++, we can just resize the vector, but for API compatibility
        // we maintain similar behavior to Ada version

        // Maps from Ada: if Identifier.Other_Component_Count + 2 <= Result'Length
        // We'll resize the result to fit
        result.resize(totalComponents);

        // Copy root and second level components
        // Maps from Ada: Result(Result'First) := Identifier.Root_Component;
        result[0] = identifier.rootComponent_;
        result[1] = identifier.secondLevelComponent_;

        // Copy other components
        // Maps from Ada: for I in Other_Count_Type range 1 .. Identifier.Other_Component_Count
        for (std::size_t i = 0; i < identifier.otherComponentCount_; ++i) {
            result[i + 2] = identifier.otherComponents_[i];
        }

        numberOfComponents = totalComponents;
        return Status::Success;
    }

    ComponentArray toSeparates(const ObjectIdentifier& identifier) {
        ComponentArray result;
        std::size_t numberOfComponents;

        // This version always succeeds
        toSeparates(identifier, result, numberOfComponents);

        return result;
    }

    std::string oidToString(const ComponentArray& separates) {
        // Maps from Ada procedure OID_To_String

        if (separates.empty()) {
            return "";
        }

        std::ostringstream oss;

        // Convert each component to string with dot separator
        // Maps from Ada: for I in Separates'Range loop
        for (std::size_t i = 0; i < separates.size(); ++i) {
            if (i > 0) {
                oss << '.';
            }
            oss << separates[i];
        }

        return oss.str();
    }

    Status stringToOID(const std::string& oidString, ComponentArray& separates) {
        // Maps from Ada procedure String_To_OID

        separates.clear();

        if (oidString.empty()) {
            return Status::Success;  // Empty string -> empty array
        }

        // Parse the dotted string
        std::istringstream iss(oidString);
        std::string token;

        // Maps from Ada: Find_Token with Outside test (finding non-dot characters)
        while (std::getline(iss, token, '.')) {
            // Skip empty tokens (from consecutive dots or leading/trailing dots)
            if (token.empty()) {
                continue;
            }

            // Convert string to component value
            // Maps from Ada: String_To_Component calling Component_Type'Value
            try {
                // Parse as unsigned long to handle full range
                std::size_t pos;
                unsigned long value = std::stoul(token, &pos);

                // Check that entire token was parsed
                if (pos != token.length()) {
                    throw std::invalid_argument("Invalid character in OID component");
                }

                // Check range (Component_Type is uint32_t, but we check for overflow)
                if (value > std::numeric_limits<ComponentType>::max()) {
                    throw std::invalid_argument("OID component value too large");
                }

                separates.push_back(static_cast<ComponentType>(value));
            }
            catch (const std::exception& e) {
                throw std::invalid_argument(std::string("Invalid OID string: ") + e.what());
            }
        }

        return Status::Success;
    }

    ComponentArray stringToOID(const std::string& oidString) {
        ComponentArray result;
        Status status = stringToOID(oidString, result);

        // This version propagates exceptions from the underlying implementation
        if (status != Status::Success) {
            throw std::invalid_argument("Failed to parse OID string");
        }

        return result;
    }

} // namespace oid
} // namespace hermes
