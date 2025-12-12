#include "thumper/crypto/cryptographic_services.hpp"

#include <cstring>
#include <mutex>
#include <stdexcept>
#include <vector>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

namespace thumper::crypto {

namespace {

// Get OpenSSL error string for debugging
std::string get_openssl_error() {
  unsigned long err = ERR_get_error();
  if (err == 0) {
    return "Unknown OpenSSL error";
  }
  std::array<char, 256> buf{};
  ERR_error_string_n(err, buf.data(), buf.size());
  return {buf.data()};
}

// Global state for RSA key management (singleton pattern)
class KeyManager {
public:
  static KeyManager& instance() {
    static KeyManager mgr;
    return mgr;
  }

  // Initialize the key from a PEM file (thread-safe)
  Status initialize(const std::string& key_file_path) {
    std::lock_guard<std::mutex> lock(mutex_);

    // If already initialized, return success
    if (key_ != nullptr) {
      return Status::Success;
    }

    // Open the key file using RAII wrapper
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    FILE* key_file = fopen(key_file_path.c_str(), "r");
    if (key_file == nullptr) {
      return Status::BadKey;
    }

    // Read the private key
    EVP_PKEY* pkey = PEM_read_PrivateKey(key_file, nullptr, nullptr, nullptr);
    // NOLINTNEXTLINE(cert-err33-c,cppcoreguidelines-owning-memory)
    (void)fclose(key_file);

    if (pkey == nullptr) {
      return Status::BadKey;
    }

    key_.reset(pkey);
    return Status::Success;
  }

  // Get the loaded key (returns nullptr if not initialized)
  EVP_PKEY* get_key() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return key_.get();
  }

  // Delete copy and move (public as per modernize-use-equals-delete)
  KeyManager(const KeyManager&) = delete;
  KeyManager& operator=(const KeyManager&) = delete;
  KeyManager(KeyManager&&) = delete;
  KeyManager& operator=(KeyManager&&) = delete;

private:
  KeyManager() = default;
  ~KeyManager() = default;

  mutable std::mutex mutex_;

  // Custom deleter for EVP_PKEY
  struct EVPKeyDeleter {
    void operator()(EVP_PKEY* key) const {
      if (key != nullptr) {
        EVP_PKEY_free(key);
      }
    }
  };

  std::unique_ptr<EVP_PKEY, EVPKeyDeleter> key_;
};

} // anonymous namespace

// Implementation structure for SHA256Context (pimpl idiom)
struct SHA256ContextImpl {
  EVP_MD_CTX* ctx;

  SHA256ContextImpl() : ctx(EVP_MD_CTX_new()) {
    if (ctx == nullptr) {
      throw std::runtime_error("Failed to create EVP_MD_CTX: " + get_openssl_error());
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
      EVP_MD_CTX_free(ctx);
      throw std::runtime_error("Failed to initialize SHA-256 context: " + get_openssl_error());
    }
  }

  ~SHA256ContextImpl() {
    if (ctx != nullptr) {
      EVP_MD_CTX_free(ctx);
    }
  }

  // Delete copy operations
  SHA256ContextImpl(const SHA256ContextImpl&) = delete;
  SHA256ContextImpl& operator=(const SHA256ContextImpl&) = delete;

  // Move operations
  SHA256ContextImpl(SHA256ContextImpl&& other) noexcept : ctx(other.ctx) { other.ctx = nullptr; }

  SHA256ContextImpl& operator=(SHA256ContextImpl&& other) noexcept {
    if (this != &other) {
      if (ctx != nullptr) {
        EVP_MD_CTX_free(ctx);
      }
      ctx = other.ctx;
      other.ctx = nullptr;
    }
    return *this;
  }
};

// SHA256Context implementation

SHA256Context::SHA256Context() : impl_(std::make_unique<SHA256ContextImpl>()) {}

SHA256Context::~SHA256Context() = default;

SHA256Context::SHA256Context(SHA256Context&& other) noexcept = default;

SHA256Context& SHA256Context::operator=(SHA256Context&& other) noexcept = default;

void SHA256Context::update(std::span<const Octet> data) {
  if (!impl_ || impl_->ctx == nullptr) {
    throw std::runtime_error("SHA256Context not initialized");
  }

  // Handle empty data (valid in Ada implementation)
  if (data.empty()) {
    return;
  }

  if (EVP_DigestUpdate(impl_->ctx, data.data(), data.size()) != 1) {
    throw std::runtime_error("Failed to update SHA-256 hash: " + get_openssl_error());
  }
}

