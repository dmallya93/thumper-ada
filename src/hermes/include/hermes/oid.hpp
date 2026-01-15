///////////////////////////////////////////////////////////////////////////
// FILE    : oid.hpp
// SUBJECT : Object Identifier (OID) support for ASN.1
// AUTHOR  : (C) Copyright 2022 by Peter C. Chapin
//
// This header provides Object Identifier representation and manipulation
// for the Hermes ASN.1 library. OIDs are used throughout RFC-3161 to
// identify cryptographic algorithms, policies, and message types.
//
// Design Decision #3 (OID Internal Representation): This implementation
// mirrors the Ada private record structure, storing the root component
// (0-2), second-level component (0-39 or 0-175), and an array of
// additional components. This design exploits ASN.1 encoding constraints
// to provide validation at construction time.
//
// Design Decision #1 (SPARK Contracts to C++ Type Safety): Ada SPARK
// contracts (Pre/Post/Depends) are translated to:
//   - Constructor validation with clear error reporting via Status enum
//   - Runtime assertions for preconditions (in debug builds)
//   - Comprehensive unit tests to catch contract violations
//   - Documentation of contracts in Doxygen comments
//
// Maps from Ada packages:
//   - Hermes.OID (hermes-oid.ads/adb)
//
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace hermes {
namespace oid {

    /// Maximum number of components in an OID
    /// Maps from Ada: Maximum_Component_Count : constant := 15;
    constexpr std::size_t MaximumComponentCount = 15;

    /// Component type for OID sub-identifiers
    /// Maps from Ada: type Component_Type is range 0 .. Integer'Last;
    /// Using uint32_t to represent component values (sufficient for ASN.1 OIDs)
    using ComponentType = std::uint32_t;

    /// Array type for OID components
    /// Maps from Ada: type Component_Array is array(Component_Index_Type range <>) of Component_Type;
    using ComponentArray = std::vector<ComponentType>;

    /// Status codes for OID operations
    /// Maps from Ada: type Status_Type is (Success, Invalid_Root, Invalid_Second_Level, Insufficient_Space);
    ///
    /// Used to indicate success or the nature of errors in OID operations.
    /// Following the Ada pattern of explicit error handling via return codes.
    enum class Status : std::uint8_t {
        Success,              // Operation completed successfully
        InvalidRoot,          // First component must be 0, 1, or 2
        InvalidSecondLevel,   // Second component out of valid range
        InsufficientSpace     // Not enough space in output array
    };

    /// Object Identifier representation
    ///
    /// Maps from Ada: type Object_Identifier is private;
    ///
    /// OIDs have special constraints per ASN.1 standards:
    /// - First component must be 0, 1, or 2
    /// - If first component is 0 or 1, second component must be < 40
    /// - If first component is 2, second component must be <= 175
    /// These constraints arise from the encoding scheme where the first
    /// two components are packed into a single octet.
    ///
    /// Internal representation (from Ada private section):
    /// ```ada
    /// type Object_Identifier is record
    ///    Root_Component         : Root_Component_Type;           -- 0..2
    ///    Second_Level_Component : Second_Level_Component_Type;   -- 0..175
    ///    Other_Components       : Other_Component_Array;          -- Additional components
    ///    Other_Component_Count  : Other_Count_Type;               -- Number of other components
    /// end record;
    /// ```
    class ObjectIdentifier {
    public:
        /// Default constructor creates an empty/invalid OID
        ObjectIdentifier() noexcept;

        /// Gets the root component (first component, 0-2)
        std::uint8_t getRootComponent() const noexcept { return rootComponent_; }

        /// Gets the second level component (second component, 0-39 or 0-175)
        std::uint8_t getSecondLevelComponent() const noexcept { return secondLevelComponent_; }

        /// Gets the count of additional components beyond the first two
        std::size_t getOtherComponentCount() const noexcept { return otherComponentCount_; }

        /// Returns the total number of components in the OID
        ///
        /// Maps from Ada: function Component_Count(Identifier : Object_Identifier) return Component_Count_Type;
        ///
        /// @return Total number of components (always at least 2 for valid OIDs)
        std::size_t componentCount() const noexcept;

        /// Allows read-only access to other components array for testing/debugging
        const std::array<ComponentType, MaximumComponentCount - 2>& getOtherComponents() const noexcept {
            return otherComponents_;
        }

    private:
        /// Root component (0, 1, or 2)
        /// Maps from Ada: Root_Component : Root_Component_Type;
        std::uint8_t rootComponent_;

        /// Second level component (0-39 for root 0/1, 0-175 for root 2)
        /// Maps from Ada: Second_Level_Component : Second_Level_Component_Type;
        std::uint8_t secondLevelComponent_;

        /// Additional components beyond the first two
        /// Maps from Ada: Other_Components : Other_Component_Array;
        /// Maximum is 13 additional components (15 total - 2 required = 13)
        std::array<ComponentType, MaximumComponentCount - 2> otherComponents_;

        /// Count of valid entries in otherComponents_
        /// Maps from Ada: Other_Component_Count : Other_Count_Type;
        std::size_t otherComponentCount_;

        // Friend functions to allow access to private members
        friend Status toObjectIdentifier(const ComponentArray& separates, ObjectIdentifier& result);
        friend Status toSeparates(const ObjectIdentifier& identifier,
                                  ComponentArray& result,
                                  std::size_t& numberOfComponents);
        friend ComponentArray toSeparates(const ObjectIdentifier& identifier);
    };

    /// Converts an OID in the form of separate components into an ObjectIdentifier
    ///
    /// Maps from Ada:
    /// ```ada
    /// procedure To_Object_Identifier
    ///   (Separates : in Component_Array; Result : out Object_Identifier; Status : out Status_Type)
    ///   with Depends => ( (Result, Status) => Separates);
    /// ```
    ///
    /// SPARK Contracts:
    ///   - Depends: Result and Status depend only on Separates (pure function)
    ///   - Post: If Status = Success, then Component_Count(Result) = Separates'Length
    ///
    /// @param separates Array of OID components (must have at least 2 elements)
    /// @param result Output parameter for constructed OID
    /// @return Success if OID is valid, error code otherwise
    Status toObjectIdentifier(const ComponentArray& separates, ObjectIdentifier& result);

    /// Converts an ObjectIdentifier back into an array of separate components
    ///
    /// Maps from Ada:
    /// ```ada
    /// procedure To_Separates
    ///   (Identifier           : in  Object_Identifier;
    ///    Result               : out Component_Array;
    ///    Number_Of_Components : out Component_Count_Type)
    ///   with Depends => ( (Result, Number_Of_Components) => (Identifier, Result) );
    /// ```
    ///
    /// SPARK Contracts:
    ///   - Pre: Result'Length >= Component_Count(Identifier)
    ///   - Post: Number_Of_Components = Component_Count(Identifier)
    ///   - Depends: Result and Number_Of_Components depend on Identifier and Result (for array bounds)
    ///
    /// @param identifier OID to convert
    /// @param result Output array (must have sufficient capacity)
    /// @param numberOfComponents Number of components written to result
    /// @return Success if conversion succeeded, InsufficientSpace if result too small
    Status toSeparates(const ObjectIdentifier& identifier,
                      ComponentArray& result,
                      std::size_t& numberOfComponents);

    /// Overload that returns a ComponentArray directly (more idiomatic C++)
    ///
    /// This version always succeeds since it creates the output array sized appropriately.
    ///
    /// @param identifier OID to convert
    /// @return Array of OID components
    ComponentArray toSeparates(const ObjectIdentifier& identifier);

    /// Converts a Component Array to an OID string in dotted notation
    ///
    /// Maps from Ada:
    /// ```ada
    /// procedure OID_To_String
    ///   (Separates  : in  Hermes.OID.Component_Array;
    ///    OID_String : out String);
    /// ```
    ///
    /// Example: {1, 2, 840, 113549} -> "1.2.840.113549"
    ///
    /// @param separates Array of OID components
    /// @return Dotted-decimal string representation
    std::string oidToString(const ComponentArray& separates);

    /// Converts an OID string in dotted notation to a Component Array
    ///
    /// Maps from Ada:
    /// ```ada
    /// procedure String_To_OID
    ///   (OID_String : in  String;
    ///    Separates  : out Hermes.OID.Component_Array);
    /// ```
    ///
    /// Example: "2.16.840.1.101.3.4.2.1" -> {2, 16, 840, 1, 101, 3, 4, 2, 1}
    ///
    /// @param oidString Dotted-decimal OID string
    /// @param separates Output array for parsed components
    /// @return Success if parsing succeeded, error code otherwise
    /// @throws std::invalid_argument if string format is invalid
    Status stringToOID(const std::string& oidString, ComponentArray& separates);

    /// Overload that returns a ComponentArray directly (more idiomatic C++)
    ///
    /// @param oidString Dotted-decimal OID string
    /// @return Array of OID components
    /// @throws std::invalid_argument if string format is invalid
    ComponentArray stringToOID(const std::string& oidString);

} // namespace oid
} // namespace hermes
