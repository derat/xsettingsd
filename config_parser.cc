// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "config_parser.h"

#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <vector>

#include "setting.h"

using std::map;
using std::string;
using std::vector;

namespace xsettingsd {

ConfigParser::ConfigParser(CharStream* stream)
    : stream_(NULL),
      error_line_num_(0) {
  Reset(stream);
}

ConfigParser::~ConfigParser() {
  delete stream_;
}

string ConfigParser::FormatError() const {
  if (error_line_num_ == 0)
    return error_str_;
  return StringPrintf("%d: %s", error_line_num_, error_str_.c_str());
}

void ConfigParser::Reset(CharStream* stream) {
  assert(stream);
  delete stream_;
  stream_ = stream;

  error_line_num_ = 0;
  error_str_.clear();
}

bool ConfigParser::Parse(SettingsMap* settings,
                         const SettingsMap* prev_settings,
                         uint32_t serial) {
  assert(settings);
  settings->mutable_map()->clear();

  string stream_error;
  if (!stream_->Init(&stream_error)) {
    SetErrorF("Couldn't init stream (%s)", stream_error.c_str());
    return false;
  }

  enum State {
    // At the beginning of a line, before we've encountered a setting name.
    NO_SETTING_NAME = 0,

    // We've gotten the setting name but not its value.
    GOT_SETTING_NAME,

    // We've got the value.
    GOT_VALUE,
  };
  State state = NO_SETTING_NAME;
  string setting_name;
  bool in_comment = false;

  while (!stream_->AtEOF()) {
    char ch = stream_->GetChar();

    if (ch == '#') {
      in_comment = true;
      continue;
    }

    if (ch == '\n') {
      if (state == GOT_SETTING_NAME) {
        SetErrorF("No value for setting \"%s\"", setting_name.c_str());
        return false;
      }
      state = NO_SETTING_NAME;
      setting_name.clear();
      in_comment = false;
    }

    if (in_comment || isspace(ch))
      continue;

    stream_->UngetChar(ch);

    switch (state) {
      case NO_SETTING_NAME:
        if (!ReadSettingName(&setting_name))
          return false;
        if (settings->map().count(setting_name)) {
          SetErrorF("Got duplicate setting name \"%s\"", setting_name.c_str());
          return false;
        }
        state = GOT_SETTING_NAME;
        break;
      case GOT_SETTING_NAME:
        {
          Setting* setting = NULL;
          if (!ReadValue(&setting))
            return false;
          const Setting* prev_setting =
              prev_settings ? prev_settings->GetSetting(setting_name) : NULL;
          setting->UpdateSerial(prev_setting, serial);
          settings->mutable_map()->insert(make_pair(setting_name, setting));
        }
        state = GOT_VALUE;
        break;
      case GOT_VALUE:
        SetErrorF("Got unexpected text after value");
        return false;
    }
  }

  if (state != NO_SETTING_NAME && state != GOT_VALUE) {
    SetErrorF("Unexpected end of file");
    return false;
  }
  return true;
}

bool ConfigParser::CharStream::Init(string* error_out) {
  assert(!initialized_);
  initialized_ = true;
  return InitImpl(error_out);
}

bool ConfigParser::CharStream::AtEOF() {
  assert(initialized_);
  return (!have_buffered_char_ && AtEOFImpl());
}

char ConfigParser::CharStream::GetChar() {
  assert(initialized_);

  prev_at_line_end_ = at_line_end_;
  if (at_line_end_) {
    line_num_++;
    at_line_end_ = false;
  }

  char ch = 0;
  if (have_buffered_char_) {
    have_buffered_char_ = false;
    ch = buffered_char_;
  } else {
    ch = GetCharImpl();
  }

  if (ch == '\n')
    at_line_end_ = true;

  return ch;
}

void ConfigParser::CharStream::UngetChar(char ch) {
  if (prev_at_line_end_)
    line_num_--;
  at_line_end_ = prev_at_line_end_;

  assert(initialized_);
  assert(!have_buffered_char_);
  buffered_char_ = ch;
  have_buffered_char_ = true;
}

ConfigParser::FileCharStream::FileCharStream(const string& filename)
    : filename_(filename),
      file_(NULL) {
}

ConfigParser::FileCharStream::~FileCharStream() {
  if (file_) {
    fclose(file_);
    file_ = NULL;
  }
}

bool ConfigParser::FileCharStream::InitImpl(string* error_out) {
  assert(!file_);
  file_ = fopen(filename_.c_str(), "r");
  if (!file_) {
    if (error_out)
      *error_out = strerror(errno);
    return false;
  }
  return true;
}

bool ConfigParser::FileCharStream::AtEOFImpl() {
  assert(file_);
  int ch = GetCharPriv();
  UngetChar(ch);
  return ch == EOF;
}

char ConfigParser::FileCharStream::GetCharImpl() {
  char ch = GetCharPriv();
  return ch;
}

int ConfigParser::FileCharStream::GetCharPriv() {
  assert(file_);
  return fgetc(file_);
}

ConfigParser::StringCharStream::StringCharStream(const string& data)
    : data_(data),
      pos_(0) {
}

bool ConfigParser::StringCharStream::AtEOFImpl() {
  return pos_ == data_.size();
}

char ConfigParser::StringCharStream::GetCharImpl() {
  char ret = GetCharPriv();
  return ret;
}

int ConfigParser::StringCharStream::GetCharPriv() {
  int ret = data_.at(pos_++);
  return ret;
}

bool ConfigParser::ReadSettingName(string* name_out) {
  assert(name_out);
  name_out->clear();

  bool prev_was_slash = false;
  while (true) {
    if (stream_->AtEOF())
      break;

    char ch = stream_->GetChar();
    if (isspace(ch) || ch == '#') {
      stream_->UngetChar(ch);
      break;
    }

    if (!(ch >= 'A' && ch <= 'Z') &&
        !(ch >= 'a' && ch <= 'z') &&
        !(ch >= '0' && ch <= '9') &&
        !(ch == '_' || ch == '/')) {
      SetErrorF("Got invalid character '%c' in setting name", ch);
      return false;
    }

    if (ch == '/' && name_out->empty()) {
      SetErrorF("Got leading slash in setting name");
      return false;
    }

    if (prev_was_slash) {
      if (ch == '/') {
        SetErrorF("Got two consecutive slashes in setting name");
        return false;
      }
      if (ch >= '0' && ch <= '9') {
        SetErrorF("Got digit after slash in setting name");
        return false;
      }
    }

    name_out->push_back(ch);

    prev_was_slash = (ch == '/');
  }

  if (name_out->empty()) {
    SetErrorF("Got empty setting name");
    return false;
  }

  if (name_out->at(name_out->size() - 1) == '/') {
    SetErrorF("Got trailing slash in setting name");
    return false;
  }

  return true;
}

bool ConfigParser::ReadValue(Setting** setting_ptr) {
  assert(setting_ptr);
  *setting_ptr = NULL;

  if (stream_->AtEOF()) {
    SetErrorF("Got EOF when starting to read value");
    return false;
  }

  char ch = stream_->GetChar();
  stream_->UngetChar(ch);

  if ((ch >= '0' && ch <= '9') || ch == '-') {
    int32_t value = 0;
    if (!ReadInteger(&value))
      return false;
    *setting_ptr = new IntegerSetting(value);
  } else if (ch == '"') {
    string value;
    if (!ReadString(&value))
      return false;
    *setting_ptr = new StringSetting(value);
  } else if (ch == '(') {
    uint16_t red, green, blue, alpha;
    if (!ReadColor(&red, &green, &blue, &alpha))
      return false;
    *setting_ptr = new ColorSetting(red, green, blue, alpha);
  } else {
    SetErrorF("Got invalid setting value");
    return false;
  }
  return true;
}

bool ConfigParser::ReadInteger(int32_t* int_out) {
  assert(int_out);
  *int_out = 0;

  bool got_digit = false;
  bool negative = false;
  while (true) {
    if (stream_->AtEOF())
      break;

    char ch = stream_->GetChar();
    if (isspace(ch) || ch == '#') {
      stream_->UngetChar(ch);
      break;
    }

    if (ch == '-') {
      if (negative) {
        SetErrorF("Got extra '-' before integer");
        return false;
      }

      if (!got_digit) {
        negative = true;
        continue;
      } else {
        SetErrorF("Got '-' mid-integer");
        return false;
      }
    }

    if (!(ch >= '0' && ch <= '9')) {
      SetErrorF("Got non-numeric character '%c'", ch);
      return false;
    }

    got_digit = true;
    *int_out *= 10;
    *int_out += (ch - '0');

    // TODO: Check for integer overflow (not a security hole; it'd just be
    // nice to warn the user that their setting is going to wrap).
  }

  if (!got_digit) {
    SetErrorF("Got empty integer");
    return false;
  }

  if (negative)
    *int_out *= -1;

  return true;
}

bool ConfigParser::ReadString(string* str_out) {
  assert(str_out);
  str_out->clear();

  bool in_backslash = false;
  if (stream_->AtEOF() || stream_->GetChar() != '\"') {
    SetErrorF("String is missing initial double-quote");
    return false;
  }

  while (true) {
    if (stream_->AtEOF()) {
      SetErrorF("Open string at end of file");
      return false;
    }

    char ch = stream_->GetChar();
    if (ch == '\n') {
      SetErrorF("Got newline mid-string");
      return false;
    }

    if (!in_backslash) {
      if (ch == '"')
        break;

      if (ch == '\\') {
        in_backslash = true;
        continue;
      }
    }

    if (in_backslash) {
      in_backslash = false;
      if (ch == 'n')      ch = '\n';
      else if (ch == 't') ch = '\t';
    }
    str_out->push_back(ch);
  }

  return true;
}

bool ConfigParser::ReadColor(uint16_t* red_out,
                             uint16_t* green_out,
                             uint16_t* blue_out,
                             uint16_t* alpha_out) {
  assert(red_out);
  assert(green_out);
  assert(blue_out);
  assert(alpha_out);

  if (stream_->AtEOF() || stream_->GetChar() != '(') {
    SetErrorF("Color is missing initial parethesis");
    return false;
  }

  vector<uint16_t> nums;

  enum State {
    BEFORE_NUM,
    IN_NUM,
    AFTER_NUM,
  };
  int num = 0;

  State state = BEFORE_NUM;
  while (true) {
    if (stream_->AtEOF()) {
      SetErrorF("Got EOF mid-color");
      return false;
    }

    char ch = stream_->GetChar();
    if (ch == '\n') {
      SetErrorF("Got newline mid-color");
      return false;
    }

    if (ch == ')') {
      if (state == BEFORE_NUM) {
        SetErrorF("Expected number but got ')'");
        return false;
      }
      if (state == IN_NUM)
        nums.push_back(num);
      break;
    }

    if (isspace(ch)) {
      if (state == IN_NUM) {
        state = AFTER_NUM;
        nums.push_back(num);
      }
      continue;
    }

    if (ch == ',') {
      if (state == BEFORE_NUM) {
        SetErrorF("Got unexpected comma");
        return false;
      }
      if (state == IN_NUM)
        nums.push_back(num);
      state = BEFORE_NUM;
      continue;
    }

    if (!(ch >= '0' && ch <= '9')) {
      SetErrorF("Got non-numeric character '%c'", ch);
      return false;
    }

    if (state == AFTER_NUM) {
      SetErrorF("Got unexpected digit '%c'", ch);
      return false;
    }
    if (state == BEFORE_NUM) {
      state = IN_NUM;
      num = 0;
    }
    num = num * 10 + (ch - '0');

    // TODO: Check for integer overflow (not a security hole; it'd just be
    // nice to warn the user that their setting is going to wrap).
  }

  if (nums.size() < 3 || nums.size() > 4) {
    SetErrorF("Got %d number%s instead of 3 or 4",
              nums.size(), (nums.size() == 1 ? "" : "s"));
    return false;
  }

  *red_out = nums.at(0);
  *green_out = nums.at(1);
  *blue_out = nums.at(2);
  *alpha_out = (nums.size() == 4) ? nums.at(3) : 65535;
  return true;
}

void ConfigParser::SetErrorF(const char* format, ...) {
  char buffer[1024];
  va_list argp;
  va_start(argp, format);
  vsnprintf(buffer, sizeof(buffer), format, argp);
  va_end(argp);
  error_line_num_ = stream_->line_num();
  error_str_.assign(buffer);
}

}  // namespace xsettingsd
