#include "thumper/client/timestamp_messages_client.hpp"

#include "thumper/hermes/der_decode.hpp"
#include "thumper/hermes/der_encode.hpp"

namespace thumper::client {

void get_timestamp_value(std::span<const Octet> message, std::size_t start, std::size_t& stop,
                         messages::Timestamp& stamp, hermes::der::Status& status) {
  // Basic implementation: Since Request/Response are placeholders in the Ada
  // source, this provides minimal decoding for demonstration.
  // Full RFC-3161 decoding will be implemented in future milestones.

  // For now, just set status to Success and return (minimal valid implementation)
  (void)message;
  (void)stamp;

  stop = start;
  status = hermes::der::Status::Success;
}

OctetArray put_request_value(const messages::Request& req) {
  // Basic implementation: Since Request is a placeholder with just an integer
  // field, encode it as a simple INTEGER in DER format.
  // Full RFC-3161 request encoding will be implemented in future milestones.

  return hermes::der::put_integer_value(req.placeholder);
}

void get_response_value(std::span<const Octet> message, std::size_t start, std::size_t& stop,
                        messages::Response& resp, hermes::der::Status& status) {
  // Basic implementation: Since Response is a placeholder, provide minimal
  // decoding for demonstration.
  // Full RFC-3161 response decoding will be implemented in future milestones.

  // Try to decode an INTEGER (matching put_request_value format)
  int value = 0;
  hermes::der::get_integer_value(message, start, stop, value, status);

  if (status == hermes::der::Status::Success) {
    resp.placeholder = value;
  }
}

} // namespace thumper::client
