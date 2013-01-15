#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <iterator>
#include <vector>
#include <bitset>
#include <map>
#include <cassert>

using std::cout;
using std::max;
using std::min;
using std::endl;
using std::setiosflags;
using std::iterator;
using std::vector;
using std::string;
using std::stringstream;
using std::abs;
using std::setprecision;
using std::setw;
using std::setfill;
using std::bitset;
using std::map;
using std::ofstream;
using std::ifstream;
using std::find;

#define LOGGER

#ifdef LOGGER
extern string GLog;
#define LOG(t) { string log = (t); \
  if (log.size() > 0) { GLog = GLog + "\n" + log; } };
#define RLOG(t) { GLog = (t); };
#else
#define LOG(t);
#define RLOG(t);
#endif
const string SLASH = "/";
const string DATA_DIR = "data";
const string STORY_DIR = DATA_DIR + SLASH + "books";
const string MENU_DIR = DATA_DIR + SLASH + "menu";
const string FONTS_DIR = DATA_DIR + SLASH + "fonts";
const string FRAMES_DIR = DATA_DIR + SLASH + "frames";

const size_t BLOCK_SIZE = 32;

typedef float real;
typedef unsigned short int usint;
typedef unsigned long int ulint;
typedef long int lint;
typedef unsigned int uint;
typedef unsigned char uchar;

typedef struct string_pair {
  string_pair(string _X, string _Y) : X(_X), Y(_Y) { };
  string_pair() { };
  void clear() {
    X.clear();
    Y.clear();
  };
  bool empty() {
    return (X.empty() || Y.empty());
  };
  bool full() {
    return (!X.empty() && !Y.empty());
  };
  size_t size();
  const char* c_str();
  string X;
  string Y;
} string_pair;

typedef struct size_t_pair {
  size_t_pair() { };
  size_t_pair(size_t _X, size_t _Y) : X(_X), Y(_Y) { };
  size_t X = 0;
  size_t Y = 0;
} size_t_pair;
typedef struct real_pair {
  real_pair() { };
  real_pair(real _X, real _Y) : X(_X), Y(_Y) { };
  real X = 0;
  real Y = 0;
} real_pair;

typedef struct Rect {
  Rect() : W(0), H(0), X(0), Y(0) { };
  Rect(size_t _W, size_t _H, size_t _X = 0, size_t _Y = 0)
    : W(_W), H(_H), X(_X), Y(_Y) { };
  bool operator!= (const Rect& other) const {
    return !(*this == other);
  };
  bool operator== (const Rect& other) const {
    return W == other.W && H == other.H && X == other.X && Y == other.Y;
  };
  // align size to block size
  void Blockify() {
    W = max(W, 2u*(size_t)BLOCK_SIZE);
    H = max(H, 2u*(size_t)BLOCK_SIZE);
    size_t newW = W - (W % BLOCK_SIZE);
    size_t newH = H - (H % BLOCK_SIZE);
    X += (W - newW) / (size_t)2;
    Y += (H - newH) / (size_t)2;
    W = newW;
    H = newH;
  };
  size_t W;
  size_t H;
  size_t X;
  size_t Y;
} Rect;

template <typename T> inline string IntoString(const T& Thing)
{
  stringstream stream;
  stream << Thing;
  return stream.str();
}

template <typename T> inline real IntoReal(const T& Thing)
{
  stringstream stream;
  real real;
  stream << Thing;
  stream >> real;
  return real;
}

template <typename T> inline int IntoInt(const T& Thing)
{
  stringstream stream;
  int integer;
  stream << Thing;
  stream >> integer;
  return integer;
}

inline string RealIntoString(const real a_real,
                             const usint a_precision = 3)
{
  stringstream stream;
  stream << setprecision(a_precision) << a_real;
  return stream.str();
}

// cutString("01234", 2, 4) returns "23"
inline string CutString(const string Text,
                        const size_t Start,
                        const size_t End = string::npos)
{
  return Text.substr(Start, End - Start);
}

template <typename T> inline void Clamp(T& Value,
                                        const T& Min,
                                        const T& Max)
{
  Value = Value < Min? Min : (Value > Max? Max : Value);
}

template <typename T> inline void ClampZero(T& Value,
    const T& Max)
{
  Value = Value < 0? 0 : (Value > Max? Max : Value);
}

template <typename T> inline void ClampZeroOne(T& Value)
{
  Value = Value < 0? 0 : (Value > 1? 1 : Value);
}

#endif // MAIN_H_INCLUDED
