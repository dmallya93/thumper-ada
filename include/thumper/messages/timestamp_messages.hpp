#ifndef THUMPER_MESSAGES_TIMESTAMP_MESSAGES_HPP
#define THUMPER_MESSAGES_TIMESTAMP_MESSAGES_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include "thumper/hermes/oid.hpp"
#include "thumper/types.hpp"

// This package defines the timestamp protocol message structures.
//
// These structures represent the core data types for RFC-3161 timestamp
// protocol messages. The Timestamp structure contains all fields required
// to create and verify timestamp tokens.
//
// Design Note: The Ada source uses discriminated records for Request and
// Response (placeholders for now). C++ uses simple structs with placeholder
// fields, which will be expanded in future milestones.

namespace thumper::messages {

// Currently only version 1 is supported.
using VersionType = int;
constexpr VersionType VERSION_1 = 1;

// Currently only 256-bit (32 byte) hash values are supported.
// To lift this restriction, Timestamp could be made a template on hash size,
// but that changes it to a less ergonomic type. See the Ada source comments
// for rationale.
constexpr std::size_t HASH_SIZE = 32;

// Forward declaration for serial number type
using SerialNumberType = std::uint64_t;

// Timestamp structure containing all fields for an RFC-3161 timestamp token.
//
// Fields:
// - version: Protocol version (currently only 1 is supported)
// - policy: OID identifying the timestamp policy
// - hash_algorithm: OID of the hash algorithm used (e.g., SHA-256)
// - hashed_message: The hash value of the timestamped document (32 bytes)
// - serial_number: Unique 64-bit identifier for this timestamp
// - generalized_time: Timestamp in ASN.1 GeneralizedTime format "YYYYMMDDHHMMSSZ"
//
// Example generalized_time: "20231215143022Z" (2023-12-15 14:30:22 UTC)
struct Timestamp {
  VersionType version{VERSION_1};
  hermes::oid::ObjectIdentifier policy;
  hermes::oid::ObjectIdentifier hash_algorithm;
  std::array<Octet, HASH_SIZE> hashed_message{};
  SerialNumberType serial_number{0};
  std::string generalized_time; // Format: "YYYYMMDDHHMMSSZ" (15 characters)

  Timestamp() = default;
};

// Request structure (placeholder for future expansion).
//
// In the Ada source, this is a record with a single Placeholder field.
// Future milestones will expand this to contain the full timestamp request
// fields as defined by RFC-3161.
struct Request {
  int placeholder{0};

  Request() = default;
};

// Response structure (placeholder for future expansion).
//
// In the Ada source, this is a record with a single Placeholder field.
// Future milestones will expand this to contain the full timestamp response
// fields as defined by RFC-3161.
struct Response {
  int placeholder{0};

  Response() = default;
};

} // namespace thumper::messages

#endif // THUMPER_MESSAGES_TIMESTAMP_MESSAGES_HPP
