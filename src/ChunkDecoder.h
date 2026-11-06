#pragma once
#include <stdint.h>

// Decodes an HTTP/1.1 "Transfer-Encoding: chunked" body byte by byte, so
// ArduinoJson can keep parsing straight from the TLS stream without an extra
// buffer. Source needs `int nextByte()` returning -1 on timeout or close.
// Chunk extensions and trailers are skipped; malformed framing ends the body.
template <typename Source>
class ChunkDecoder {
public:
  explicit ChunkDecoder(Source& src, bool chunked) : _src(src), _chunked(chunked) {}

  int read() {
    if (!_chunked) return _src.nextByte();
    if (_done) return -1;
    if (_remaining == 0 && !readChunkHeader()) { _done = true; return -1; }
    int c = _src.nextByte();
    if (c < 0) { _done = true; return -1; }
    if (--_remaining == 0) {
      // Every chunk is followed by CRLF.
      if (_src.nextByte() != '\r' || _src.nextByte() != '\n') _done = true;
    }
    return c;
  }

  bool failed() const { return _malformed; }

private:
  bool readChunkHeader() {
    uint32_t size = 0;
    uint8_t digits = 0;
    bool extension = false;
    for (;;) {
      int c = _src.nextByte();
      if (c < 0) return false;
      if (c == '\r') continue;
      if (c == '\n') break;
      if (extension) continue;
      int v = -1;
      if (c >= '0' && c <= '9') v = c - '0';
      else if (c >= 'a' && c <= 'f') v = c - 'a' + 10;
      else if (c >= 'A' && c <= 'F') v = c - 'A' + 10;
      if (v < 0) {
        if (c == ';' || c == ' ' || c == '\t') { extension = true; continue; }
        _malformed = true; return false;
      }
      if (++digits > 7) { _malformed = true; return false; }  // > 256 MiB: not for us
      size = (size << 4) | (uint32_t)v;
    }
    if (!digits) { _malformed = true; return false; }
    _remaining = size;
    return size != 0;  // last chunk: body complete, trailers ignored
  }

  Source& _src;
  bool _chunked;
  bool _done = false;
  bool _malformed = false;
  uint32_t _remaining = 0;
};
