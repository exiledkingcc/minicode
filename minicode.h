#ifndef _MINICODE_H_
#define _MINICODE_H_ 1

#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace minicode {

///////////////////////////////////////////////////////////////////////////////
// useful unicode facilities
///////////////////////////////////////////////////////////////////////////////

inline bool is_surrogate_high(std::uint32_t u) {
  return 0xd800 <= u && u <= 0xdbff;
}

inline bool is_surrogate_low(std::uint32_t u) {
  return 0xdc00 <= u && u <= 0xdfff;
}

inline bool is_surrogate(std::uint32_t u) {
  return 0xd800 <= u && u <= 0xdfff;
}

inline std::uint32_t surrogate_combine(std::uint32_t high, std::uint32_t low) {
  return 0x10000 + ((high & 0x3ff) << 10 | (low & 0x3ff));
}

inline void surrogate_split(std::uint32_t u, std::uint32_t& high, std::uint32_t& low) {
  u -= 0x10000;
  low = (u & 0x3ff) | 0xdc00;
  high = ((u >> 10) & 0x3ff) | 0xd800;
}

inline bool is_valid_unicode(std::uint32_t u) {
  return u <= 0x10ffff && !is_surrogate(u);
}

// utf-8 continunation
inline bool is_utf8_cont(std::uint8_t b) {
  return (b & 0xc0) == 0x80;
}


///////////////////////////////////////////////////////////////////////////////
// exceptions, NOT supported now
///////////////////////////////////////////////////////////////////////////////

class encode_error: public std::logic_error {
public:
  encode_error(const char* s):std::logic_error(s){}
  encode_error(const std::string& s):std::logic_error(s){}
};

class decode_error: public std::logic_error {
public:
  decode_error(const char* s):std::logic_error(s){}
  decode_error(const std::string& s):std::logic_error(s){}
};


///////////////////////////////////////////////////////////////////////////////
//  bytes and str class
///////////////////////////////////////////////////////////////////////////////

struct uchar {
  uchar() = default;
  uchar(const uchar&) = default;
  uchar(uchar&&) = default;
  uchar& operator=(const uchar&) = default;
  uchar& operator=(uchar&&) = default;

  uchar(char c):_value(static_cast<std::uint8_t>(c)){}
  explicit uchar(std::uint32_t u):_value(u){}

  bool operator==(const uchar& s) const { return _value == s._value; }
  bool operator!=(const uchar& s) const { return _value != s._value; }

  uint32_t value() const { return _value; }
  uint32_t& value() { return _value; }
private:
  std::uint32_t _value;
};


template<typename T>
class sequence {
public:
  sequence() = default;
  sequence(const sequence&) = default;
  sequence(sequence&&) = default;
  sequence& operator=(const sequence&) = default;
  sequence& operator=(sequence&&) = default;

  sequence(const T* beg, std::size_t n):_data(beg, beg + n){}
  sequence(std::size_t n, const T val = T()):_data(n, val){}

  bool operator==(const sequence& s) const { return _data == s._data; }
  bool operator!=(const sequence& s) const { return _data != s._data; };

  T& operator[](int idx) { idx = idx < 0 ? idx + (int)size() : idx; return _data[idx]; }
  const T& operator[](int idx) const { idx = idx < 0 ? idx + (int)size() : idx; return _data[idx]; }

  std::size_t size() const { return _data.size(); }

  T* data() { return _data.data(); }
  const T* data() const { return _data.data(); }
  const T* limit() const { return _data.data() + _data.size(); }

  void assign(const T* b, const T* e) { _data.assign(b, e); }
private:
  std::vector<T> _data;
};

typedef sequence<char> bytes;
typedef sequence<uchar> str;


///////////////////////////////////////////////////////////////////////////////
//  template functions for encode, decode and convert
///////////////////////////////////////////////////////////////////////////////

template<typename T>
int encode(const str& ss, bytes& bs) {
  const uchar *sb = ss.data();
  const uchar *se = ss.limit();
  std::vector<char> b((se -sb) * 4); // enough buff
  char *bb = b.data();
  char *be = b.data() + b.size();
  T t;
  while (sb < se) {
    int p = t(*sb, bb, be - bb);
    if (p < 0) {
      break;
    } else {
      ++sb;
      bb += p;
    }
  }
  bs.assign(b.data(), bb);
  return se - sb;
}

