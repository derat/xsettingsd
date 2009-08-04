#include <cassert>
#include <map>
#include <string>

#include <gtest/gtest.h>

#include "config_parser.h"
#include "setting.h"

using std::map;
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
  // Helper methods to get the return value or string from
  // ConfigParser::ReadSettingName().
  bool GetReadSettingNameResult(const string& input) {
    bool result;
    RunReadSettingName(input, &result, NULL);
    return result;
  }
  string GetReadSettingNameData(const string& input) {
    bool result;
    string data;
    RunReadSettingName(input, &result, &data);
    assert(result);
    return data;
  }

  bool GetReadIntegerResult(const string& input) {
    bool result;
    RunReadInteger(input, &result, NULL);
    return result;
  }
  int32 GetReadIntegerData(const string& input) {
    bool result;
    int32 data;
    RunReadInteger(input, &result, &data);
    assert(result);
    return data;
  }

  bool GetReadStringResult(const string& input) {
    bool result;
    RunReadString(input, &result, NULL);
    return result;
  }
  string GetReadStringData(const string& input) {
    bool result;
    string data;
    RunReadString(input, &result, &data);
    assert(result);
    return data;
  }

 private:
  void RunReadSettingName(const string& input,
                          bool* result_out,
                          string* name_out) {
    ConfigParser::CharStream* stream = new ConfigParser::StringCharStream(input);
    assert(stream->Init());
    ConfigParser parser(stream);
    string name;
    bool result = parser.ReadSettingName(&name);
    if (result_out)
      *result_out = result;
    if (name_out)
      *name_out = name;
  }

  void RunReadInteger(const string& input,
                      bool* result_out,
                      int32* num_out) {
    ConfigParser::CharStream* stream = new ConfigParser::StringCharStream(input);
    assert(stream->Init());
    ConfigParser parser(stream);
    int32 num;
    bool result = parser.ReadInteger(&num);
    if (result_out)
      *result_out = result;
    if (num_out)
      *num_out = num;
  }

  void RunReadString(const string& input,
                     bool* result_out,
                     string* str_out) {
    ConfigParser::CharStream* stream = new ConfigParser::StringCharStream(input);
    assert(stream->Init());
    ConfigParser parser(stream);
    string str;
    bool result = parser.ReadString(&str);
    if (result_out)
      *result_out = result;
    if (str_out)
      *str_out = str;
  }
};

TEST_F(ConfigParserTest, ReadSettingName) {
  EXPECT_EQ("test",           GetReadSettingNameData("test"));
  EXPECT_EQ("First/Second",   GetReadSettingNameData("First/Second"));
  EXPECT_EQ("Has_Underscore", GetReadSettingNameData("Has_Underscore"));
  EXPECT_EQ("trailing_space", GetReadSettingNameData("trailing_space  "));
  EXPECT_EQ("blah",           GetReadSettingNameData("blah#comment"));
  EXPECT_FALSE(GetReadSettingNameResult(" leading_space"));
  EXPECT_FALSE(GetReadSettingNameResult("/leading_slash"));
  EXPECT_FALSE(GetReadSettingNameResult("trailing_slash/"));
  EXPECT_FALSE(GetReadSettingNameResult("double//slash"));
  EXPECT_FALSE(GetReadSettingNameResult("digit_after_slash/0"));

  // For good measure, test the examples of legitimate and illegitimate
  // names from the spec.
  EXPECT_EQ("GTK/colors/background0",
            GetReadSettingNameData("GTK/colors/background0"));
  EXPECT_EQ("_background", GetReadSettingNameData("_background"));
  EXPECT_EQ("_111",        GetReadSettingNameData("_111"));
  EXPECT_FALSE(GetReadSettingNameResult("/"));
  EXPECT_FALSE(GetReadSettingNameResult("_background/"));
  EXPECT_FALSE(GetReadSettingNameResult("GTK//colors"));
  EXPECT_FALSE(GetReadSettingNameResult(""));
}

TEST_F(ConfigParserTest, ReadInteger) {
  EXPECT_EQ(0,           GetReadIntegerData("0"));
  EXPECT_EQ(10,          GetReadIntegerData("10"));
  EXPECT_EQ(12,          GetReadIntegerData("0012"));
  EXPECT_EQ(15,          GetReadIntegerData("15#2 comment"));
  EXPECT_EQ(20,          GetReadIntegerData("20   "));
  EXPECT_EQ(2147483647,  GetReadIntegerData("2147483647"));
  EXPECT_EQ(-5,          GetReadIntegerData("-5"));
  EXPECT_EQ(-2147483648, GetReadIntegerData("-2147483648"));
  EXPECT_FALSE(GetReadIntegerResult(""));
  EXPECT_FALSE(GetReadIntegerResult("-"));
  EXPECT_FALSE(GetReadIntegerResult("--2"));
  EXPECT_FALSE(GetReadIntegerResult("2-3"));
  EXPECT_FALSE(GetReadIntegerResult(" "));
  EXPECT_FALSE(GetReadIntegerResult(" 23"));
}

TEST_F(ConfigParserTest, ReadString) {
  EXPECT_EQ("test",              GetReadStringData("\"test\""));
  EXPECT_EQ(" some whitespace ", GetReadStringData("\" some whitespace \""));
  EXPECT_EQ("a\tb\nc",           GetReadStringData("\"a\\tb\\nc\""));
  EXPECT_EQ("a\"b\\c",           GetReadStringData("\"a\\\"b\\\\c\""));
  EXPECT_EQ("ar",                GetReadStringData("\"\\a\\r\""));
  EXPECT_EQ(" ",                 GetReadStringData("\" \""));
  EXPECT_EQ("",                  GetReadStringData("\"\""));
  EXPECT_EQ("a",                 GetReadStringData("\"a\"   "));
  EXPECT_FALSE(GetReadStringResult(""));
  EXPECT_FALSE(GetReadStringResult("a"));
  EXPECT_FALSE(GetReadStringResult("\""));
  EXPECT_FALSE(GetReadStringResult("\"\n\""));
}

TEST_F(ConfigParserTest, Parse) {
  const char* input =
      "IntSetting  3\n"
      "StringSetting \"this is a string\"\n"
      "AnotherIntSetting 2  # trailing comment\n";
  ConfigParser parser(new ConfigParser::StringCharStream(input));
  SettingsMap settings;
  ASSERT_TRUE(parser.Parse(&settings));
  ASSERT_EQ(3, settings.map().size());
}

}  // namespace xsettingsd

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
