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
  if (log.size() > 0) { GLog = GLog + "\n- " + log; } };
#define RLOG(t) { GLog = (t); };
#else
#define LOG(t);
#define RLOG(t);
#endif

const string STORY_DIR = "data/books/";
const string KEYWORDS_DIR = "data/cache/keywords/";

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
  Rect(uint _W, uint _H, uint _X = 0, uint _Y = 0)
    : W(_W), H(_H), X(_X), Y(_Y) { };
  uint W;
  uint H;
  uint X;
  uint Y;
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
inline string cutString(const string Text, const size_t Start,
                        const size_t End = string::npos)
{
  return Text.substr(Start, End - Start);
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
