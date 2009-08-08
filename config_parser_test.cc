// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cassert>
#include <map>
#define __STDC_LIMIT_MACROS  // needed to get MAX and MIN macros from stdint.h
#include <stdint.h>
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
  ASSERT_TRUE(stream.Init(NULL));
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
  ASSERT_TRUE(stream.Init(NULL));
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
  int32_t GetReadIntegerData(const string& input) {
    bool result;
    int32_t data;
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

  bool GetReadColorResult(const string& input) {
    bool result;
    RunReadColor(input, &result, NULL);
    return result;
  }
  string GetReadColorData(const string& input) {
    bool result;
    string data;
    RunReadColor(input, &result, &data);
    assert(result);
    return data;
  }

 private:
  void RunReadSettingName(const string& input,
                          bool* result_out,
                          string* name_out) {
    ConfigParser::CharStream* stream =
        new ConfigParser::StringCharStream(input);
    assert(stream->Init(NULL));
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
                      int32_t* num_out) {
    ConfigParser::CharStream* stream =
        new ConfigParser::StringCharStream(input);
    assert(stream->Init(NULL));
    ConfigParser parser(stream);
    int32_t num = 0;
    bool result = parser.ReadInteger(&num);
    if (result_out)
      *result_out = result;
    if (num_out)
      *num_out = num;
  }

  void RunReadString(const string& input,
                     bool* result_out,
                     string* str_out) {
    ConfigParser::CharStream* stream =
        new ConfigParser::StringCharStream(input);
    assert(stream->Init(NULL));
    ConfigParser parser(stream);
    string str;
    bool result = parser.ReadString(&str);
    if (result_out)
      *result_out = result;
    if (str_out)
      *str_out = str;
  }

  void RunReadColor(const string& input,
                    bool* result_out,
                    string* str_out) {
    ConfigParser::CharStream* stream =
        new ConfigParser::StringCharStream(input);
    assert(stream->Init(NULL));
    ConfigParser parser(stream);
    uint16_t red = 0, green = 0, blue = 0, alpha = 0;
    bool result = parser.ReadColor(&red, &green, &blue, &alpha);
    if (result_out)
      *result_out = result;
    if (str_out)
      *str_out = StringPrintf("(%d,%d,%d,%d)", red, green, blue, alpha);
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
  EXPECT_EQ(0,         GetReadIntegerData("0"));
  EXPECT_EQ(10,        GetReadIntegerData("10"));
  EXPECT_EQ(12,        GetReadIntegerData("0012"));
  EXPECT_EQ(15,        GetReadIntegerData("15#2 comment"));
  EXPECT_EQ(20,        GetReadIntegerData("20   "));
  EXPECT_EQ(30,        GetReadIntegerData("30\n"));
  EXPECT_EQ(INT32_MAX, GetReadIntegerData("2147483647"));
  EXPECT_EQ(-5,        GetReadIntegerData("-5"));
  EXPECT_EQ(INT32_MIN, GetReadIntegerData("-2147483648"));
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

TEST_F(ConfigParserTest, ReadColor) {
  EXPECT_EQ("(1,2,3,4)", GetReadColorData("(1,2,3,4)"));
  EXPECT_EQ("(1,2,3,65535)", GetReadColorData("(1,2,3)"));
  EXPECT_EQ("(32768,32769,32770,32771)",
            GetReadColorData("(  32768 ,32769  , 32770, 32771  )"));
  EXPECT_FALSE(GetReadColorResult(""));
  EXPECT_FALSE(GetReadColorResult("("));
  EXPECT_FALSE(GetReadColorResult(")"));
  EXPECT_FALSE(GetReadColorResult("()"));
  EXPECT_FALSE(GetReadColorResult("( )"));
  EXPECT_FALSE(GetReadColorResult("(2)"));
  EXPECT_FALSE(GetReadColorResult("(,2)"));
  EXPECT_FALSE(GetReadColorResult("(2,)"));
  EXPECT_FALSE(GetReadColorResult("(2,3)"));
  EXPECT_FALSE(GetReadColorResult("(2,3,4,)"));
  EXPECT_FALSE(GetReadColorResult("(2,3,,4)"));
  EXPECT_FALSE(GetReadColorResult("(2,3,4,5,)"));
  EXPECT_FALSE(GetReadColorResult("(2(,3,4,5)"));
  EXPECT_FALSE(GetReadColorResult("(2,3,4,5,6)"));
  EXPECT_FALSE(GetReadColorResult("(2a,3,4,5)"));
  EXPECT_FALSE(GetReadColorResult("(2 1,3,4,5)"));
  EXPECT_FALSE(GetReadColorResult("(2,3,4,5"));
  EXPECT_FALSE(GetReadColorResult("(2,3\n,4,5)"));
}

testing::AssertionResult IntegerSettingEquals(
    const char* expected_expr,
    const char* actual_expr,
    int32_t expected,
    const Setting* actual) {
  if (!actual) {
    testing::Message msg;
    msg << "Expected: " << expected << "\n"
        << "  Actual: " << actual_expr << " is NULL";
    return testing::AssertionFailure(msg);
  }

  const IntegerSetting *setting = dynamic_cast<const IntegerSetting*>(actual);
  if (!setting) {
    testing::Message msg;
    msg << "Expected: " << expected << "\n"
        << "  Actual: " << actual_expr << " (not an IntegerSetting)";
    return testing::AssertionFailure(msg);
  }

  if (setting->value() != expected) {
    testing::Message msg;
    msg << "Expected: " << expected << "\n"
        << "  Actual: " << actual_expr << " contains " << setting->value();
    return testing::AssertionFailure(msg);
  }

  return testing::AssertionSuccess();
}

testing::AssertionResult StringSettingEquals(
    const char* expected_expr,
    const char* actual_expr,
    const string& expected,
    const Setting* actual) {
  if (!actual) {
    testing::Message msg;
    msg << "Expected: " << expected << "\n"
        << "  Actual: " << actual_expr << " is NULL";
    return testing::AssertionFailure(msg);
  }

  const StringSetting *setting = dynamic_cast<const StringSetting*>(actual);
  if (!setting) {
    testing::Message msg;
    msg << "Expected: " << expected << "\n"
        << "  Actual: " << actual_expr << " (not a StringSetting)";
    return testing::AssertionFailure(msg);
  }

  if (setting->value() != expected) {
    testing::Message msg;
    msg << "Expected: \"" << expected << "\"\n"
        << "  Actual: " << actual_expr << " contains \""
        << setting->value() << "\"";
    return testing::AssertionFailure(msg);
  }

  return testing::AssertionSuccess();
}

testing::AssertionResult ColorSettingEquals(
    const char* expected_expr,
    const char* actual_expr,
    const string& expected_str,
    const Setting* actual) {
  if (!actual) {
    testing::Message msg;
    msg << "Expected: " << expected_str << "\n"
        << "  Actual: " << actual_expr << " is NULL";
    return testing::AssertionFailure(msg);
  }

  const ColorSetting *setting = dynamic_cast<const ColorSetting*>(actual);
  if (!setting) {
    testing::Message msg;
    msg << "Expected: " << expected_str << "\n"
        << "  Actual: " << actual_expr << " (not a ColorSetting)";
    return testing::AssertionFailure(msg);
  }

  string actual_str = StringPrintf("(%d,%d,%d,%d)",
                                   setting->red(), setting->green(),
                                   setting->blue(), setting->alpha());
  if (actual_str != expected_str) {
    testing::Message msg;
    msg << "Expected: \"" << expected_str << "\"\n"
        << "  Actual: " << actual_expr << " contains \""
        << actual_str << "\"";
    return testing::AssertionFailure(msg);
  }

  return testing::AssertionSuccess();
}

TEST_F(ConfigParserTest, Parse) {
  const char* good_input =
      "Setting1  5\n"
      "Setting2 \"this is a string\"\n"
      "# commented line\n"
      "\n"
      "Setting3 2  # trailing comment\n"
      "Setting4 \"\\\"quoted\\\"\"# comment\n"
      "Setting5 (45,21, 5 , 8)# color";
  ConfigParser parser(new ConfigParser::StringCharStream(good_input));
  SettingsMap settings;
  ASSERT_TRUE(parser.Parse(&settings, NULL, 0));
  ASSERT_EQ(5, settings.map().size());
  EXPECT_PRED_FORMAT2(IntegerSettingEquals, 5, settings.GetSetting("Setting1"));
  EXPECT_PRED_FORMAT2(StringSettingEquals,
                      "this is a string",
                      settings.GetSetting("Setting2"));
  EXPECT_PRED_FORMAT2(IntegerSettingEquals, 2, settings.GetSetting("Setting3"));
  EXPECT_PRED_FORMAT2(StringSettingEquals,
                      "\"quoted\"",
                      settings.GetSetting("Setting4"));
  EXPECT_PRED_FORMAT2(ColorSettingEquals,
                      "(45,21,5,8)",
                      settings.GetSetting("Setting5"));

  const char* extra_name = "SettingName 3 blah";
  parser.Reset(new ConfigParser::StringCharStream(extra_name));
  EXPECT_FALSE(parser.Parse(&settings, NULL, 0));

  const char* missing_value = "SettingName";
  parser.Reset(new ConfigParser::StringCharStream(missing_value));
  EXPECT_FALSE(parser.Parse(&settings, NULL, 0));

  const char* comment_instead_of_value = "SettingName # test 3\n";
  parser.Reset(new ConfigParser::StringCharStream(comment_instead_of_value));
  EXPECT_FALSE(parser.Parse(&settings, NULL, 0));

  const char* duplicate_name = "SettingName 4\nSettingName 3";
  parser.Reset(new ConfigParser::StringCharStream(duplicate_name));
  EXPECT_FALSE(parser.Parse(&settings, NULL, 0));
}

}  // namespace xsettingsd

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
