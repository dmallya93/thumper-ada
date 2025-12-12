#ifndef THUMPER_HERMES_OID_HPP
#define THUMPER_HERMES_OID_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "thumper/types.hpp"

namespace thumper::hermes::oid {

// Maximum number of components allowed in an OID.
// This matches the Ada constant Maximum_Component_Count.
constexpr std::size_t MAX_COMPONENT_COUNT = 15;

// Component type for OID values.
// Each component is a non-negative integer. Ada uses "range 0 .. Integer'Last".
using ComponentType = std::int32_t;

// Status codes returned by OID operations.
// Corresponds to Status_Type in hermes-oid.ads.
enum class Status {
  Success,            // Operation succeeded
  InvalidRoot,        // Root component must be 0, 1, or 2
  InvalidSecondLevel, // Second level component out of valid range
  InsufficientSpace   // Not enough space for the operation
};

// Object Identifier representation.
//
// An OID consists of at least two components:
// - Root component (0, 1, or 2)
// - Second level component (0-39 for root 0 or 1; 0-175 for root 2)
// - Up to 13 additional components
//
// The Ada implementation uses a record with separate fields for root,
// second_level, and other_components. We use a std::array for simplicity
// and to avoid complex index arithmetic.
//
// Design Decision: Using std::array<ComponentType, MaxComponentCount> instead
// of std::vector<ComponentType> because:
// - Fixed maximum size matches Ada semantics
// - No dynamic allocation
// - Better cache locality
// - Simpler memory model
class ObjectIdentifier {
public:
  // Default constructor creates an empty (invalid) OID.
  ObjectIdentifier() noexcept;

  // Creates an OID from a span of components.
  // Returns Status::Success if the OID is valid, error status otherwise.
  static Status from_components(std::span<const ComponentType> components,
                                ObjectIdentifier& result);

  // Returns the number of components in this OID.
  [[nodiscard]] std::size_t component_count() const noexcept;

  // Extracts components into the provided array.
  // Returns the number of components extracted.
  // If the result array is too small, returns 0.
  [[nodiscard]] std::size_t to_components(std::span<ComponentType> result) const noexcept;

  // Returns a view of the components.
  [[nodiscard]] std::span<const ComponentType> components() const noexcept;

  // Validates that the OID satisfies all constraints.
  [[nodiscard]] bool is_valid() const noexcept;

  // Comparison operators
  bool operator==(const ObjectIdentifier& other) const noexcept;
  bool operator!=(const ObjectIdentifier& other) const noexcept;

private:
  // Component storage. Only the first component_count_ elements are valid.
  std::array<ComponentType, MAX_COMPONENT_COUNT> components_;
  std::size_t component_count_;

  // Private constructor used by from_components
  ObjectIdentifier(std::span<const ComponentType> components, std::size_t count) noexcept;
};

// Ada-style free function API (matching TASK.md specification)

// Converts an OID in the form of separate components into an abstract object identifier.
// This corresponds to To_Object_Identifier in hermes-oid.ads.
//
// @param separates The array of OID components
// @param result Output: The constructed ObjectIdentifier
// @return Status indicating success or the type of error
Status to_object_identifier(std::span<const ComponentType> separates, ObjectIdentifier& result);

// Returns the number of components inside the given object identifier.
// This corresponds to Component_Count in hermes-oid.ads.
//
// @param identifier The object identifier to query
// @return The number of components
std::size_t component_count(const ObjectIdentifier& identifier);

// Converts an object identifier into an array of separate components.
// This corresponds to To_Separates in hermes-oid.ads.
//
// @param identifier The object identifier to decompose
// @param result Output vector that will be populated with components
void to_separates(const ObjectIdentifier& identifier, std::vector<ComponentType>& result);

// Converts an OID component array to a dotted string notation.
// Example: [2, 16, 840, 1, 101, 3, 4, 2, 1] -> "2.16.840.1.101.3.4.2.1"
//
// This corresponds to OID_To_String in hermes-oid.adb.
//
// @param components The array of OID components
// @return A string in dotted notation
std::string oid_to_string(std::span<const ComponentType> components);

// Converts an OID to a dotted string notation.
// Convenience function that operates on an ObjectIdentifier.
//
// @param oid The object identifier
// @return A string in dotted notation
std::string oid_to_string(const ObjectIdentifier& oid);

// Parses an OID string in dotted notation into component array.
// Example: "2.16.840.1.101.3.4.2.1" -> [2, 16, 840, 1, 101, 3, 4, 2, 1]
//
// This corresponds to String_To_OID in hermes-oid.adb.
//
// @param oid_string The OID string to parse (e.g., "2.5.4.3")
// @param result Output vector that will be populated with components
// @throws std::invalid_argument if the string is malformed
void string_to_oid(std::string_view oid_string, std::vector<ComponentType>& result);

// Validates OID constraints.
// - Root component must be 0, 1, or 2
// - If root is 0 or 1, second component must be 0-39
// - If root is 2, second component must be 0-175
//
// @param components The component array to validate
// @return Status indicating whether the OID is valid
Status validate_oid(std::span<const ComponentType> components);

} // namespace thumper::hermes::oid

#endif // THUMPER_HERMES_OID_HPP
