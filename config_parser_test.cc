#include <cassert>
#include <string>

#include <gtest/gtest.h>

#include "config_parser.h"

using std::string;

namespace xsettingsd {

// Test the basic operation of CharStream.
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

// Test that line numbers are reported correctly as we get and un-get
// characters.
TEST(CharStreamTest, LineNumbers) {
  ConfigParser::StringCharStream stream("a\nb\n\nc");

  // We use line 0 to represent not having read any characters yet.
  ASSERT_TRUE(stream.Init());
  EXPECT_EQ(0, stream.line_num());

  // Getting the first 'a' should put us on line 1.  We move back to 0 when
  // we un-get it and back to 1 when we re-get it.
  EXPECT_EQ('a', stream.GetChar());
  EXPECT_EQ(1, stream.line_num());
  stream.UngetChar('a');
  EXPECT_EQ(0, stream.line_num());
  EXPECT_EQ('a', stream.GetChar());
  EXPECT_EQ(1, stream.line_num());

  // The first newline should show up as being on line 1 as well.
  EXPECT_EQ('\n', stream.GetChar());
  EXPECT_EQ(1, stream.line_num());
  stream.UngetChar('\n');
  EXPECT_EQ(1, stream.line_num());
  EXPECT_EQ('\n', stream.GetChar());
  EXPECT_EQ(1, stream.line_num());

  // The 'b' is on line 2...
  EXPECT_EQ('b', stream.GetChar());
  EXPECT_EQ(2, stream.line_num());
  stream.UngetChar('b');
  EXPECT_EQ(1, stream.line_num());
  EXPECT_EQ('b', stream.GetChar());
  EXPECT_EQ(2, stream.line_num());

  // ... as is the first newline after it.
  EXPECT_EQ('\n', stream.GetChar());
  EXPECT_EQ(2, stream.line_num());
  stream.UngetChar('\n');
  EXPECT_EQ(2, stream.line_num());
  EXPECT_EQ('\n', stream.GetChar());
  EXPECT_EQ(2, stream.line_num());

  // The second newline should show up as being on line 3.
  EXPECT_EQ('\n', stream.GetChar());
  EXPECT_EQ(3, stream.line_num());
  stream.UngetChar('\n');
  EXPECT_EQ(2, stream.line_num());
  EXPECT_EQ('\n', stream.GetChar());
  EXPECT_EQ(3, stream.line_num());

  // And the 'c' is on line 4.
  EXPECT_EQ('c', stream.GetChar());
  EXPECT_EQ(4, stream.line_num());
  stream.UngetChar('c');
  EXPECT_EQ(3, stream.line_num());
  EXPECT_EQ('c', stream.GetChar());
  EXPECT_EQ(4, stream.line_num());

  EXPECT_TRUE(stream.AtEOF());
}

class ConfigParserTest : public testing::Test {
 protected:
  string GetReadSettingNameString(const string& data) {
    ConfigParser::CharStream* stream = new ConfigParser::StringCharStream(data);
    assert(stream->Init());
    ConfigParser parser(stream);
    string name;
    assert(parser.ReadSettingName(&name));
    return name;
  }

  bool GetReadSettingNameResult(const string& data) {
    ConfigParser::CharStream* stream = new ConfigParser::StringCharStream(data);
    assert(stream->Init());
    ConfigParser parser(stream);
    string name;
    return parser.ReadSettingName(&name);
  }
};

TEST_F(ConfigParserTest, ReadSettingName) {
  EXPECT_EQ(GetReadSettingNameString("test"), "test");
  EXPECT_EQ(GetReadSettingNameString("First/Second"), "First/Second");
  EXPECT_EQ(GetReadSettingNameString("Has_Underscore"), "Has_Underscore");
  EXPECT_EQ(GetReadSettingNameString("trailing_space  "), "trailing_space");
  EXPECT_FALSE(GetReadSettingNameResult(" leading_space"));
  EXPECT_FALSE(GetReadSettingNameResult("/leading_slash"));
  EXPECT_FALSE(GetReadSettingNameResult("trailing_slash/"));
  EXPECT_FALSE(GetReadSettingNameResult("double//slash"));
  EXPECT_FALSE(GetReadSettingNameResult("digit_after_slash/0"));
}

}  // namespace xsettingsd

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
