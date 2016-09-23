#include <iostream>
#include <fstream>
#include <string>
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
}


int main() {
  bytes utf8 = read_file("utf8.txt");
  bytes utf16be = read_file("utf16be.txt");
  bytes utf16le = read_file("utf16le.txt");
  bytes utf32be = read_file("utf32be.txt");
  bytes utf32le = read_file("utf32le.txt");

  cout<<"test <utf8, utf8> ..."<<endl;
  test_convert<minicode::utf8, minicode::utf8>(utf8, utf8);

  cout<<"test <utf8, utf16le> ..."<<endl;
  test_convert<minicode::utf8, minicode::utf16le>(utf8, utf16le);

  cout<<"test <utf8, utf16be> ..."<<endl;
  test_convert<minicode::utf8, minicode::utf16be>(utf8, utf16be);

  cout<<"test <utf16le, utf16le> ..."<<endl;
  test_convert<minicode::utf16le, minicode::utf16le>(utf16le, utf16le);

  cout<<"test <utf16le, utf16be> ..."<<endl;
  test_convert<minicode::utf16le, minicode::utf16be>(utf16le, utf16be);

  cout<<"test <utf16be, utf16be> ..."<<endl;
  test_convert<minicode::utf16be, minicode::utf16be>(utf16be, utf16be);

  return 0;
}
