#ifndef THUMPER_MESSAGES_MESSAGES_HPP
#define THUMPER_MESSAGES_MESSAGES_HPP

#include <array>
#include <cstddef>

#include "thumper/hermes/hermes.hpp"
#include "thumper/network/network.hpp"
#include "thumper/types.hpp"

// This package defines the basic message types exchanged between network
// and ASN.1 layers.
//
// The Ada source distinguishes between Network.Octet_Array (for raw network
// data) and Hermes.Octet_Array (for ASN.1 data). In C++, both use the same
// underlying type (std::vector<uint8_t>), but the conversion functions maintain
// the architectural boundary between network I/O and ASN.1 processing.
//
// Design Note: The Ada implementation guarantees that unused octets are zeroed
// in conversion results. This C++ implementation preserves that behavior for
// compatibility, though it may not be strictly necessary.

namespace thumper::messages {

// Maximum message size (512 octets as per Ada specification)
constexpr std::size_t MAX_MESSAGE_SIZE = 512;

// Type Network_Message represents the raw data sent/received on the network.
struct NetworkMessage {
  std::array<network::Octet, MAX_MESSAGE_SIZE> data{};
  std::size_t size{0};

  NetworkMessage() = default;
  NetworkMessage(const network::OctetArray& vec, std::size_t msg_size);
};

// Type Message represents the ASN.1 data.
struct Message {
  std::array<Octet, MAX_MESSAGE_SIZE> data{};
  std::size_t size{0};

  Message() = default;
  Message(const OctetArray& vec, std::size_t msg_size);
};

// Converts a network-level message to an ASN.1 message.
//
// This function copies data from the network buffer to the ASN.1 buffer,
// converting between network::Octet and hermes::Octet types (which are
// identical in C++, but conceptually separate in the architecture).
//
// Post-condition (matching Ada):
// - Result.size == low_level.size
// - For i <= low_level.size: Result.data[i] == low_level.data[i]
// - For i > low_level.size: Result.data[i] == 0 (zero-filled)
//
// @param low_level The network message to convert
// @return The converted ASN.1 message
Message from_network(const NetworkMessage& low_level);

// Converts an ASN.1 message to a network-level message.
//
// This function copies data from the ASN.1 buffer to the network buffer,
// converting between hermes::Octet and network::Octet types.
//
// Post-condition (matching Ada):
// - Result.size == high_level.size
// - For i <= high_level.size: Result.data[i] == high_level.data[i]
// - For i > high_level.size: Result.data[i] == 0 (zero-filled)
//
// @param high_level The ASN.1 message to convert
// @return The converted network message
NetworkMessage to_network(const Message& high_level);

} // namespace thumper::messages

#endif // THUMPER_MESSAGES_MESSAGES_HPP
