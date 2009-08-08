// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __XSETTINGSD_DATA_WRITER_H__
#define __XSETTINGSD_DATA_WRITER_H__

#include <cstdlib>  // for size_t
#include <stdint.h>

#include "common.h"

namespace xsettingsd {

// Provides an interface for writing different types of data to a buffer.
class DataWriter {
 public:
  DataWriter(char* buffer, size_t buf_len);

  size_t bytes_written() const { return bytes_written_; }

  bool WriteBytes(const char* data, size_t bytes_to_write);
  bool WriteInt8(int8_t num);
  bool WriteInt16(int16_t num);
  bool WriteInt32(int32_t num);
  bool WriteZeros(size_t bytes_to_write);

 private:
  char* buffer_;  // not owned

  size_t buf_len_;

  size_t bytes_written_;

  DISALLOW_COPY_AND_ASSIGN(DataWriter);
};

}  // namespace xsettingsd

#endif
