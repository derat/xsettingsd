// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cstdlib>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "common.h"

using std::string;
using std::vector;

namespace xsettingsd {
namespace {

// Returns |parts| joined by '|'.
string Join(const vector<string>& parts) {
  string out;
  for (size_t i = 0; i < parts.size(); ++i)
    out += (i > 0 ? "|" : "") + parts[i];
  return out;
}

}  // namespace

TEST(CommonTest, SplitString) {
  EXPECT_EQ("a|b|c", Join(SplitString("a,b,c", ",")));
  EXPECT_EQ("a|b|", Join(SplitString("a,b,", ",")));
  EXPECT_EQ("|a|b", Join(SplitString(",a,b", ",")));
  EXPECT_EQ("|a|b|", Join(SplitString(",a,b,", ",")));
  EXPECT_EQ("a||b", Join(SplitString("a,,b", ",")));
  EXPECT_EQ("|", Join(SplitString(",", ",")));
  EXPECT_EQ("foo", Join(SplitString("foo", ",")));
  EXPECT_EQ("foo|bar", Join(SplitString("fooabcbar", "abc")));
  EXPECT_EQ("|foo", Join(SplitString("abcfoo", "abc")));
  EXPECT_EQ("foo|", Join(SplitString("fooabc", "abc")));
  EXPECT_EQ(0, SplitString("", ",").size());
  EXPECT_EQ(0, SplitString("", "").size());
  EXPECT_EQ("abc", Join(SplitString("abc", "")));
}

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

TEST(CommonTest, GetDefaultConfigFilePath) {
  // With $HOME missing and none of the XDG vars, we should just use /etc.
  ASSERT_EQ(0, unsetenv("HOME"));
  ASSERT_EQ(0, unsetenv("XDG_CONFIG_HOME"));
  ASSERT_EQ(0, unsetenv("XDG_CONFIG_DIRS"));
  EXPECT_EQ("/etc/xsettingsd/xsettingsd.conf",
            Join(GetDefaultConfigFilePaths()));

  // Now set $HOME. It should be searched first, followed by the default XDG
  // paths.
  ASSERT_EQ(0, setenv("HOME", "/home/user", 1 /* overwrite */));
  EXPECT_EQ("/home/user/.xsettingsd|"
            "/home/user/.config/xsettingsd/xsettingsd.conf|"
            "/etc/xsettingsd/xsettingsd.conf",
            Join(GetDefaultConfigFilePaths()));

  // Use a custom $XDG_CONFIG_HOME.
  ASSERT_EQ(0, setenv("XDG_CONFIG_HOME", "/home/user/.myconf", 1));
  EXPECT_EQ("/home/user/.xsettingsd|"
            "/home/user/.myconf/xsettingsd/xsettingsd.conf|"
            "/etc/xsettingsd/xsettingsd.conf",
            Join(GetDefaultConfigFilePaths()));

  // Now put a few paths in $XDG_CONFIG_DIRS.
  ASSERT_EQ(0, setenv("XDG_CONFIG_DIRS", "/etc2:/etc3", 1));
  EXPECT_EQ("/home/user/.xsettingsd|"
            "/home/user/.myconf/xsettingsd/xsettingsd.conf|"
            "/etc2/xsettingsd/xsettingsd.conf|"
            "/etc3/xsettingsd/xsettingsd.conf",
            Join(GetDefaultConfigFilePaths()));
}

}  // namespace xsettingsd

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
