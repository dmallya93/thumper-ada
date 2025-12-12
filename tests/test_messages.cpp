// NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
// Note: Test code uses variable array indices for verification, which is acceptable

#include <gtest/gtest.h>

#include "thumper/messages/messages.hpp"
#include "thumper/messages/timestamp_messages.hpp"

using namespace thumper::messages;

// Test basic message conversion from network to ASN.1
TEST(MessagesTest, FromNetworkBasic) {
  NetworkMessage net_msg;
  net_msg.size = 5;
  net_msg.data[0] = 0x01;
  net_msg.data[1] = 0x02;
  net_msg.data[2] = 0x03;
  net_msg.data[3] = 0x04;
  net_msg.data[4] = 0x05;

  Message msg = from_network(net_msg);

  EXPECT_EQ(msg.size, 5);
  EXPECT_EQ(msg.data[0], 0x01);
  EXPECT_EQ(msg.data[1], 0x02);
  EXPECT_EQ(msg.data[2], 0x03);
  EXPECT_EQ(msg.data[3], 0x04);
  EXPECT_EQ(msg.data[4], 0x05);

  // Verify unused octets are zeroed (matching Ada postcondition)
  for (std::size_t i = 5; i < MAX_MESSAGE_SIZE; ++i) {
    EXPECT_EQ(msg.data[i], 0) << "Unused octet at index " << i << " should be zero";
  }
}

// Test basic message conversion from ASN.1 to network
TEST(MessagesTest, ToNetworkBasic) {
  Message msg;
  msg.size = 5;
  msg.data[0] = 0xAA;
  msg.data[1] = 0xBB;
  msg.data[2] = 0xCC;
  msg.data[3] = 0xDD;
  msg.data[4] = 0xEE;

  NetworkMessage net_msg = to_network(msg);

  EXPECT_EQ(net_msg.size, 5);
  EXPECT_EQ(net_msg.data[0], 0xAA);
  EXPECT_EQ(net_msg.data[1], 0xBB);
  EXPECT_EQ(net_msg.data[2], 0xCC);
  EXPECT_EQ(net_msg.data[3], 0xDD);
  EXPECT_EQ(net_msg.data[4], 0xEE);

  // Verify unused octets are zeroed (matching Ada postcondition)
  for (std::size_t i = 5; i < MAX_MESSAGE_SIZE; ++i) {
    EXPECT_EQ(net_msg.data[i], 0) << "Unused octet at index " << i << " should be zero";
  }
}

// Test round-trip conversion preserves data
TEST(MessagesTest, RoundTripConversion) {
  NetworkMessage original;
  original.size = 10;
  for (std::size_t i = 0; i < 10; ++i) {
    original.data[i] = static_cast<thumper::network::Octet>(i * 17);
  }

  Message asn1_msg = from_network(original);
  NetworkMessage result = to_network(asn1_msg);

  EXPECT_EQ(result.size, original.size);
  for (std::size_t i = 0; i < original.size; ++i) {
    EXPECT_EQ(result.data[i], original.data[i]) << "Data mismatch at index " << i;
  }
}

// Test empty message (size = 0)
TEST(MessagesTest, EmptyMessage) {
  NetworkMessage net_msg;
  net_msg.size = 0;

  Message msg = from_network(net_msg);

  EXPECT_EQ(msg.size, 0);

  // All octets should be zeroed
  for (std::size_t i = 0; i < MAX_MESSAGE_SIZE; ++i) {
    EXPECT_EQ(msg.data[i], 0);
  }
}

// Test maximum size message
TEST(MessagesTest, MaxSizeMessage) {
  NetworkMessage net_msg;
  net_msg.size = MAX_MESSAGE_SIZE;
  for (std::size_t i = 0; i < MAX_MESSAGE_SIZE; ++i) {
    net_msg.data[i] = static_cast<thumper::network::Octet>(i % 256);
  }

  Message msg = from_network(net_msg);

  EXPECT_EQ(msg.size, MAX_MESSAGE_SIZE);
  for (std::size_t i = 0; i < MAX_MESSAGE_SIZE; ++i) {
    EXPECT_EQ(msg.data[i], static_cast<thumper::Octet>(i % 256));
  }
}

// Test NetworkMessage constructor with vector
TEST(MessagesTest, NetworkMessageConstructorWithVector) {
  thumper::network::OctetArray vec = {0x11, 0x22, 0x33, 0x44};
  NetworkMessage msg(vec, 4);

  EXPECT_EQ(msg.size, 4);
  EXPECT_EQ(msg.data[0], 0x11);
  EXPECT_EQ(msg.data[1], 0x22);
  EXPECT_EQ(msg.data[2], 0x33);
  EXPECT_EQ(msg.data[3], 0x44);

  // Verify remaining octets are zeroed
  for (std::size_t i = 4; i < MAX_MESSAGE_SIZE; ++i) {
    EXPECT_EQ(msg.data[i], 0);
  }
}

// Test Message constructor with vector
TEST(MessagesTest, MessageConstructorWithVector) {
  thumper::OctetArray vec = {0x55, 0x66, 0x77, 0x88, 0x99};
  Message msg(vec, 5);

  EXPECT_EQ(msg.size, 5);
  EXPECT_EQ(msg.data[0], 0x55);
  EXPECT_EQ(msg.data[1], 0x66);
  EXPECT_EQ(msg.data[2], 0x77);
  EXPECT_EQ(msg.data[3], 0x88);
  EXPECT_EQ(msg.data[4], 0x99);

  // Verify remaining octets are zeroed
  for (std::size_t i = 5; i < MAX_MESSAGE_SIZE; ++i) {
    EXPECT_EQ(msg.data[i], 0);
  }
}

// Test timestamp message structure initialization
TEST(TimestampMessagesTest, TimestampDefaultConstruction) {
  Timestamp ts;

  EXPECT_EQ(ts.version, VERSION_1);
  EXPECT_EQ(ts.serial_number, 0);
  EXPECT_TRUE(ts.generalized_time.empty());

  // Hash should be zero-initialized
  for (std::size_t i = 0; i < HASH_SIZE; ++i) {
    EXPECT_EQ(ts.hashed_message[i], 0);
  }
}

// Test timestamp with manual field initialization
TEST(TimestampMessagesTest, TimestampManualInitialization) {
  Timestamp ts;
  ts.version = VERSION_1;
  ts.serial_number = 12345678;
  ts.generalized_time = "20231215143022Z";

  // Set hash
  for (std::size_t i = 0; i < HASH_SIZE; ++i) {
    ts.hashed_message[i] = static_cast<thumper::Octet>(i);
  }

  EXPECT_EQ(ts.version, VERSION_1);
  EXPECT_EQ(ts.serial_number, 12345678);
  EXPECT_EQ(ts.generalized_time, "20231215143022Z");
  EXPECT_EQ(ts.generalized_time.length(), 15);

  for (std::size_t i = 0; i < HASH_SIZE; ++i) {
    EXPECT_EQ(ts.hashed_message[i], static_cast<thumper::Octet>(i));
  }
}

// Test Request structure
TEST(TimestampMessagesTest, RequestDefaultConstruction) {
  Request req;
  EXPECT_EQ(req.placeholder, 0);
}

// Test Response structure
TEST(TimestampMessagesTest, ResponseDefaultConstruction) {
  Response resp;
  EXPECT_EQ(resp.placeholder, 0);
}

// NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)
