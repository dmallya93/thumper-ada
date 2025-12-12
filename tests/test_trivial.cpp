#include <gtest/gtest.h>

#include "thumper/types.hpp"

namespace thumper {
namespace {

// Test that the basic type definitions are usable
TEST(TrivialTest, OctetTypeDefinition) {
  Octet byte = 0x42;
  EXPECT_EQ(byte, 0x42);
  EXPECT_EQ(sizeof(Octet), 1);
}

// Test that OctetArray is usable
TEST(TrivialTest, OctetArrayCreation) {
  OctetArray array{0x01, 0x02, 0x03, 0x04};
  EXPECT_EQ(array.size(), 4);
  EXPECT_EQ(array[0], 0x01);
  EXPECT_EQ(array[3], 0x04);
}

// Test that OctetArray supports dynamic operations
TEST(TrivialTest, OctetArrayDynamic) {
  OctetArray array;
  EXPECT_TRUE(array.empty());

  array.push_back(0xAB);
  array.push_back(0xCD);
  EXPECT_EQ(array.size(), 2);
  EXPECT_EQ(array[0], 0xAB);
  EXPECT_EQ(array[1], 0xCD);
}

// Test that we can verify the build system works
TEST(TrivialTest, BuildSystemWorks) {
  EXPECT_TRUE(true);
}

} // namespace
} // namespace thumper
