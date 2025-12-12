// NOLINTBEGIN(readability-math-missing-parentheses)
// Note: LCG formula A*X+B is standard notation and doesn't need extra parentheses
// NOLINTBEGIN(clang-diagnostic-sign-conversion)
// Note: Test code uses int for loop counters which may be converted to size_t

#include <set>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "thumper/serial_generator/serial_generator.hpp"

using namespace thumper::serial_generator;

// Test that serial numbers are generated
TEST(SerialGeneratorTest, GeneratesSerialNumbers) {
  SerialNumberType num1 = next();
  SerialNumberType num2 = next();

  // Both should be non-zero (extremely unlikely to get 0)
  EXPECT_NE(num1, 0);
  EXPECT_NE(num2, 0);

  // Numbers should be different
  EXPECT_NE(num1, num2);
}

// Test that the LCG produces a deterministic sequence
TEST(SerialGeneratorTest, DeterministicSequence) {
  // Set a known state
  constexpr SerialNumberType TEST_SEED = 42;
  internal::set_state_for_testing(TEST_SEED);

  // LCG formula: X_{n+1} = (A * X_n + B) mod 2^64
  constexpr SerialNumberType A = 2'862'933'555'777'941'757ULL;
  constexpr SerialNumberType B = 3'037'000'493ULL;

  // Generate expected values
  SerialNumberType expected1 = A * TEST_SEED + B;
  SerialNumberType expected2 = A * expected1 + B;
  SerialNumberType expected3 = A * expected2 + B;

  // Generate actual values
  SerialNumberType actual1 = next();
  SerialNumberType actual2 = next();
  SerialNumberType actual3 = next();

  EXPECT_EQ(actual1, expected1);
  EXPECT_EQ(actual2, expected2);
  EXPECT_EQ(actual3, expected3);
}

// Test that serial numbers are unique in a sequence
TEST(SerialGeneratorTest, UniquenessInSequence) {
  // Reset to a known state
  internal::set_state_for_testing(12345);

  std::set<SerialNumberType> seen;
  constexpr int NUM_SAMPLES = 10000;

  for (int i = 0; i < NUM_SAMPLES; ++i) {
    SerialNumberType num = next();
    EXPECT_EQ(seen.count(num), 0) << "Duplicate serial number: " << num;
    seen.insert(num);
  }

  EXPECT_EQ(seen.size(), NUM_SAMPLES);
}

// Test thread safety: multiple threads calling next() concurrently
TEST(SerialGeneratorTest, ThreadSafety) {
  // Reset to a known state
  internal::set_state_for_testing(99999);

  constexpr int NUM_THREADS = 10;
  constexpr int NUMS_PER_THREAD = 1000;
  std::vector<std::thread> threads;
  threads.reserve(NUM_THREADS);
  std::vector<std::vector<SerialNumberType>> results(NUM_THREADS);

  // Launch threads
  for (int t = 0; t < NUM_THREADS; ++t) {
    threads.emplace_back([&results, t]() {
      for (int i = 0; i < NUMS_PER_THREAD; ++i) {
        results[static_cast<std::size_t>(t)].push_back(next());
      }
    });
  }

  // Wait for all threads to complete
  for (auto& thread : threads) {
    thread.join();
  }

  // Collect all serial numbers
  std::set<SerialNumberType> all_numbers;
  for (const auto& thread_results : results) {
    for (SerialNumberType num : thread_results) {
      all_numbers.insert(num);
    }
  }

  // All numbers should be unique (no duplicates from race conditions)
  EXPECT_EQ(all_numbers.size(), NUM_THREADS * NUMS_PER_THREAD)
      << "Thread safety violation: duplicate serial numbers detected";
}

// Test that the generator cycles with period 2^64 (partial test)
// We can't test the full period, but we can verify the LCG properties
TEST(SerialGeneratorTest, LCGProperties) {
  // The LCG constants A and B give maximal period for modulus 2^64
  // We verify the formula works correctly
  constexpr SerialNumberType A = 2'862'933'555'777'941'757ULL;
  constexpr SerialNumberType B = 3'037'000'493ULL;

  // Test with a few seed values
  std::vector<SerialNumberType> seeds = {0, 1, 100, 0xFFFFFFFFFFFFFFFFULL};

  for (SerialNumberType seed : seeds) {
    internal::set_state_for_testing(seed);
    SerialNumberType result = next();
    SerialNumberType expected = A * seed + B;
    EXPECT_EQ(result, expected) << "LCG formula failed for seed " << seed;
  }
}

// Test that seeding produces different sequences on different "boot times"
// This is a conceptual test since we can't easily control system time
TEST(SerialGeneratorTest, SeedingProducesVariation) {
  // Initialize twice and check that we get different sequences
  // (This assumes initialize_state() uses current time)

  internal::initialize_state();
  SerialNumberType seq1_num1 = next();
  SerialNumberType seq1_num2 = next();

  // Wait enough time to ensure clock changes (1 second for reliable difference)
  std::this_thread::sleep_for(std::chrono::seconds(1));

  internal::initialize_state();
  SerialNumberType seq2_num1 = next();
  SerialNumberType seq2_num2 = next();

  // The sequences should be different (high probability with 1 second delay)
  // Note: This test verifies that the seeding mechanism uses time,
  // which provides different initial states across system restarts
  bool different = (seq1_num1 != seq2_num1) || (seq1_num2 != seq2_num2);
  EXPECT_TRUE(different)
      << "Expected different sequences after re-initialization with 1-second delay. "
      << "seq1_num1=" << seq1_num1 << ", seq2_num1=" << seq2_num1;
}

// Test overflow behavior (implicit mod 2^64)
TEST(SerialGeneratorTest, OverflowBehavior) {
  // Set state to a value near maximum
  constexpr SerialNumberType NEAR_MAX = 0xFFFFFFFFFFFFFF00ULL;
  internal::set_state_for_testing(NEAR_MAX);

  // Generate a few numbers - should wrap around without crashing
  SerialNumberType num1 = next();
  SerialNumberType num2 = next();
  SerialNumberType num3 = next();

  // All should be valid (non-crashing is the main test)
  EXPECT_NE(num1, NEAR_MAX);
  EXPECT_NE(num2, num1);
  EXPECT_NE(num3, num2);
}

// Test get_state_for_testing function
TEST(SerialGeneratorTest, GetStateForTesting) {
  constexpr SerialNumberType TEST_STATE = 777777;
  internal::set_state_for_testing(TEST_STATE);

  SerialNumberType retrieved_state = internal::get_state_for_testing();
  EXPECT_EQ(retrieved_state, TEST_STATE);

  // After calling next(), state should change
  next();
  SerialNumberType new_state = internal::get_state_for_testing();
  EXPECT_NE(new_state, TEST_STATE);
}

// NOLINTEND(clang-diagnostic-sign-conversion)
// NOLINTEND(readability-math-missing-parentheses)
