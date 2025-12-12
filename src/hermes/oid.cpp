#include "thumper/hermes/oid.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <sstream>
#include <stdexcept>

namespace thumper::hermes::oid {

// Helper function to validate root and second level components
namespace {

bool is_valid_root(ComponentType root) noexcept {
  return root >= 0 && root <= 2;
}

bool is_valid_second_level(ComponentType root, ComponentType second_level) noexcept {
  if (!is_valid_root(root)) {
    return false;
  }

  // For root 0 or 1, second component must be 0-39
  if (root == 0 || root == 1) {
    return second_level >= 0 && second_level <= 39;
  }

  // For root 2, second component must be 0-175
  // (because 2*40 + 175 = 255, the maximum first octet in DER encoding)
  return second_level >= 0 && second_level <= 175;
}

} // namespace

ObjectIdentifier::ObjectIdentifier() noexcept : components_{}, component_count_(0) {}

ObjectIdentifier::ObjectIdentifier(std::span<const ComponentType> components,
                                   std::size_t count) noexcept
    : components_{}, component_count_(count) {
  std::copy(components.begin(), components.begin() + static_cast<std::ptrdiff_t>(count),
            components_.begin());
}

Status ObjectIdentifier::from_components(std::span<const ComponentType> components,
                                         ObjectIdentifier& result) {
  // Must have at least 2 components
  if (components.size() < 2) {
    result = ObjectIdentifier();
    return Status::InvalidRoot;
  }

  // Cannot exceed maximum component count
  if (components.size() > MAX_COMPONENT_COUNT) {
    result = ObjectIdentifier();
    return Status::InsufficientSpace;
  }

  // Validate root component
  const ComponentType ROOT = components[0];
  if (!is_valid_root(ROOT)) {
    result = ObjectIdentifier();
    return Status::InvalidRoot;
  }

  // Validate second level component
  const ComponentType SECOND_LEVEL = components[1];
  if (!is_valid_second_level(ROOT, SECOND_LEVEL)) {
    result = ObjectIdentifier();
    return Status::InvalidSecondLevel;
  }

  // All components must be non-negative
  for (const auto COMPONENT : components) {
    if (COMPONENT < 0) {
      result = ObjectIdentifier();
      return Status::InvalidRoot;
    }
  }

  // Create the OID
  result = ObjectIdentifier(components, components.size());
  return Status::Success;
}

std::size_t ObjectIdentifier::component_count() const noexcept {
  return component_count_;
}

std::size_t ObjectIdentifier::to_components(std::span<ComponentType> result) const noexcept {
  if (result.size() < component_count_) {
    return 0; // Insufficient space
  }

  std::copy(components_.begin(),
            components_.begin() + static_cast<std::ptrdiff_t>(component_count_), result.begin());
  return component_count_;
}

std::span<const ComponentType> ObjectIdentifier::components() const noexcept {
  return {components_.data(), component_count_};
}

bool ObjectIdentifier::is_valid() const noexcept {
  if (component_count_ < 2 || component_count_ > MAX_COMPONENT_COUNT) {
    return false;
  }

  // Check root component
  if (!is_valid_root(components_[0])) {
    return false;
  }

  // Check second level component
  if (!is_valid_second_level(components_[0], components_[1])) {
    return false;
  }

  // All components must be non-negative
  for (std::size_t i = 0; i < component_count_; ++i) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    if (components_[i] < 0) {
      return false;
    }
  }

  return true;
}

bool ObjectIdentifier::operator==(const ObjectIdentifier& other) const noexcept {
  if (component_count_ != other.component_count_) {
    return false;
  }

  return std::equal(components_.begin(), components_.begin() + component_count_,
                    other.components_.begin());
}

bool ObjectIdentifier::operator!=(const ObjectIdentifier& other) const noexcept {
  return !(*this == other);
}

std::string oid_to_string(std::span<const ComponentType> components) {
  if (components.empty()) {
    return "";
  }

  std::ostringstream oss;
  oss << components[0];

  for (std::size_t i = 1; i < components.size(); ++i) {
    oss << '.' << components[i];
  }

  return oss.str();
}

std::string oid_to_string(const ObjectIdentifier& oid) {
  return oid_to_string(oid.components());
}

void string_to_oid(std::string_view oid_string, std::vector<ComponentType>& result) {
  result.clear();

  if (oid_string.empty()) {
    return;
  }

  std::size_t pos = 0;
  while (pos < oid_string.size()) {
    // Skip whitespace
    while (pos < oid_string.size() &&
           std::isspace(static_cast<unsigned char>(oid_string[pos])) != 0) {
      ++pos;
    }

    if (pos >= oid_string.size()) {
      break;
    }

    // Find the end of the number (next dot or end of string)
    std::size_t end = pos;
    while (end < oid_string.size() && oid_string[end] != '.') {
      ++end;
    }

    // Parse the number
    const std::string_view NUMBER_STR = oid_string.substr(pos, end - pos);

    // Trim trailing whitespace from number
    std::size_t number_end = NUMBER_STR.size();
    while (number_end > 0 &&
           std::isspace(static_cast<unsigned char>(NUMBER_STR[number_end - 1])) != 0) {
      --number_end;
    }

    const std::string_view TRIMMED_NUMBER = NUMBER_STR.substr(0, number_end);

    if (TRIMMED_NUMBER.empty()) {
      throw std::invalid_argument("Empty component in OID string");
    }

    // Use from_chars for efficient parsing
    ComponentType component = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const auto* first = TRIMMED_NUMBER.data();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const auto* last = first + TRIMMED_NUMBER.size();
    auto [ptr, ec] = std::from_chars(first, last, component);

    if (ec != std::errc() || ptr != last) {
      throw std::invalid_argument("Invalid number in OID string: " + std::string(TRIMMED_NUMBER));
    }

    if (component < 0) {
      throw std::invalid_argument("Negative component in OID string");
    }

    result.push_back(component);

    // Move past the dot
    pos = end;
    if (pos < oid_string.size() && oid_string[pos] == '.') {
      ++pos;
    }
  }
}

Status validate_oid(std::span<const ComponentType> components) {
  // Must have at least 2 components
  if (components.size() < 2) {
    return Status::InvalidRoot;
  }

  // Cannot exceed maximum component count
  if (components.size() > MAX_COMPONENT_COUNT) {
    return Status::InsufficientSpace;
  }

  // Validate root component
  const ComponentType ROOT = components[0];
  if (!is_valid_root(ROOT)) {
    return Status::InvalidRoot;
  }

  // Validate second level component
  const ComponentType SECOND_LEVEL = components[1];
  if (!is_valid_second_level(ROOT, SECOND_LEVEL)) {
    return Status::InvalidSecondLevel;
  }

  // All components must be non-negative
  for (const auto COMPONENT : components) {
    if (COMPONENT < 0) {
      return Status::InvalidRoot;
    }
  }

  return Status::Success;
}

// Ada-style free function API (matching TASK.md specification)

Status to_object_identifier(std::span<const ComponentType> separates, ObjectIdentifier& result) {
  return ObjectIdentifier::from_components(separates, result);
}

std::size_t component_count(const ObjectIdentifier& identifier) {
  return identifier.component_count();
}

void to_separates(const ObjectIdentifier& identifier, std::vector<ComponentType>& result) {
  result.clear();
  const auto COMPONENTS = identifier.components();
  result.assign(COMPONENTS.begin(), COMPONENTS.end());
}

} // namespace thumper::hermes::oid
