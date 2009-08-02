#include "config_parser.h"

#include <cassert>
#include <cerrno>
#include <cstring>

using std::string;

namespace xsettingsd {

ConfigParser::ConfigParser(const string& filename)
    : filename_(filename),
      file_(NULL),
      have_buffered_char_(false),
      buffered_char_(0) {
}

ConfigParser::~ConfigParser() {
  if (file_)
    fclose(file_);
}

bool ConfigParser::OpenFile() {
  assert(!file_);
  file_ = fopen(filename_.c_str(), "r");
  if (!file_) {
    fprintf(stderr, "Unable to open %s: %s\n", filename_.c_str(),
            strerror(errno));
    return false;
  }
  return true;
}

bool ConfigParser::AtEOF() const {
  assert(file_);
  return (!have_buffered_char_ && feof(file_));
}

char ConfigParser::GetChar() {
  assert(file_);
  if (have_buffered_char_) {
    have_buffered_char_ = false;
    return buffered_char_;
  }
  int ch = fgetc(file_);
  assert(ch != EOF);
  return ch;
}

void ConfigParser::UngetChar(char ch) {
  assert(!have_buffered_char_);
  buffered_char_ = ch;
  have_buffered_char_ = true;
}

bool ConfigParser::ReadSettingName(string* name_out) {
  assert(name_out);
  name_out->clear();

  bool prev_was_slash = false;
  while (true) {
    if (AtEOF())
      break;

    char ch = GetChar();
    if (isspace(ch)) {
      UngetChar(ch);
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

}  // namespace xsettingsd
