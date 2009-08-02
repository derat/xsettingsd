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

bool DataWriter::WriteInt8(int8 num) {
  if (sizeof(int8) > buf_len_ - bytes_written_)
    return false;

  *(reinterpret_cast<int8*>(buffer_ + bytes_written_)) = num;
  bytes_written_ += sizeof(int8);
  return true;
}

bool DataWriter::WriteInt16(int16 num) {
  if (sizeof(int16) > buf_len_ - bytes_written_)
    return false;

  *(reinterpret_cast<int16*>(buffer_ + bytes_written_)) = num;
  bytes_written_ += sizeof(int16);
  return true;
}

bool DataWriter::WriteInt32(int32 num) {
  if (sizeof(int32) > buf_len_ - bytes_written_)
    return false;

  *(reinterpret_cast<int32*>(buffer_ + bytes_written_)) = num;
  bytes_written_ += sizeof(int32);
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
