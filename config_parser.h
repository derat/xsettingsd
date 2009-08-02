#ifndef __XSETTINGSD_CONFIG_PARSER_H__
#define __XSETTINGSD_CONFIG_PARSER_H__

#include "common.h"

#include <string>

namespace xsettingsd {

class ConfigParser {
 public:
  ConfigParser(const std::string& filename);
  ~ConfigParser();

  bool OpenFile();

 private:
  bool AtEOF() const;
  char GetChar();
  void UngetChar(char ch);

  bool ReadSettingName(std::string* name_out);

  // Read an integer starting at the current position in the file.
  bool ReadInteger(int32* int_out);

  // Read a double-quoted string starting at the current position in the
  // file.
  bool ReadString(std::string* str_out);

  bool ReadColor(uint16* red_out, uint16* blue_out,
                 uint16* green_out, uint16* alpha_out);

  std::string filename_;

  FILE* file_;

  bool have_buffered_char_;

  char buffered_char_;
};

}  // namespace

#endif
