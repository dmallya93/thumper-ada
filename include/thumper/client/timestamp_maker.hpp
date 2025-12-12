#ifndef THUMPER_CLIENT_TIMESTAMP_MAKER_HPP
#define THUMPER_CLIENT_TIMESTAMP_MAKER_HPP

#include <span>

#include "thumper/types.hpp"

// Client timestamp creation and verification.
//
// This module encapsulates the work of creating and verifying timestamps on
// the client side. It provides the core logic for constructing timestamp
// requests and validating timestamp responses.
//
// Design Note: This corresponds to the Ada package Client_Timestamp_Maker.
// The Ada implementation is incomplete (raises Program_Error), so this C++
// version provides placeholder implementations that will be completed as
// network communication is added in future milestones.
//
// Thread Safety: All functions are thread-safe (no shared state).

namespace thumper::client {

// Creates a timestamp request for the given document hash.
//
// This function encodes a document hash into an ASN.1 timestamp request
// suitable for sending to a timestamp server. The resulting timestamp is
// written to the provided output buffer.
//
// @param hash The SHA-256 hash of the document to timestamp
// @param timestamp Output buffer for the encoded timestamp request (must be
// large enough)
// @param timestamp_size Output parameter: the actual size of the encoded
// timestamp
//
// Note: The Ada version uses out parameters for both timestamp and
// timestamp_size. In C++, we use std::span for the timestamp buffer and a
// reference for the size.
//
// Future Implementation: This will encode the hash into an RFC-3161 timestamp
// request and (in future milestones) send it to the server. For now, it's a
// placeholder.
//
// Corresponds to Ada's Create_Timestamp procedure.
void create_timestamp(std::span<const Octet> hash, std::span<Octet> timestamp,
                      std::size_t& timestamp_size);

// Verifies that a timestamp is valid for the given document hash.
//
// This function decodes a timestamp response and verifies that it correctly
// corresponds to the provided document hash.
//
// @param hash The SHA-256 hash of the document
// @param timestamp The encoded timestamp response to verify
// @return true if the timestamp is valid for the hash, false otherwise
//
// Future Implementation: This will decode the timestamp response, extract
// the hash, and compare it to the provided hash. For now, it's a placeholder.
//
// Corresponds to Ada's Verify_Timestamp function.
bool verify_timestamp(std::span<const Octet> hash, std::span<const Octet> timestamp);

} // namespace thumper::client

#endif // THUMPER_CLIENT_TIMESTAMP_MAKER_HPP
