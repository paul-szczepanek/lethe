#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <iterator>
#include <functional>
#include <vector>
#include <bitset>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <cmath>
#include <cassert>
#include <stack>

using std::cout;
using std::max;
using std::min;
using std::endl;
using std::pair;
using std::log;
using std::setiosflags;
using std::sqrt;
using std::log10;
using std::iterator;
using std::vector;
using std::stack;
using std::list;
using std::string;
using std::stringstream;
using std::abs;
using std::setprecision;
using std::setw;
using std::setfill;
using std::bitset;
using std::map;
using std::multimap;
using std::ofstream;
using std::ifstream;
using std::make_pair;
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

const string STORY_DIR = "data/books/";
const string FONTS_DIR = "data/fonts/";
const string FRAMES_DIR = "data/frames/";
const string KEYWORDS_DIR = "data/cache/keywords/";

const size_t BLOCK_SIZE = 32;

typedef float real;
typedef unsigned short int usint;
typedef unsigned long int ulint;
typedef long int lint;
typedef unsigned int uint;
typedef unsigned char uchar;

typedef struct size_t_pair {
  size_t_pair() : X(0), Y(0) { };
  size_t_pair(size_t _X, size_t _Y) : X(_X), Y(_Y) { };
  size_t X;
  size_t Y;
} size_t_pair;

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

typedef struct int_pair {
  int_pair() : X(0), Y(0) { };
  int X;
  int Y;
} int_pair;
typedef struct uint_pair {
  uint_pair() : X(0), Y(0) { };
  uint X;
  uint Y;
} uint_pair;
typedef struct usint_pair {
  usint_pair() : X(0), Y(0) { };
  usint X;
  usint Y;
} usint_pair;
typedef struct ulint_pair {
  ulint_pair() : X(0), Y(0) { };
  ulint X;
  ulint Y;
} ulint_pair;
typedef struct real_pair {
  real_pair() : X(0), Y(0) { };
  real_pair(real _X, real _Y) : X(_X), Y(_Y) { };
  real X;
  real Y;
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

template <typename T> inline void clamp(T& Value, const T& Min, const T& Max)
{
  Value = Value < Min? Min : (Value > Max? Max : Value);
}

template <typename T> inline void clampZero(T& Value, const T& Max)
{
  Value = Value < 0? 0 : (Value > Max? Max : Value);
}

template <typename T> inline void clampZeroOne(T& Value)
{
  Value = Value < 0? 0 : (Value > 1? 1 : Value);
}

template <typename T> inline string intoString(const T& Thing)
{
  stringstream stream;
  stream << Thing;
  return stream.str();
}

template <typename T> inline real intoReal(const T& Thing)
{
  stringstream stream;
  real real;
  stream << Thing;
  stream >> real;
  return real;
}

template <typename T> inline int intoInt(const T& Thing)
{
  stringstream stream;
  int integer;
  stream << Thing;
  stream >> integer;
  return integer;
}

inline string realIntoString(const real a_real, const usint a_precision = 3)
{
  stringstream stream;
  stream << setprecision(a_precision) << a_real;
  return stream.str();
}

// cutString("01234", 2, 4) returns "23"
inline string cutString(const string Text, const size_t Action,
                        const size_t End = string::npos)
{
  return Text.substr(Action, End - Action);
}

inline bool take(bool& do_once)
{
  if (do_once) {
    do_once = false;
    return true;
  }
  return false;
}

// handy text parsing function for escaping special chars

inline bool isEscaped(const string& Text, size_t Pos)
{
  if (Pos > 0) {
    return (Text[Pos-1] == '\\');
  } else {
    return false;
  }
}

inline bool isSpecial(const string& Text, size_t Pos, char token)
{
  if ((Pos > 0) && (Text[Pos-1] == '\\')) {
    return true;
  } else {
    return (Text[Pos] == token);
  }
}

#endif // MAIN_H_INCLUDED
