// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "data_reader.h"

#include <arpa/inet.h>

using std::string;

namespace xsettingsd {

DataReader::DataReader(const char* buffer, size_t buf_len)
    : buffer_(buffer),
      buf_len_(buf_len),
      bytes_read_(0),
      reverse_bytes_(false) {
}

bool DataReader::ReadBytes(string* out, size_t size) {
  if (bytes_read_ + size > buf_len_ ||
      bytes_read_ + size < bytes_read_) {
    return false;
  }
  if (out)
    out->assign(buffer_ + bytes_read_, size);
  bytes_read_ += size;
  return true;
}

bool DataReader::ReadInt8(int8_t* out) {
  if (bytes_read_ + sizeof(int8_t) > buf_len_ ||
      bytes_read_ + sizeof(int8_t) < bytes_read_) {
    return false;
  }

  if (out)
    *out = *reinterpret_cast<const int8_t*>(buffer_ + bytes_read_);
  bytes_read_ += sizeof(int8_t);
  return true;
}

bool DataReader::ReadInt16(int16_t* out) {
  if (bytes_read_ + sizeof(int16_t) > buf_len_ ||
      bytes_read_ + sizeof(int16_t) < bytes_read_) {
    return false;
  }

  if (out) {
    *out = *reinterpret_cast<const int16_t*>(buffer_ + bytes_read_);
    if (reverse_bytes_) {
      if (IsLittleEndian())
        *out = ntohs(*out);
      else
        *out = htons(*out);
    }
  }
  bytes_read_ += sizeof(int16_t);
  return true;
}

bool DataReader::ReadInt32(int32_t* out) {
  if (bytes_read_ + sizeof(int32_t) > buf_len_ ||
      bytes_read_ + sizeof(int32_t) < bytes_read_) {
    return false;
  }

  if (out) {
    *out = *reinterpret_cast<const int32_t*>(buffer_ + bytes_read_);
    if (reverse_bytes_) {
      if (IsLittleEndian())
        *out = ntohl(*out);
      else
        *out = htonl(*out);
    }
  }
  bytes_read_ += sizeof(int32_t);
  return true;
}

}  // namespace xsettingsd
