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
      string expected_bytes;
      string actual_bytes;
      string hl;
      for (size_t j = 0; j < size; ++j) {
        expected_bytes += StringPrintf(" %02x", expected[j]);
        actual_bytes += StringPrintf(" %02x", actual[j]);
        hl += (expected[j] == actual[j]) ? "   " : " ^^";
        if (j > 0 && (j % 8) == 0) {
          if (j == 8) {
            msg << "Expected:" << expected_bytes
                << "\n  Actual:" << actual_bytes
                << "\n         " << hl << "\n";
          } else {
            msg << "         " << expected_bytes
                << "\n         " << actual_bytes
                << "\n         " << hl << "\n";
          }
          expected_bytes = actual_bytes = hl = "";
        }
      }
      if (!expected_bytes.empty()) {
        msg << "         " << expected_bytes
            << "\n         " << actual_bytes
            << "\n         " << hl;
      }
      return testing::AssertionFailure(msg);
    }
  }
  return testing::AssertionSuccess();
}

TEST(IntegerSettingTest, WriteBody) {
  static const int kBufSize = 1024;
  char buffer[kBufSize];

  DataWriter writer(buffer, kBufSize);
  IntegerSetting setting(5);
  ASSERT_TRUE(setting.Write("name", &writer));
  const char expected[] = {
    0x0,                     // type
    0x0,                     // unused
    0x4, 0x0,                // name-len
    0x6e, 0x61, 0x6d, 0x65,  // "name" (multiple of 4, so no padding)
    0x0, 0x0, 0x0, 0x0,      // serial
    0x5, 0x0, 0x0, 0x0,      // value  TODO: fix for big-endian
  };
  ASSERT_EQ(sizeof(expected), writer.bytes_written());
  EXPECT_PRED_FORMAT3(BytesAreEqual, expected, buffer, sizeof(expected));
}

}  // namespace xsettingsd

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
