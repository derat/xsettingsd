#include "config_parser.h"

#include <cassert>
#include <cerrno>
#include <cstring>

using std::string;

namespace xsettingsd {

ConfigParser::ConfigParser(CharStream* stream)
    : stream_(stream) {
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

bool ConfigParser::Parse() {
  stream_->Init();

  string name;
  ReadSettingName(&name);
}

bool ConfigParser::ReadSettingName(string* name_out) {
  assert(name_out);
  name_out->clear();

  bool prev_was_slash = false;
  while (true) {
    if (stream_->AtEOF())
      break;

    char ch = stream_->GetChar();
    if (isspace(ch)) {
      stream_->UngetChar(ch);
      break;
    }

    if (!(ch >= 'A' && ch <= 'Z') &&
        !(ch >= 'a' && ch <= 'z') &&
        !(ch >= '0' && ch <= '9') &&
        !(ch == '_' || ch == '/')) {
      fprintf(stderr, "Got invalid character '%c' in setting name\n", ch);
      return false;
    }

    if (ch == '/' && name_out->empty()) {
      fprintf(stderr, "Got leading slash in setting name\n");
      return false;
    }

    if (prev_was_slash) {
      if (ch == '/') {
        fprintf(stderr, "Got two consecutive slashes in setting name\n");
        return false;
      }
      if (ch >= '0' && ch <= '9') {
        fprintf(stderr, "Got digit after slash in setting name\n");
        return false;
      }
    }

    name_out->push_back(ch);

    if (ch == '/')
      prev_was_slash = true;
  }

  if (name_out->empty()) {
    fprintf(stderr, "Got empty setting name\n");
    return false;
  }

  if (name_out->at(name_out->size() - 1) == '/') {
    fprintf(stderr, "Got trailing slash in setting name\n");
    return false;
  }

  return true;
}

bool ConfigParser::ReadString(string* str_out) {
  assert(str_out);
  str_out->clear();
}

}  // namespace xsettingsd
