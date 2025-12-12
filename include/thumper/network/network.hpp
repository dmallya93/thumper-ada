#ifndef THUMPER_NETWORK_NETWORK_HPP
#define THUMPER_NETWORK_NETWORK_HPP

#include "thumper/types.hpp"

// This file defines network-level types for handling raw data on the network.
//
// In the Ada source, Network.Octet and Hermes.Octet are distinct types
// (both "mod 2**8") requiring explicit conversion. In C++, we use the same
// underlying type (uint8_t from types.hpp) for both contexts, as type
// distinctions are enforced through namespaces rather than distinct types.
//
// This design simplifies conversions while maintaining clear separation
// between network-level and ASN.1-level operations through namespace boundaries.

namespace thumper::network {

// Type for handling raw data on the network.
// This is the same as thumper::Octet (uint8_t), but scoped in the network
// namespace to indicate its use in network I/O contexts.
using Octet = thumper::Octet;
using OctetArray = thumper::OctetArray;

} // namespace thumper::network

#endif // THUMPER_NETWORK_NETWORK_HPP
