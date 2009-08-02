#ifndef __XSETTINGSD_CONFIG_PARSER_H__
#define __XSETTINGSD_CONFIG_PARSER_H__

#include <string>
#ifdef __TESTING
#include <gtest/gtest_prod.h>
#endif

#include "common.h"

namespace xsettingsd {

class ConfigParser {
 public:
  class CharStream;

  // The parser takes ownership of 'stream' and calls its Init() method.
  ConfigParser(CharStream* stream);
  ~ConfigParser();

  // Abstract base class for reading a stream of characters.
  class CharStream {
   public:
    CharStream()
        : initialized_(false),
          have_buffered_char_(false),
          buffered_char_(0),
          at_line_end_(true),
          prev_at_line_end_(false),
          line_num_(0) {
    }
    virtual ~CharStream() {}

    int line_num() const { return line_num_; }

    // Must be called before using the stream.
    // The stream is unusable if false is returned.
    bool Init();

    // Are we currently at the end of the stream?
    bool AtEOF() const;

    // Get the next character in the stream.
    char GetChar();

    // Push a previously-read character back onto the stream.
    // At most one character can be buffered.
    void UngetChar(char ch);

   private:
    virtual bool InitImpl() { return true; }
    virtual bool AtEOFImpl() const = 0;
    virtual char GetCharImpl() = 0;

    // Has Init() been called?
    bool initialized_;

    // Has a character been returned with UngetChar() but not yet re-read?
    bool have_buffered_char_;

    // The character returned by UngetChar().
    char buffered_char_;

    // Are we currently at the end of the line?
    bool at_line_end_;

    bool prev_at_line_end_;

    // The line number of the last-fetched character.
    int line_num_;
  };

  // An implementation of CharStream that reads from a file.
  class FileCharStream : public CharStream {
   public:
    FileCharStream(const std::string& filename);
    ~FileCharStream();

   private:
    bool InitImpl();
    bool AtEOFImpl() const;
    char GetCharImpl();

    std::string filename_;
    FILE* file_;
  };

  // An implementation of CharStream that reads from an in-memory string.
  class StringCharStream : public CharStream {
   public:
    StringCharStream(const std::string& data);

   private:
    bool AtEOFImpl() const;
    char GetCharImpl();

    std::string data_;
    size_t pos_;
  };

  bool Parse();

 private:
#ifdef __TESTING
  friend class ConfigParserTest;
  FRIEND_TEST(ConfigParserTest, ReadSettingName);
#endif

  // Read a setting name starting at the current position in the file.
  // Returns false if the setting name is invalid.
  bool ReadSettingName(std::string* name_out);

  // Read an integer starting at the current position in the file.
  bool ReadInteger(int32* int_out);

  // Read a double-quoted string starting at the current position in the
  // file.
  bool ReadString(std::string* str_out);

  bool ReadColor(uint16* red_out, uint16* blue_out,
                 uint16* green_out, uint16* alpha_out);

  void SetErrorF(const char* format, ...);

  CharStream* stream_;

  int error_line_num_;
  std::string error_str_;
};

}  // namespace

#endif
