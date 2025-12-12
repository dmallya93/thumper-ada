#include "thumper/serial_generator/serial_generator.hpp"

#include <chrono>
#include <ctime>
#include <mutex>

namespace thumper::serial_generator {

namespace {

// Linear Congruential Generator constants from the Ada source.
// See: http://nuclear.llnl.gov/CNP/rng/rngman/node4.html
// This generator has maximal period (2^64).
constexpr SerialNumberType A = 2'862'933'555'777'941'757ULL;
constexpr SerialNumberType B = 3'037'000'493ULL;

// Internal state (SPARK Abstract_State in Ada)
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
// Note: These are intentionally mutable global variables to implement
// the PRNG state. This is required for the serial number generator design.
SerialNumberType current_state = 0;
std::mutex state_mutex;
bool initialized = false;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

} // anonymous namespace

namespace internal {

void initialize_state() {
  // Use system_clock to match Ada.Calendar.Clock
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);

  // Convert to broken-down time (matching Ada's Split operation)
  std::tm tm_now{};
#ifdef _WIN32
  localtime_s(&tm_now, &time_t_now);
#else
  // NOLINTNEXTLINE(cert-err33-c)
  (void)localtime_r(&time_t_now, &tm_now);
#endif

  // Extract time components
  // tm_year is years since 1900, tm_mon is 0-11
  int year = tm_now.tm_year + 1900; // Convert to actual year
  int month = tm_now.tm_mon + 1;    // Convert to 1-12
  int day = tm_now.tm_mday;         // 1-31

  // Calculate seconds since midnight from time components
  int seconds = (tm_now.tm_hour * 3600) + (tm_now.tm_min * 60) + tm_now.tm_sec;

  // Store the date/time into a 64-bit value matching Ada's layout:
  // Year (9 bits, offset from year 1901 to match Ada.Calendar.Year_Number'First)
  // Month (4 bits)
  // Day (5 bits)
  // Seconds (remaining bits)
  //
  // Ada Year_Number'First is 1901, so we offset from that.
  constexpr int YEAR_OFFSET = 1901;

  current_state = static_cast<SerialNumberType>(year - YEAR_OFFSET) << 54 |
                  static_cast<SerialNumberType>(month) << 50 |
                  static_cast<SerialNumberType>(day) << 45 | static_cast<SerialNumberType>(seconds);
}

SerialNumberType get_state_for_testing() {
  std::lock_guard<std::mutex> lock(state_mutex);
  return current_state;
}

void set_state_for_testing(SerialNumberType state) {
  std::lock_guard<std::mutex> lock(state_mutex);
  current_state = state;
  initialized = true;
}

} // namespace internal

SerialNumberType next() {
  std::lock_guard<std::mutex> lock(state_mutex);

  // Lazy initialization on first call (matching Ada's package initialization)
  if (!initialized) {
    internal::initialize_state();
    initialized = true;
  }

  // Linear Congruential Generator: X_{n+1} = (A * X_n + B) mod 2^64
  // The mod 2^64 is implicit due to uint64_t overflow behavior
  current_state = A * current_state + B;

  return current_state;
}

} // namespace thumper::serial_generator