SHA256Hash SHA256Context::finalize() {
  if (!impl_ || impl_->ctx == nullptr) {
    throw std::runtime_error("SHA256Context not initialized");
  }

  SHA256Hash hash{};
  unsigned int hash_len = 0;

  if (EVP_DigestFinal_ex(impl_->ctx, hash.data(), &hash_len) != 1) {
    throw std::runtime_error("Failed to finalize SHA-256 hash: " + get_openssl_error());
  }

  if (hash_len != 32) {
    throw std::runtime_error("Unexpected SHA-256 hash length: " + std::to_string(hash_len));
  }

  // Re-initialize the context for potential reuse
  // (matching Ada semantics where Context is in out parameter)
  if (EVP_DigestInit_ex(impl_->ctx, EVP_sha256(), nullptr) != 1) {
    throw std::runtime_error("Failed to re-initialize SHA-256 context: " + get_openssl_error());
  }

  return hash;
}

// RSA key management and signature functions

Status initialize_key(const std::string& key_file_path) {
  return KeyManager::instance().initialize(key_file_path);
}

OctetArray make_signature(std::span<const Octet> data) {
  EVP_PKEY* key = KeyManager::instance().get_key();
  if (key == nullptr) {
    throw std::runtime_error("No RSA key loaded. Call initialize_key() first.");
  }

  // Create signature context
  EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
  if (md_ctx == nullptr) {
    throw std::runtime_error("Failed to create signature context: " + get_openssl_error());
  }

  // RAII wrapper for cleanup
  struct CtxDeleter {
    void operator()(EVP_MD_CTX* ctx) const {
      if (ctx != nullptr) {
        EVP_MD_CTX_free(ctx);
      }
    }
  };
  std::unique_ptr<EVP_MD_CTX, CtxDeleter> ctx_guard(md_ctx);

  // Initialize signing operation with SHA-256
  if (EVP_DigestSignInit(md_ctx, nullptr, EVP_sha256(), nullptr, key) != 1) {
    throw std::runtime_error("Failed to initialize signing operation: " + get_openssl_error());
  }

  // Update with data to sign
  if (EVP_DigestSignUpdate(md_ctx, data.data(), data.size()) != 1) {
    throw std::runtime_error("Failed to update signature with data: " + get_openssl_error());
  }

  // Determine signature length
  size_t sig_len = 0;
  if (EVP_DigestSignFinal(md_ctx, nullptr, &sig_len) != 1) {
    throw std::runtime_error("Failed to determine signature length: " + get_openssl_error());
  }

  // Allocate buffer and compute signature
  OctetArray signature(sig_len);
  if (EVP_DigestSignFinal(md_ctx, signature.data(), &sig_len) != 1) {
    throw std::runtime_error("Failed to compute signature: " + get_openssl_error());
  }

  // Resize to actual signature length (may be smaller than allocated)
  signature.resize(sig_len);

  return signature;
}

bool verify_signature(std::span<const Octet> data, std::span<const Octet> signature,
                      const std::string& public_key_path) {
  // Load the public key
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
  FILE* key_file = fopen(public_key_path.c_str(), "r");
  if (key_file == nullptr) {
    throw std::runtime_error("Failed to open public key file: " + public_key_path);
  }

  EVP_PKEY* pkey = PEM_read_PUBKEY(key_file, nullptr, nullptr, nullptr);
  // NOLINTNEXTLINE(cert-err33-c,cppcoreguidelines-owning-memory)
  (void)fclose(key_file);

  if (pkey == nullptr) {
    throw std::runtime_error("Failed to read public key: " + get_openssl_error());
  }

  // RAII wrapper for key cleanup
  struct KeyDeleter {
    void operator()(EVP_PKEY* key) const {
      if (key != nullptr) {
        EVP_PKEY_free(key);
      }
    }
  };
  std::unique_ptr<EVP_PKEY, KeyDeleter> key_guard(pkey);

  // Create verification context
  EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
  if (md_ctx == nullptr) {
    throw std::runtime_error("Failed to create verification context: " + get_openssl_error());
  }

  // RAII wrapper for context cleanup
  struct CtxDeleter {
    void operator()(EVP_MD_CTX* ctx) const {
      if (ctx != nullptr) {
        EVP_MD_CTX_free(ctx);
      }
    }
  };
  std::unique_ptr<EVP_MD_CTX, CtxDeleter> ctx_guard(md_ctx);

  // Initialize verification operation with SHA-256
  if (EVP_DigestVerifyInit(md_ctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) {
    throw std::runtime_error("Failed to initialize verification operation: " + get_openssl_error());
  }

  // Update with data to verify
  if (EVP_DigestVerifyUpdate(md_ctx, data.data(), data.size()) != 1) {
    throw std::runtime_error("Failed to update verification with data: " + get_openssl_error());
  }

  // Verify the signature
  int result = EVP_DigestVerifyFinal(md_ctx, signature.data(), signature.size());
  return result == 1; // 1 = success, 0 = verification failed, <0 = error
}

} // namespace thumper::crypto
