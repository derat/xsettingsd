// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <gtest/gtest.h>

#include "common.h"

namespace xsettingsd {

TEST(CommonTest, GetPadding) {
  EXPECT_EQ(0, GetPadding(0, 4));
  EXPECT_EQ(3, GetPadding(1, 4));
  EXPECT_EQ(2, GetPadding(2, 4));
  EXPECT_EQ(1, GetPadding(3, 4));
  EXPECT_EQ(0, GetPadding(4, 4));
  EXPECT_EQ(3, GetPadding(5, 4));
  EXPECT_EQ(2, GetPadding(6, 4));
  EXPECT_EQ(1, GetPadding(7, 4));
  EXPECT_EQ(0, GetPadding(8, 4));
}

}  // namespace xsettingsd

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
