// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __XSETTINGSD_CONFIG_PARSER_H__
#define __XSETTINGSD_CONFIG_PARSER_H__

#include <map>
#include <stdint.h>
#include <string>

#ifdef __TESTING
#include <gtest/gtest_prod.h>
#endif

#include "common.h"

namespace xsettingsd {

class Setting;
class SettingsMap;

// Doing the parsing by hand like this for a line-based config format is
// pretty much the worst idea ever -- it would've been much easier to use
// libpcrecpp. :-(  The tests all pass, though, for whatever that's worth.
class ConfigParser {
 public:
  class CharStream;

  // The parser takes ownership of 'stream', which must be uninitialized
  // (that is, its Init() method shouldn't have been called yet).
  explicit ConfigParser(CharStream* stream);
  ~ConfigParser();

  // Returns a formatted string describing a parse error.  Can be called
  // after Parse() returns false.
  std::string FormatError() const;

  // Reset the parser to read from a new stream.
  void Reset(CharStream* stream);

  // Parse the data in the stream into 'settings', using 'prev_settings'
  // (pass the previous version if it exists or NULL otherwise) and
  // 'serial' (the new serial number) to determine which serial number each
  // setting should have.  This method calls the stream's Init() method;
  // don't do it beforehand.
  bool Parse(SettingsMap* settings,
             const SettingsMap* prev_settings,
             uint32_t serial);

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
    bool Init(std::string* error_out);

    // Are we currently at the end of the stream?
    bool AtEOF();

    // Get the next character in the stream.
    int GetChar();

    // Push a previously-read character back onto the stream.
    // At most one character can be buffered.
    void UngetChar(int ch);

   private:
    virtual bool InitImpl(std::string* error_out) { return true; }
    virtual bool AtEOFImpl() = 0;
    virtual int GetCharImpl() = 0;

    // Has Init() been called?
    bool initialized_;

    // Has a character been returned with UngetChar() but not yet re-read?
    bool have_buffered_char_;

    // The character returned by UngetChar().
    int buffered_char_;

    // Are we currently at the end of the line?
    bool at_line_end_;

    bool prev_at_line_end_;

    // The line number of the last-fetched character.
    int line_num_;

    DISALLOW_COPY_AND_ASSIGN(CharStream);
  };

  // An implementation of CharStream that reads from a file.
  class FileCharStream : public CharStream {
   public:
    FileCharStream(const std::string& filename);
    ~FileCharStream();

   private:
    bool InitImpl(std::string* error_out);
    bool AtEOFImpl();
    int GetCharImpl();

    std::string filename_;
    FILE* file_;

    DISALLOW_COPY_AND_ASSIGN(FileCharStream);
  };

  // An implementation of CharStream that reads from an in-memory string.
  class StringCharStream : public CharStream {
   public:
    StringCharStream(const std::string& data);

   private:
    bool AtEOFImpl();
    int GetCharImpl();

    std::string data_;
    size_t pos_;

    DISALLOW_COPY_AND_ASSIGN(StringCharStream);
  };

 private:
#ifdef __TESTING
  friend class ConfigParserTest;
  FRIEND_TEST(ConfigParserTest, ReadSettingName);
#endif

  // Read a setting name starting at the current position in the stream.
  // Returns false if the setting name is invalid.
  bool ReadSettingName(std::string* name_out);

  // Read the value starting at the current position in the stream.
  // Its type is inferred from the first character.  'setting_ptr' is
  // updated to point at a newly-allocated Setting object (which the caller
  // is responsible for deleting).
  bool ReadValue(Setting** setting_ptr);

  // Read an integer starting at the current position in the stream.
  bool ReadInteger(int32_t* int_out);

  // Read a double-quoted string starting at the current position in the
  // stream.
  bool ReadString(std::string* str_out);

  // Read a color of the form "(red, green, blue, alpha)" or "(red, green,
  // blue)".
  bool ReadColor(uint16_t* red_out, uint16_t* green_out,
                 uint16_t* blue_out, uint16_t* alpha_out);

  // Record an error to 'error_str_', also saving the stream's current line
  // number to 'error_line_num_'.
  void SetErrorF(const char* format, ...);

  // Stream from which the config is being parsed.
  CharStream* stream_;

  // If an error was encountered while parsing, the line number where
  // it happened and a string describing it.  Line 0 is used for errors
  // occurring before making any progress into the file.
  int error_line_num_;
  std::string error_str_;

  DISALLOW_COPY_AND_ASSIGN(ConfigParser);
};

}  // namespace

#endif
