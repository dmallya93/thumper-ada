#include "thumper/messages/messages.hpp"

#include <algorithm>
#include <stdexcept>

namespace thumper::messages {

NetworkMessage::NetworkMessage(const network::OctetArray& vec, std::size_t msg_size)
    : size(msg_size) {
  if (msg_size > MAX_MESSAGE_SIZE) {
    throw std::out_of_range("Message size exceeds maximum");
  }
  if (msg_size > vec.size()) {
    throw std::out_of_range("Message size exceeds vector size");
  }
  std::copy_n(vec.begin(), msg_size, data.begin());
  // Zero-fill remaining octets
  std::fill(data.begin() + static_cast<std::ptrdiff_t>(msg_size), data.end(), 0);
}

Message::Message(const OctetArray& vec, std::size_t msg_size) : size(msg_size) {
  if (msg_size > MAX_MESSAGE_SIZE) {
    throw std::out_of_range("Message size exceeds maximum");
  }
  if (msg_size > vec.size()) {
    throw std::out_of_range("Message size exceeds vector size");
  }
  std::copy_n(vec.begin(), msg_size, data.begin());
  // Zero-fill remaining octets
  std::fill(data.begin() + static_cast<std::ptrdiff_t>(msg_size), data.end(), 0);
}

Message from_network(const NetworkMessage& low_level) {
  Message high_level;
  high_level.size = low_level.size;

  // Copy used octets using std::copy_n to avoid array indexing warnings
  std::copy_n(low_level.data.begin(), low_level.size, high_level.data.begin());

  // Zero-fill unused octets (matching Ada postcondition)
  std::fill(high_level.data.begin() + static_cast<std::ptrdiff_t>(low_level.size),
            high_level.data.end(), 0);

  return high_level;
}

NetworkMessage to_network(const Message& high_level) {
  NetworkMessage low_level;
  low_level.size = high_level.size;

  // Copy used octets using std::copy_n to avoid array indexing warnings
  std::copy_n(high_level.data.begin(), high_level.size, low_level.data.begin());

  // Zero-fill unused octets (matching Ada postcondition)
  std::fill(low_level.data.begin() + static_cast<std::ptrdiff_t>(high_level.size),
            low_level.data.end(), 0);

  return low_level;
}

} // namespace thumper::messages
