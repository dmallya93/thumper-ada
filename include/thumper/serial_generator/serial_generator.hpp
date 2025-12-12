#ifndef THUMPER_SERIAL_GENERATOR_SERIAL_GENERATOR_HPP
#define THUMPER_SERIAL_GENERATOR_SERIAL_GENERATOR_HPP

#include <cstdint>

// This package abstracts the serial number generator for timestamp tokens.
//
// Serial numbers are required to be unique over the lifetime of the system's
// deployment. This is accomplished by generating pseudo-random 64-bit numbers
// using a Linear Congruential Generator (LCG) with maximal period (2^64).
//
// The RNG is seeded using the time of system startup, so when the system
// reboots it will produce a different sequence with high probability. Note
// that the RNG used is not cryptographically secure; the serial numbers it
// produces are predictable if the system boot time is known or can be guessed.
//
// Design Decision: This implements Design Decision #3 from MILESTONE.md
// (SPARK Abstract State to C++ State Management) by using namespace-level
// static state with thread-safe access via std::mutex. The PRNG is seeded
// on first use (lazy initialization with std::call_once).
//
// Thread Safety: The next() function is thread-safe and can be called
// concurrently from multiple threads without risk of duplicate serial numbers
// or data races.

namespace thumper::serial_generator {

// Serial number type (64-bit unsigned integer).
// Corresponds to "type Serial_Number_Type is mod 2**64" in Ada.
using SerialNumberType = std::uint64_t;

// Computes the next serial number and updates the internal state to prepare
// for a following call to next(). This function cannot fail. However, the
// numbers it generates cycle with a period of 2^64.
//
// The function is thread-safe and uses internal synchronization to prevent
// concurrent access issues.
//
// The Ada version uses an out parameter: "procedure Next(Number : out Serial_Number_Type)"
// C++ uses a return value for more idiomatic style.
//
// @return The next serial number in the sequence
SerialNumberType next();

// Internal functions (exposed for testing purposes)
namespace internal {

// Seeds the PRNG state with a value derived from the current system time.
// This matches the Ada package initialization block which uses Ada.Calendar.Clock.
//
// The seed encoding matches the Ada implementation:
// - Year (9 bits, offset from Year_Number'First)
// - Month (4 bits)
// - Day (5 bits)
// - Seconds (remaining bits for day duration)
//
// This function is called automatically on the first call to next().
// It can also be called explicitly for testing purposes.
void initialize_state();

// Gets the current state (for testing purposes only).
// This is not part of the public API in Ada (Abstract_State is encapsulated).
SerialNumberType get_state_for_testing();

// Sets the state (for testing purposes only).
// This allows deterministic testing of the LCG sequence.
void set_state_for_testing(SerialNumberType state);

} // namespace internal

} // namespace thumper::serial_generator

#endif // THUMPER_SERIAL_GENERATOR_SERIAL_GENERATOR_HPP
