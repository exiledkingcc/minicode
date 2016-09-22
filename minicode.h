#ifndef _MINICODE_H_
#define _MINICODE_H_ 1

#include <cstdint>
#include <vector>

namespace minicode {

struct uchar {
  uchar() = default;
  uchar(const uchar) = default;
  uchar(uchar&&) = default;
  uchar& operator=(const uchar&) = default;
  uchar& operator=(uchar&&) = default;

  uint32_t value() const { return _value; }
  uint32_t& value() { return _value; }
private:
  std::uint32_t _value;
};


template<typename T>
class sequence {
  sequence() = default;
  sequence(const sequence) = default;
  sequence(sequence&&) = default;
  sequence& operator=(const sequence&) = default;
  sequence& operator=(sequence&&) = default;

  std::size_t size() const { return _data.size(); }

  T* data() { return _data.data(); }
  const T* data() const { return _data.data(); }
  const T* limit() const { return _data.data() + _data.size(); }

  void assign(const T *b, const T *e) { _data.assign(b, e); }
private:
  std::vector<T> _data;
};

typedef sequence<char> bytes;
typedef sequence<uchar> str;


struct ascii {
  int operator()(const char *bs, int n, uchar& uc) {
    assert(n > 0);
    std::uint32_t& u = uc.value();
    const std::uint8_t *bytes = reinterpret_cast<const uint8_t *>(bs);
    if (bytes[0] < 0x80) {
      u = bytes[0];
      return 1;
    } else {
      return -1;
    }
  }

  int operator()(const uchar uc, char *bs, int n) {
    assert(n > 0);
    const std::uint32_t u = uc.value();
    std::uint8_t *bytes = reinterpret_cast<uint8_t *>(bs);
    if (u < 0x80) {
      bytes[0] = u;
      return 1;
    } else {
      return -1;
    }
  }
};

struct utf8 {
  int operator()(const char *bs, int n, uchar& uc) {
    assert(n > 0);
    std::uint32_t& u = uc.value();
    const std::uint8_t *bytes = reinterpret_cast<const uint8_t *>(bs);
    if(bytes[0] < 0x80 && n >= 1) {
      u = bytes[0];
      return 1;
    } else if (bytes[0] < 0xe0 && n >= 2) {
      u = ((bytes[0] & 0x1f) << 6) |
          (bytes[1] & 0x3f);
      return 2;
    } else if (bytes[0] < 0xf0 && n >= 3) {
      u = ((bytes[0] & 0x0f) << 12) |
          ((bytes[1] & 0x3f) << 6) |
          (bytes[2] & 0x3f);
      return 3;
    } else if (bytes[0] < 0xf8 && n >= 4) {
      u = ((bytes[0] & 0x07) << 18) |
          ((bytes[1] & 0x3f) << 12) |
          ((bytes[2] & 0x3f) << 6) |
          (bytes[3] & 0x3f);
      return 4;
    } else if (bytes[0] < 0xfc && n >= 5) {
      u = ((bytes[0] & 0x03) << 24) |
          ((bytes[1] & 0x3f) << 18) |
          ((bytes[2] & 0x3f) << 12) |
          ((bytes[3] & 0x3f) << 6) |
          (bytes[4] & 0x3f);
      return 5;
    } else if (bytes[0] < 0xfe && n >= 6) {
      u = ((bytes[0] & 0x01) << 30) |
          ((bytes[1] & 0x3f) << 24) |
          ((bytes[2] & 0x3f) << 18) |
          ((bytes[3] & 0x3f) << 12) |
          ((bytes[4] & 0x3f) << 6) |
          (bytes[5] & 0x3f);
      return 6;
    } else {
      return -1;
    }
  }

  int operator()(const uchar uc, char* bs, int n) {
    assert(n > 0);
    const std::uint32_t u = uc.value();
    std::uint8_t *bytes = reinterpret_cast<uint8_t *>(bs);
    if(u < 0x80 && n >= 1) {
      bytes[0] = u;
      return 1;
    } else if (u < 0x800 && n >= 2) {
      bytes[0] = (u & 07700) >> 6 | 0300;
      bytes[1] = (u & 077) | 0200;
      return 2;
    } else if (u < 0x10000 && n >= 3) {
      bytes[0] = (u & 0770000) >> 12 | 0340;
      bytes[1] = (u & 07700) >> 6 | 0200;
      bytes[2] = (u & 077) | 0200;
      return 3;
    } else if (u < 0x200000 && n >= 4) {
      bytes[0] = (u & 077000000) >> 18;
      bytes[1] = (u & 0770000) >> 12 | 0200;
      bytes[2] = (u & 07700) >> 6 | 0200;
      bytes[3] = (u & 077) | 0200;
      return 4;
    } else if (u < 0x4000000 && n >= 5) {
      bytes[0] = (u & 07700000000) >> 24 | 0373;
      bytes[1] = (u & 077000000) >> 18 | 0200;
      bytes[2] = (u & 0770000) >> 12 | 0200;
      bytes[3] = (u & 07700) >> 6 | 0200;
      bytes[4] = (u & 077) | 0200;
      return 5;
    } else if (u < 0x80000000 && n >= 6) {
      bytes[0] = (u & 017000000000) >> 30;
      bytes[1] = (u & 07700000000) >> 24 | 0200;
      bytes[2] = (u & 077000000) >> 18 | 0200;
      bytes[3] = (u & 0770000) >> 12 | 0200;
      bytes[4] = (u & 07700) >> 6 | 0200;
      bytes[5] = (u & 077) | 0200;
      return 6;
    } else {
      return -1;
    }
  }
};

struct utf16le {};
struct utf16be {};
struct utf32le {};
struct utf32be {};

template<typename T>
int encode(const str& ss, bytes& bs) {
  const uchar *sb = ss.data();
  const uchar *se = ss.limit();
  std::vector<char> b(n * 6); // enough buff
  char *bb = b.data();
  char *be = b.data() + b.size();
  T t;
  while (sb < se) {
    int p = t1(*sb, bb, be - bb);
    if (p < 0) {
      break;
    } else {
      ++sb;
      bb += p;
    }
  }
  bs.assign(b.data(), bb);
  return sb - str.data();
}

template<typename T>
int decode(const bytes& bs, str& ss) {
  const char *bb = bs.data();
  const char *be = bs.data() + bs.size();
  std::vector<uchar> s(n); // enough buff
  uchar *sb = s.data();
  T t;
  while (bb < be) {
    int p = t(bb, be - bb, *sb);
    if (p < 0) {
      break;
    } else {
      bb += p;
      ++sb;
    }
  }
  ss.assign(s.data(), sb);
  return be - bb;
}

template<typename T1, typename T2>
int convert(const bytes& b1, bytes& b2) {
  const char *b1b = b1.data();
  const char *b1e = b1.limit();
  std::vector<char> b(n * 4); // enough buff
  char* bb = b.data();
  const char* be = b.data() + b.size();
  T1 t1;
  T2 t2;
  while (b1b < b1e) {
    uchar u;
    int p = t1(b1b, b1e - b1b, u);
    if (p < 0) {
      break;
    } else {
      b1b += p;
      int q = t2(u, bb, be - bb);
      bb += q;
    }
  }
  b2.assign(b.data(), bb);
  return b1e - b1b;
}


} // namespace minicode

#endif // _MINICODE_H_