template<typename T>
int decode(const bytes& bs, str& ss) {
  const char *bb = bs.data();
  const char *be = bs.data() + bs.size();
  std::vector<uchar> s(bs.size()); // enough buff
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
  std::vector<char> b((b1e - b1b) * 4); // enough buff
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
      if (q < 0) {
        break;
      } else {
        bb += q;
      }
    }
  }
  b2.assign(b.data(), bb);
  return b1e - b1b;
}


///////////////////////////////////////////////////////////////////////////////
//  encode and decode operators of each encoding
///////////////////////////////////////////////////////////////////////////////

struct ascii {
  int operator()(const char *bs, int n, uchar& uc) {
    assert(n > 0);
    std::uint32_t& u = uc.value();
    const std::uint8_t *bss = reinterpret_cast<const uint8_t *>(bs);
    if (bss[0] < 0x80) {
      u = bss[0];
      return 1;
    } else {
      return -1;
    }
  }

  int operator()(const uchar uc, char *bs, int n) {
    assert(n > 0);
    const std::uint32_t u = uc.value();
    std::uint8_t *bss = reinterpret_cast<uint8_t *>(bs);
    if (u < 0x80) {
      bss[0] = u;
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
    const std::uint8_t *bss = reinterpret_cast<const uint8_t *>(bs);
    if(bss[0] < 0x80 && n >= 1) {
      u = bss[0];
      return 1;
    } else if (bss[0] < 0xe0 &&
               n >= 2 &&
               is_utf8_cont(bss[1])) {
      u = ((bss[0] & 0x1f) << 6) |
          (bss[1] & 0x3f);
      return 2;
    } else if (bss[0] < 0xf0 &&
               n >= 3 &&
               is_utf8_cont(bss[1]) &&
               is_utf8_cont(bss[2])) {
      u = ((bss[0] & 0x0f) << 12) |
          ((bss[1] & 0x3f) << 6) |
          (bss[2] & 0x3f);
      return 3;
    } else if (bss[0] < 0xf8 &&
               n >= 4 &&
               is_utf8_cont(bss[1]) &&
               is_utf8_cont(bss[2]) &&
               is_utf8_cont(bss[3])) {
      u = ((bss[0] & 0x07) << 18) |
          ((bss[1] & 0x3f) << 12) |
          ((bss[2] & 0x3f) << 6) |
          (bss[3] & 0x3f);
      return 4;
    } else {
      return -1;
    }
  }

  int operator()(const uchar uc, char* bs, int n) {
    assert(n > 0);
    const std::uint32_t u = uc.value();
    if (!is_valid_unicode(u)) {
      return -1;
    }
    std::uint8_t *bss = reinterpret_cast<uint8_t *>(bs);
    if(u < 0x80 && n >= 1) {
      bss[0] = u;
      return 1;
    } else if (u < 0x800 && n >= 2) {
      bss[0] = (u & 07700) >> 6 | 0300;
      bss[1] = (u & 077) | 0200;
      return 2;
    } else if (u < 0x10000 && n >= 3) {
      bss[0] = (u & 0770000) >> 12 | 0340;
      bss[1] = (u & 07700) >> 6 | 0200;
      bss[2] = (u & 077) | 0200;
      return 3;
    } else if (u < 0x110000 && n >= 4) {
      bss[0] = (u & 077000000) >> 18 | 0360;
      bss[1] = (u & 0770000) >> 12 | 0200;
      bss[2] = (u & 07700) >> 6 | 0200;
      bss[3] = (u & 077) | 0200;
      return 4;
    } else {
      return -1;
    }
  }
};

struct utf16le {
  int operator()(const char *bs, int n, uchar& uc) {
    assert(n >= 2);
    std::uint32_t& u = uc.value();
    const std::uint8_t *bss = reinterpret_cast<const uint8_t *>(bs);
    std::uint32_t x = bss[0] | (bss[1] << 8);
    if (is_valid_unicode(x) && n >= 2) {
      u = x;
      return 2;
    } else if (is_surrogate_high(x) && x >= 4){
      std::uint32_t y = bss[2] | (bss[3] << 8);
      if (is_surrogate_low(y)) {
        u = surrogate_combine(x, y);
        return 4;
      } else {
        return -1;
      }
    } else {
      return -1;
    }
  }

