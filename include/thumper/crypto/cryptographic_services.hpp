#ifndef THUMPER_CRYPTO_CRYPTOGRAPHIC_SERVICES_HPP
#define THUMPER_CRYPTO_CRYPTOGRAPHIC_SERVICES_HPP

#include <array>
#include <memory>
#include <span>
#include <string>

#include "thumper/types.hpp"

namespace thumper::crypto {

// Status type for cryptographic operations that can fail.
// Corresponds to Ada's "Status_Type is (Success, Bad_Key)"
enum class Status { Success, BadKey };

// SHA-256 hash output type.
// Fixed-size array holding the 32-byte (256-bit) hash value.
// Corresponds to Ada's "subtype SHA256_Hash_Type is Hermes.Octet_Array(0 ..
// 31)"
using SHA256Hash = std::array<Octet, 32>;

// Forward declaration for OpenSSL context (pimpl idiom for ABI stability)
struct SHA256ContextImpl;

// SHA-256 hashing context for streaming hash computation.
//
// This class provides a streaming interface for computing SHA-256 hashes,
// allowing data to be hashed in chunks without loading entire files into
// memory. It uses RAII for automatic resource management.
//
// Usage:
//   SHA256Context ctx;
//   ctx.update(data_chunk_1);
//   ctx.update(data_chunk_2);
//   SHA256Hash hash = ctx.finalize();
//
// This corresponds to Ada's SHA256_CTX type and the three-phase API:
// - Initialize_Hash (constructor)
// - Update_Hash (update method)
// - Finalize_Hash (finalize method)
//
// Thread Safety: Not thread-safe. Each thread should use its own context.
// SPARK Contracts: Global => null (no global state, fully self-contained)
class SHA256Context {
public:
  // Constructs and initializes a new SHA-256 context.
  // Corresponds to Ada's Initialize_Hash procedure.
  //
  // @throws std::runtime_error if OpenSSL initialization fails
  SHA256Context();

  // Destructor - cleans up OpenSSL resources.
  ~SHA256Context();

  // Move constructor and assignment (delete copy for safety)
  SHA256Context(SHA256Context&& other) noexcept;
  SHA256Context& operator=(SHA256Context&& other) noexcept;

  // Delete copy operations to prevent double-free of OpenSSL context
  SHA256Context(const SHA256Context&) = delete;
  SHA256Context& operator=(const SHA256Context&) = delete;

  // Updates the hash with a chunk of data.
  //
  // This method can be called multiple times to hash data incrementally.
  // The data can be of any size (including zero), and the size need not be
  // consistent between calls.
  //
  // Corresponds to Ada's Update_Hash procedure.
  //
  // @param data The data chunk to hash
  // @throws std::runtime_error if OpenSSL update operation fails
  void update(std::span<const Octet> data);

  // Finalizes the hash computation and returns the result.
  //
  // After calling this method, the context is reset and can be reused for
  // a new hash computation by calling update() again.
  //
  // Corresponds to Ada's Finalize_Hash procedure.
  //
  // @return A 32-byte SHA-256 hash value
  // @throws std::runtime_error if OpenSSL finalization fails
  SHA256Hash finalize();

private:
  // Opaque pointer to OpenSSL EVP_MD_CTX (pimpl idiom)
  std::unique_ptr<SHA256ContextImpl> impl_;
};

// Loads the server's RSA private key from the file system.
//
// This function initializes the global RSA key used for signing operations.
// The key is loaded once and stored internally. The function is thread-safe
// and uses lazy initialization.
//
// Corresponds to Ada's Initialize_Key procedure.
//
// @param key_file_path Path to the PEM-encoded RSA private key file
// @return Status::Success if the key was loaded successfully,
//         Status::BadKey if the file doesn't exist or is invalid
//
// Note: In the Ada version, this reads from a hardcoded path. The C++ version
// accepts the path as a parameter for flexibility.
Status initialize_key(const std::string& key_file_path);

// Computes the RSA signature of the given data.
//
// This function signs the provided data using the private key loaded by
// initialize_key(). The signature is computed using RSA with SHA-256 and
// returned in DER-encoded format (PKCS#1 v1.5).
//
// Corresponds to Ada's Make_Signature function.
//
// Preconditions:
// - initialize_key() must have been called successfully before this function
//
// @param data The data to sign
// @return A DER-encoded RSA signature
// @throws std::runtime_error if no key has been loaded or signing fails
//
// SPARK Contract: Global => null (reads from internal package state)
OctetArray make_signature(std::span<const Octet> data);

// Verifies an RSA signature against the given data using a public key.
//
// This function verifies that the provided signature was created by signing
// the data with the corresponding private key. It uses RSA with SHA-256.
//
// @param data The original data that was signed
// @param signature The signature to verify (DER-encoded)
// @param public_key_path Path to the PEM-encoded RSA public key file
// @return true if the signature is valid, false otherwise
// @throws std::runtime_error if the public key cannot be loaded or verification
// fails due to errors
//
// Note: This function is primarily for testing. In production, the server would
// only use make_signature(), and clients would use verify_signature().
bool verify_signature(std::span<const Octet> data, std::span<const Octet> signature,
                      const std::string& public_key_path);

// Helper function: Computes SHA-256 hash of data in one call.
//
// This is a convenience function for cases where all data is available at once
// and streaming is not needed.
//
// @param data The data to hash
// @return A 32-byte SHA-256 hash value
inline SHA256Hash compute_sha256(std::span<const Octet> data) {
  SHA256Context ctx;
  ctx.update(data);
  return ctx.finalize();
}

} // namespace thumper::crypto

#endif // THUMPER_CRYPTO_CRYPTOGRAPHIC_SERVICES_HPP
