#include <cassert>
#include <string>

#include <gtest/gtest.h>

#include "config_parser.h"

using std::string;

namespace xsettingsd {

TEST(CharStreamTest, Basic) {
  ConfigParser::StringCharStream stream("012");

  // We should die if we try to do anything before calling Init().
  ASSERT_DEATH(stream.AtEOF(), "initialized");
  ASSERT_DEATH(stream.GetChar(), "initialized");
  ASSERT_DEATH(stream.UngetChar('0'), "initialized");

  // Now read a character, put it back, and read it again.
  ASSERT_TRUE(stream.Init());
  EXPECT_FALSE(stream.AtEOF());
  EXPECT_EQ('0', stream.GetChar());
  stream.UngetChar('0');
  EXPECT_EQ('0', stream.GetChar());

  // Do the same thing with the second character...
  EXPECT_FALSE(stream.AtEOF());
  EXPECT_EQ('1', stream.GetChar());
  stream.UngetChar('1');
  EXPECT_EQ('1', stream.GetChar());

  // ... and with the third.  We should be at EOF after reading it.
  EXPECT_EQ('2', stream.GetChar());
  EXPECT_TRUE(stream.AtEOF());
  stream.UngetChar('2');
  EXPECT_FALSE(stream.AtEOF());
  EXPECT_EQ('2', stream.GetChar());
  EXPECT_TRUE(stream.AtEOF());
}

class ConfigParserTest : public testing::Test {
 protected:
  bool TestReadSettingName(const string& data) {
    ConfigParser::CharStream* stream = new ConfigParser::StringCharStream(data);
    assert(stream->Init());
    ConfigParser parser(stream);
    string name;
    return parser.ReadSettingName(&name);
  }
};

TEST_F(ConfigParserTest, ReadSettingName) {
  EXPECT_TRUE(TestReadSettingName("test"));
  EXPECT_TRUE(TestReadSettingName("First/Second"));
  EXPECT_TRUE(TestReadSettingName("Has_Underscore"));
  EXPECT_FALSE(TestReadSettingName("/leading_slash"));
  EXPECT_FALSE(TestReadSettingName("trailing_slash/"));
  EXPECT_FALSE(TestReadSettingName("double//slash"));
  EXPECT_FALSE(TestReadSettingName("digit_after_slash/0"));
}

}  // namespace xsettingsd

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
