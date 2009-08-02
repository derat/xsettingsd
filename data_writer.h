#ifndef __XSETTINGSD_DATA_WRITER_H__
#define __XSETTINGSD_DATA_WRITER_H__

#include <cstdlib>  // for size_t

#include "common.h"

namespace xsettingsd {

class DataWriter {
 public:
  DataWriter(char* buffer, size_t buf_len);

  size_t bytes_written() const { return bytes_written_; }

  bool WriteBytes(const char* data, size_t bytes_to_write);
  bool WriteInt8(int8 num);
  bool WriteInt16(int16 num);
  bool WriteInt32(int32 num);
  bool WriteZeros(size_t bytes_to_write);

 private:
  char* buffer_;  // not owned

  size_t buf_len_;

  size_t bytes_written_;
};

}  // namespace xsettingsd

#endif
