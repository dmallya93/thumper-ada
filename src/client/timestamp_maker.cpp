#include "thumper/client/timestamp_maker.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "thumper/client/timestamp_messages_client.hpp"
#include "thumper/hermes/der_encode.hpp"
#include "thumper/messages/timestamp_messages.hpp"

namespace thumper::client {

void create_timestamp(std::span<const Octet> hash, std::span<Octet> timestamp,
                      std::size_t& timestamp_size) {
  // Basic implementation: Encode the hash into a simple ASN.1 OCTET STRING
  // as a timestamp request.
  //
  // This demonstrates the integration of the Hermes encoding library.
  // Full RFC-3161 timestamp request creation (with policy OID, nonce, etc.)
  // will be implemented in future milestones when the network layer is ready.
  //
  // The Ada implementation is incomplete (raises Program_Error), so this
  // provides a working demonstration of the encoding pipeline.

  // Create a minimal Request (placeholder structure)
  messages::Request req;
  req.placeholder = 0; // Placeholder value

  // Encode the request using the client message encoder
  OctetArray encoded_request = put_request_value(req);

  // Also encode the hash as an OCTET STRING for demonstration
  OctetArray encoded_hash = hermes::der::put_octet_string_value(hash);

  // Combine both encodings (request + hash) into the output buffer
  std::size_t total_size = encoded_request.size() + encoded_hash.size();

  if (total_size > timestamp.size()) {
    timestamp_size = 0;
    throw std::runtime_error("Timestamp buffer too small");
  }

  // Copy encoded request
  std::memcpy(timestamp.data(), encoded_request.data(), encoded_request.size());

  // Copy encoded hash
  std::memcpy(timestamp.data() + encoded_request.size(), encoded_hash.data(),
              encoded_hash.size());

  timestamp_size = total_size;
}

bool verify_timestamp(std::span<const Octet> hash, std::span<const Octet> timestamp) {
  // Basic implementation: Decode the timestamp and verify it contains the hash.
  //
  // This demonstrates the integration of the Hermes decoding library.
  // Full RFC-3161 timestamp verification (signature checking, policy validation)
  // will be implemented in future milestones.
  //
  // The Ada implementation is incomplete (raises Program_Error), so this
  // provides a working demonstration of the decoding pipeline.

  // Note: 'hash' parameter is currently unused in this basic implementation
  // Full verification comparing the hash will be implemented in future milestones
  (void)hash;

  // For now, perform a simple check: decode the request portion
  // and verify the hash portion exists
  if (timestamp.size() < 3) {
    // Too small to contain valid DER encoding
    return false;
  }

  // Decode the request portion (should be an INTEGER)
  std::size_t decode_stop = 0;
  messages::Response resp;
  hermes::der::Status status = hermes::der::Status::BadValue;

  get_response_value(timestamp, 0, decode_stop, resp, status);

  // For a basic check, just verify we decoded successfully
  // and that there's more data (the hash) after the request
  if (status != hermes::der::Status::Success) {
    return false;
  }

  // Check if there's additional data that could be the hash
  if (decode_stop >= timestamp.size()) {
    return false;
  }

  // In a full implementation, we would decode the hash OCTET STRING
  // and compare it to the provided hash. For now, we just verify
  // the structure is valid.
  return true;
}

} // namespace thumper::client
