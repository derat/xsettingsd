// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "data_writer.h"

#include <algorithm>
#include <cstring>

using std::min;

namespace xsettingsd {

DataWriter::DataWriter(char* buffer, size_t buf_len)
    : buffer_(buffer),
      buf_len_(buf_len),
      bytes_written_(0) {
}

bool DataWriter::WriteBytes(const char* data, size_t bytes_to_write) {
  if (bytes_to_write > buf_len_ - bytes_written_)
    return false;

  memcpy(buffer_ + bytes_written_,
         data,
         min(bytes_to_write, buf_len_ - bytes_written_));
  bytes_written_ += bytes_to_write;
  return true;
}

bool DataWriter::WriteInt8(int8_t num) {
  if (sizeof(int8_t) > buf_len_ - bytes_written_)
    return false;

  *(reinterpret_cast<int8_t*>(buffer_ + bytes_written_)) = num;
  bytes_written_ += sizeof(int8_t);
  return true;
}

bool DataWriter::WriteInt16(int16_t num) {
  if (sizeof(int16_t) > buf_len_ - bytes_written_)
    return false;

  *(reinterpret_cast<int16_t*>(buffer_ + bytes_written_)) = num;
  bytes_written_ += sizeof(int16_t);
  return true;
}

bool DataWriter::WriteInt32(int32_t num) {
  if (sizeof(int32_t) > buf_len_ - bytes_written_)
    return false;

  *(reinterpret_cast<int32_t*>(buffer_ + bytes_written_)) = num;
  bytes_written_ += sizeof(int32_t);
  return true;
}

bool DataWriter::WriteZeros(size_t bytes_to_write) {
  if (bytes_to_write > buf_len_ - bytes_written_)
    return false;

  bzero(buffer_ + bytes_written_,
        min(bytes_to_write, buf_len_ - bytes_written_));
  bytes_written_ += bytes_to_write;
  return true;
}

}  // namespace xsettingsd
