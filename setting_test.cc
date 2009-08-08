// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <stdint.h>
#include <string>

#include <gtest/gtest.h>

#include "data_writer.h"
#include "setting.h"

using std::string;

namespace xsettingsd {

testing::AssertionResult BytesAreEqual(
    const char* expected_expr,
    const char* actual_expr,
    const char* size_expr,
    const char* expected,
    const char* actual,
    size_t size) {
  for (size_t i = 0; i < size; ++i) {
    if (expected[i] != actual[i]) {
      testing::Message msg;
      string expected_str, actual_str, hl_str;
      bool first = true;
      for (size_t j = 0; j < size; ++j) {
        expected_str +=
            StringPrintf(" %02x", static_cast<unsigned char>(expected[j]));
        actual_str +=
            StringPrintf(" %02x", static_cast<unsigned char>(actual[j]));
        hl_str += (expected[j] == actual[j]) ? "   " : " ^^";
        if ((j % 16) == 15 || j == size - 1) {
          msg << (first ? "Expected:" : "\n         ") << expected_str << "\n"
              << (first ? "  Actual:" : "         ") << actual_str << "\n"
              << "         " << hl_str;
          expected_str = actual_str = hl_str = "";
          first = false;
        }
      }
      return testing::AssertionFailure(msg);
    }
  }
  return testing::AssertionSuccess();
}

TEST(IntegerSettingTest, Write) {
  static const int kBufSize = 1024;
  char buffer[kBufSize];

  DataWriter writer(buffer, kBufSize);
  IntegerSetting setting(5);
  setting.UpdateSerial(NULL, 3);
  ASSERT_TRUE(setting.Write("name", &writer));
  // TODO: Won't work on big-endian systems.
  const char expected[] = {
    0x0,                     // type
    0x0,                     // unused
    0x4, 0x0,                // name-len
    0x6e, 0x61, 0x6d, 0x65,  // "name" (multiple of 4, so no padding)
    0x3, 0x0, 0x0, 0x0,      // serial
    0x5, 0x0, 0x0, 0x0,      // value
  };
  ASSERT_EQ(sizeof(expected), writer.bytes_written());
  EXPECT_PRED_FORMAT3(BytesAreEqual, expected, buffer, sizeof(expected));
}

TEST(StringSettingTest, Write) {
  static const int kBufSize = 1024;
  char buffer[kBufSize];

  DataWriter writer(buffer, kBufSize);
  StringSetting setting("testing");
  setting.UpdateSerial(NULL, 5);
  ASSERT_TRUE(setting.Write("setting", &writer));
  // TODO: Won't work on big-endian systems.
  const char expected[] = {
    0x1,                     // type
    0x0,                     // unused
    0x7, 0x0,                // name-len
    0x73, 0x65, 0x74, 0x74, 0x69, 0x6e, 0x67,  // "setting" (name)
    0x0,                     // padding
    0x5, 0x0, 0x0, 0x0,      // serial
    0x7, 0x0, 0x0, 0x0,      // value-len
    0x74, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67,  // "testing" (value)
    0x0,                     // padding
  };
  ASSERT_EQ(sizeof(expected), writer.bytes_written());
  EXPECT_PRED_FORMAT3(BytesAreEqual, expected, buffer, sizeof(expected));
}

TEST(ColorSettingTest, Write) {
  static const int kBufSize = 1024;
  char buffer[kBufSize];

  DataWriter writer(buffer, kBufSize);
  ColorSetting setting(32768, 65535, 0, 255);
  setting.UpdateSerial(NULL, 2);
  ASSERT_TRUE(setting.Write("name", &writer));
  // TODO: Won't work on big-endian systems.
  const char expected[] = {
    0x2,                     // type
    0x0,                     // unused
    0x4, 0x0,                // name-len
    0x6e, 0x61, 0x6d, 0x65,  // "name" (multiple of 4, so no padding)
    0x2, 0x0, 0x0, 0x0,      // serial
    0x0, 0x80,               // red
    0x0, 0x0,                // blue (yes, the order is strange)
    0xff, 0xff,              // green
    0xff, 0x0,               // alpha
  };
  ASSERT_EQ(sizeof(expected), writer.bytes_written());
  EXPECT_PRED_FORMAT3(BytesAreEqual, expected, buffer, sizeof(expected));
}

TEST(SettingTest, Serials) {
  // Create a setting and give it a serial of 3.
  IntegerSetting setting(4);
  setting.UpdateSerial(NULL, 3);
  EXPECT_EQ(3, setting.serial());

  // Now create a new setting with a different value.
  // It should get a new serial number.
  IntegerSetting setting2(5);
  setting2.UpdateSerial(&setting, 4);
  EXPECT_EQ(4, setting2.serial());

  // Create a new setting with the same value.
  // The serial should stay the same as before.
  IntegerSetting setting3(5);
  setting3.UpdateSerial(&setting2, 5);
  EXPECT_EQ(4, setting3.serial());
}

}  // namespace xsettingsd

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