  int operator()(const uchar uc, char* bs, int n) {
    assert(n >= 2);
    const std::uint32_t u = uc.value();
    std::uint8_t *bss = reinterpret_cast<uint8_t *>(bs);
    if (u < 0x10000 && !is_surrogate(u) && n >= 2) {
      bss[0] = u & 0xff;
      bss[1] = (u >> 8) & 0xff;
      return 2;
    } else if (0x10000 <= u && u <= 0x10ffff && n >= 4) {
      std::uint32_t x, y;
      surrogate_split(u, x, y);
      bss[0] = x & 0xff;
      bss[1] = (x >> 8) & 0xff;
      bss[2] = y & 0xff;
      bss[3] = (y >> 8) & 0xff;
      return 4;
    } else {
      return -1;
    }
  }
};

struct utf16be {
  int operator()(const char *bs, int n, uchar& uc) {
    assert(n >= 2);
    std::uint32_t& u = uc.value();
    const std::uint8_t *bss = reinterpret_cast<const uint8_t *>(bs);
    std::uint32_t x = bss[1] | (bss[0] << 8);
    if (is_valid_unicode(x) && n >= 2) {
      u = x;
      return 2;
    } else if (is_surrogate_high(x) && x >= 4){
      std::uint32_t y = bss[3] | (bss[2] << 8);
      if (is_surrogate_low(y)) {
        u = surrogate_combine(x, y);
        return 4;
      } else {
        return -1;
      }
    } else {
      return -1;
    }
  }

  int operator()(const uchar uc, char* bs, int n) {
    assert(n >= 2);
    const std::uint32_t u = uc.value();
    std::uint8_t *bss = reinterpret_cast<uint8_t *>(bs);
    if (u < 0x10000 && !is_surrogate(u) && n >= 2) {
      bss[1] = u & 0xff;
      bss[0] = (u >> 8) & 0xff;
      return 2;
    } else if (0x10000 <= u && u <= 0x10ffff && n >= 4) {
      std::uint32_t x, y;
      surrogate_split(u, x, y);
      bss[1] = x & 0xff;
      bss[0] = (x >> 8) & 0xff;
      bss[3] = y & 0xff;
      bss[2] = (y >> 8) & 0xff;
      return 4;
    } else {
      return -1;
    }
  }
};

struct utf32le {
  int operator()(const char *bs, int n, uchar& uc) {
    assert(n >= 4);
    std::uint32_t& u = uc.value();
    const std::uint8_t *bss = reinterpret_cast<const uint8_t *>(bs);
    std::uint32_t x = bss[0] | (bss[1] << 8) | (bss[2] << 16) | (bss[3] << 24);
    if (is_valid_unicode(x)) {
      u = x;
      return 4;
    } else {
      return -1;
    }
  }

  int operator()(const uchar uc, char* bs, int n) {
    assert(n >= 4);
    const std::uint32_t u = uc.value();
    std::uint8_t *bss = reinterpret_cast<uint8_t *>(bs);
    if (is_valid_unicode(u)) {
      bss[0] = u & 0xff;
      bss[1] = (u >> 8) & 0xff;
      bss[2] = (u >> 16) & 0xff;
      bss[3] = (u >>24) & 0xff;
      return 4;
    } else {
      return -1;
    }
  }
};

struct utf32be {
  int operator()(const char *bs, int n, uchar& uc) {
    assert(n >= 4);
    std::uint32_t& u = uc.value();
    const std::uint8_t *bss = reinterpret_cast<const uint8_t *>(bs);
    std::uint32_t x = bss[3] | (bss[2] << 8) | (bss[1] << 16) | (bss[0] << 24);
    if (is_valid_unicode(x)) {
      u = x;
      return 4;
    } else {
      return -1;
    }
  }

  int operator()(const uchar uc, char* bs, int n) {
    assert(n >= 4);
    const std::uint32_t u = uc.value();
    std::uint8_t *bss = reinterpret_cast<uint8_t *>(bs);
    if (is_valid_unicode(u)) {
      bss[3] = u & 0xff;
      bss[2] = (u >> 8) & 0xff;
      bss[1] = (u >> 16) & 0xff;
      bss[0] = (u >>24) & 0xff;
      return 4;
    } else {
      return -1;
    }
  }
};


} // namespace minicode

#endif // _MINICODE_H_
