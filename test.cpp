#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "minicode.h"

using namespace std;
using minicode::bytes;
using minicode::str;

bytes read_file(const string& filename) {
  ifstream file(filename, std::ios::binary | std::ios::ate);
  streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  file.read(buffer.data(), size);
  return bytes(buffer.data(), buffer.size());
}

template<typename T1, typename T2>
void test_convert(const bytes& b1, const bytes& b2) {
  bytes b3, b4;
  int r1 = minicode::convert<T1, T2>(b1, b3);
  if (r1 != 0) {
    cerr<<"convert failed!"<<endl;
  }
  int r2 = minicode::convert<T2, T1>(b2, b4);
  if (r2 != 0) {
    cerr<<"convert failed!"<<endl;
  }
  if (r1 == 0 && r2 == 0) {
    bool equal1 = (b2 == b3);
    bool equal2 = (b1 == b4);
    cout<<boolalpha<<"compare "<<equal1<<" "<<equal2<<endl;
  }
  cout<<endl;
}

str read_unicode(const string& filename) {
  ifstream file(filename);
  vector<minicode::uchar> vv;
  char buff[16];
  while (file.getline(buff, 16)) {
    uint32_t x = strtoul(buff, nullptr, 10);
    vv.push_back(minicode::uchar(x));
  }
  str ss;
  ss.assign(vv.data(), vv.data() + vv.size());
  return ss;
}

template<typename T>
void test_encode_decode(const str& ss, const bytes& bb) {
  str s1;
  bytes b1;
  int x = minicode::encode<T>(ss, b1);
  if (x != 0) {
    cout<<"encode error"<<endl;
  }
  int y = minicode::decode<T>(bb, s1);
  if (y != 0) {
    cout<<"decode error"<<endl;
  }
  if (x == 0 && y == 0) {
    bool equal1 = (bb == b1);
    bool equal2 = (ss == s1);
    cout<<boolalpha<<"compare "<<equal1<<" "<<equal2<<endl;
  }
  cout<<endl;
}

int main() {
  str unicode = read_unicode("unicode.txt");
  bytes utf8 = read_file("utf8.txt");
  bytes utf16be = read_file("utf16be.txt");
  bytes utf16le = read_file("utf16le.txt");
  bytes utf32be = read_file("utf32be.txt");
  bytes utf32le = read_file("utf32le.txt");

  cout<<"test convert <utf8, utf8> ..."<<endl;
  test_convert<minicode::utf8, minicode::utf8>(utf8, utf8);

  cout<<"test convert <utf8, utf16le> ..."<<endl;
  test_convert<minicode::utf8, minicode::utf16le>(utf8, utf16le);

  cout<<"test convert <utf8, utf16be> ..."<<endl;
  test_convert<minicode::utf8, minicode::utf16be>(utf8, utf16be);

  cout<<"test convert <utf16le, utf16le> ..."<<endl;
  test_convert<minicode::utf16le, minicode::utf16le>(utf16le, utf16le);

  cout<<"test convert <utf16le, utf16be> ..."<<endl;
  test_convert<minicode::utf16le, minicode::utf16be>(utf16le, utf16be);

  cout<<"test convert <utf16be, utf16be> ..."<<endl;
  test_convert<minicode::utf16be, minicode::utf16be>(utf16be, utf16be);

  cout<<"test convert <utf8, utf32le> ..."<<endl;
  test_convert<minicode::utf8, minicode::utf32le>(utf8, utf32le);

  cout<<"test convert <utf8, utf32be> ..."<<endl;
  test_convert<minicode::utf8, minicode::utf32be>(utf8, utf32be);

  cout<<"test convert <utf16le, utf32le> ..."<<endl;
  test_convert<minicode::utf16le, minicode::utf32le>(utf16le, utf32le);

  cout<<"test convert <utf16le, utf32be> ..."<<endl;
  test_convert<minicode::utf16le, minicode::utf32be>(utf16le, utf32be);

  cout<<"test convert <utf16be, utf32le> ..."<<endl;
  test_convert<minicode::utf16be, minicode::utf32le>(utf16be, utf32le);

  cout<<"test convert <utf16be, utf32be> ..."<<endl;
  test_convert<minicode::utf16be, minicode::utf32be>(utf16be, utf32be);

  cout<<"test convert <utf32le, utf32le> ..."<<endl;
  test_convert<minicode::utf32le, minicode::utf32le>(utf32le, utf32le);

  cout<<"test convert <utf32le, utf32be> ..."<<endl;
  test_convert<minicode::utf32le, minicode::utf32be>(utf32le, utf32be);

  cout<<"test convert <utf32be, utf32be> ..."<<endl;
  test_convert<minicode::utf32be, minicode::utf32be>(utf32be, utf32be);

  cout<<"test encode, decode <uft8> ..."<<endl;
  test_encode_decode<minicode::utf8>(unicode, utf8);

  cout<<"test encode, decode <uft16le> ..."<<endl;
  test_encode_decode<minicode::utf16le>(unicode, utf16le);

  cout<<"test encode, decode <uft16be> ..."<<endl;
  test_encode_decode<minicode::utf16be>(unicode, utf16be);

  cout<<"test encode, decode <uft32le> ..."<<endl;
  test_encode_decode<minicode::utf32le>(unicode, utf32le);

  cout<<"test encode, decode <uft32be> ..."<<endl;
  test_encode_decode<minicode::utf32be>(unicode, utf32be);

  return 0;
}
