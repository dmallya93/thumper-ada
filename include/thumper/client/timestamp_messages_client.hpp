#ifndef THUMPER_CLIENT_TIMESTAMP_MESSAGES_CLIENT_HPP
#define THUMPER_CLIENT_TIMESTAMP_MESSAGES_CLIENT_HPP

#include <span>

#include "thumper/hermes/der.hpp"
#include "thumper/messages/timestamp_messages.hpp"
#include "thumper/types.hpp"

// Client-side timestamp message encoding and decoding.
//
// This module provides functions for encoding and decoding timestamp protocol
// messages specifically for client use. It handles the ASN.1 DER encoding of
// timestamp requests and decoding of timestamp responses.
//
// Design Note: This corresponds to the Ada package Timestamp_Messages.Client
// which is a child package providing client-specific operations.
//
// All functions use Hermes DER encoding/decoding conventions:
// - Decoding functions take a message buffer, start index, and return a stop
//   index and status
// - Encoding functions return an OctetArray containing the DER-encoded result
//
// Thread Safety: All functions are thread-safe (no shared state).

namespace thumper::client {

// Decodes a Timestamp from a DER-encoded octet sequence.
//
// This function extracts a Timestamp structure from a DER-encoded message.
// The behavior follows the same pattern as other Hermes DER decoding
// procedures.
//
// @param message The DER-encoded message buffer
// @param start The starting index in the message (must be within message
// bounds)
// @param stop Output parameter: index of the last octet processed + 1
// @param stamp Output parameter: the decoded Timestamp structure
// @param status Output parameter: Success or error code
//
// Preconditions:
// - start must be within message bounds (start < message.size())
//
// SPARK Contracts:
// - Global => null (no global state)
// - Depends => ((stop, stamp, status) => (message, start))
//
// Corresponds to Ada's Get_Timestamp_Value procedure.
void get_timestamp_value(std::span<const Octet> message, std::size_t start, std::size_t& stop,
                         messages::Timestamp& stamp, hermes::der::Status& status);

// Encodes a Request to a DER-encoded octet sequence.
//
// This function converts a Request structure into DER-encoded format suitable
// for network transmission. The behavior follows the same pattern as other
// Hermes DER encoding procedures.
//
// @param req The Request structure to encode
// @return An OctetArray containing the DER-encoded request
//
// Note: In the Ada source, this returns an unconstrained Octet_Array. In C++,
// we return an OctetArray (std::vector<Octet>) which provides dynamic sizing.
//
// SPARK Contracts:
// - Global => null (no global state)
//
// Corresponds to Ada's Put_Request_Value function.
OctetArray put_request_value(const messages::Request& req);

// Decodes a Response from a DER-encoded octet sequence.
//
// This function extracts a Response structure from a DER-encoded message.
// The behavior follows the same pattern as other Hermes DER decoding
// procedures.
//
// @param message The DER-encoded message buffer
// @param start The starting index in the message (must be within message
// bounds)
// @param stop Output parameter: index of the last octet processed + 1
// @param resp Output parameter: the decoded Response structure
// @param status Output parameter: Success or error code
//
// Preconditions:
// - start must be within message bounds (start < message.size())
//
// SPARK Contracts:
// - Global => null (no global state)
// - Depends => ((stop, resp, status) => (message, start))
//
// Corresponds to Ada's Get_Response_Value procedure.
void get_response_value(std::span<const Octet> message, std::size_t start, std::size_t& stop,
                        messages::Response& resp, hermes::der::Status& status);

} // namespace thumper::client

#endif // THUMPER_CLIENT_TIMESTAMP_MESSAGES_CLIENT_HPP
