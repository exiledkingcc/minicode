# minicode
A mini Unicode library, providing python-like API for Unicode.

### feature
- one header only
- python-like API

### requirement
C++11 support

### usage
- use `minicode::bytes` as binary data sequence, like bytes in Python3.
- use `minicode::str` as Unicode character sequence, like str in Python3.
- use `minicode::encode` to encode Unicode string into binary data.
- use `minicode::decode` to decode Unicode string from binary data.
- use `minicode::convert` to convert binary data from one encoding to another.
- use `minicode::utf8`, `minicode::utf16le`, `minicode::utf16be`,
`minicode::utf32le`, `minicode::utf32be` to specify the encoding.

examples:

```c++
minicode::bytes b, b1, b2;
minicode::str s;
//...
minicode::encode<minicde::utf8>(s, b); // encode s using utf-8
minicode::encode<minicde::utf16le>(s, b); // encode s using utf-16le
//...
minicode::decode<minicde::utf8>(b, s); // decode b and store to s using utf-8
minicode::decode<minicde::utf32be>(b, s); // decode b and store to s using utf-32be
//...
minicode::convert<minicde::utf8, minicode::utf16be>(b1, b2); // convert utf-8 encoding to utf-16be encoding
```
