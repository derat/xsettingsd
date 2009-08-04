#include "config_parser.h"

#include <cassert>
#include <cerrno>
#include <cstdarg>
#include <cstring>

#include "setting.h"

using std::map;
using std::string;

namespace xsettingsd {

ConfigParser::ConfigParser(CharStream* stream)
    : stream_(stream),
      error_line_num_(0) {
  assert(stream_);
}

ConfigParser::~ConfigParser() {
  delete stream_;
}

bool ConfigParser::CharStream::Init() {
  assert(!initialized_);
  initialized_ = true;
  return InitImpl();
}

bool ConfigParser::CharStream::AtEOF() const {
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

bool ConfigParser::FileCharStream::InitImpl() {
  assert(!file_);
  file_ = fopen(filename_.c_str(), "r");
  if (!file_) {
    fprintf(stderr, "Unable to open %s: %s\n", filename_.c_str(),
            strerror(errno));
    return false;
  }
  return true;
}

bool ConfigParser::FileCharStream::AtEOFImpl() const {
  assert(file_);
  return feof(file_);
}

char ConfigParser::FileCharStream::GetCharImpl() {
  assert(file_);
  int ch = fgetc(file_);
  assert(ch != EOF);
  return ch;
}

ConfigParser::StringCharStream::StringCharStream(const string& data)
    : data_(data),
      pos_(0) {
}

bool ConfigParser::StringCharStream::AtEOFImpl() const {
  return pos_ == data_.size();
}

char ConfigParser::StringCharStream::GetCharImpl() {
  return data_.at(pos_++);
}

bool ConfigParser::Parse(map<string, Setting*>* settings_map) {
  assert(settings_map);
  assert(settings_map->empty());

  if (!stream_->Init()) {
    SetErrorF("Unable to initialize stream");
    return false;
  }

  enum State {
    NO_SETTING_NAME = 0,
    GOT_SETTING_NAME,
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
        if (settings_map->count(setting_name)) {
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
          settings_map->insert(make_pair(setting_name, setting));
        }
        state = GOT_VALUE;
        break;
      case GOT_VALUE:
        SetErrorF("Got unexpected text after value");
        return false;
    }
  }
  return true;
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

  char ch = stream_->GetChar();
  stream_->UngetChar(ch);

  if (ch >= '0' && ch <= '9') {
    int32 value = 0;
    if (!ReadInteger(&value))
      return false;
    *setting_ptr = new IntegerSetting(value);
  } else if (ch == '"') {
    string value;
    if (!ReadString(&value))
      return false;
    *setting_ptr = new StringSetting(value);
  } else {
    SetErrorF("Got invalid setting value");
    return false;
  }
  return true;
}

bool ConfigParser::ReadInteger(int32* int_out) {
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

    // TODO: Check for overflow.
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
      // TODO: Maybe handle other characters.
    }
    str_out->push_back(ch);
  }

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
