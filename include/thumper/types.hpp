#ifndef THUMPER_TYPES_HPP
#define THUMPER_TYPES_HPP

#include <cstdint>
#include <vector>

namespace thumper {

// Fundamental byte type for the Thumper system.
// This corresponds to Ada's "type Octet is mod 2**8" used throughout the
// Hermes ASN.1 library and network stack.
//
// Design Decision: Using uint8_t instead of std::byte for:
// - Compatibility with OpenSSL APIs (which use unsigned char*)
// - Support for arithmetic operations used in DER encoding
// - Clear numeric semantics matching Ada's modular type
// - Interoperability with C APIs
using Octet = std::uint8_t;

// Dynamic array of octets.
// This corresponds to Ada's unconstrained "Octet_Array" type used for
// variable-length buffers in ASN.1 encoding/decoding and network operations.
//
// Uses std::vector for:
// - Automatic memory management (RAII)
// - Dynamic sizing with bounds checking via .at()
// - Standard iterator support
// - Move semantics for efficient transfers
using OctetArray = std::vector<Octet>;

} // namespace thumper

#endif // THUMPER_TYPES_HPP
