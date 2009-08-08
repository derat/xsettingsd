// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __XSETTINGSD_DATA_READER_H__
#define __XSETTINGSD_DATA_READER_H__

#include <cstdlib>  // for size_t
#include <stdint.h>
#include <string>

#include "common.h"

namespace xsettingsd {

// Like DataWriter, but not.
class DataReader {
 public:
  DataReader(const char* buffer, size_t buf_len);

  void set_reverse_bytes(bool reverse) { reverse_bytes_ = reverse; }

  size_t bytes_read() const { return bytes_read_; }

  bool ReadBytes(std::string* out, size_t size);
  bool ReadInt8(int8_t* out);
  bool ReadInt16(int16_t* out);
  bool ReadInt32(int32_t* out);

 private:
  const char* buffer_;  // not owned

  size_t buf_len_;

  size_t bytes_read_;

  bool reverse_bytes_;

  DISALLOW_COPY_AND_ASSIGN(DataReader);
};

}  // namespace xsettingsd

#endif
