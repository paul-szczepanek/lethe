#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <vector>
#include <map>
#include <cassert>

using std::cout;
using std::endl;
using std::flush;
using std::setprecision;

using std::ofstream;
using std::ifstream;
using std::stringstream;

using std::string;
using std::iterator;
using std::vector;
using std::map;

using std::min;
using std::max;
using std::abs;

typedef float real;
typedef unsigned short int usint;
typedef unsigned long int ulint;
typedef long int lint;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef const size_t csz;
typedef size_t sz;

#ifdef DEVBUILD
extern string GLog;
extern string GTrace;
extern sz GTraceIndent;
#define LOG(t) { string log = (t); \
  if (log.size() > 0) { GLog = GLog + "\n" + log; } };
#else
#define LOG(t) nop;
#endif

const string SLASH = "/";
const string DATA_DIR = "data";
const string STORY_DIR = DATA_DIR + SLASH + "books";
const string MENU_DIR = DATA_DIR + SLASH + "menu";
const string FONTS_DIR = DATA_DIR + SLASH + "fonts";
const string FRAMES_DIR = DATA_DIR + SLASH + "frames";
const string BUTTONS_DIR = DATA_DIR + SLASH + "buttons";
const string SESSION_EXT = ".session";
const string STORY_EXT = ".story";
const string STORY_FILE = "story";
const string SESSION_MAP = "session";
const string QUICK_BOOKMARK = "Quick bookmark";
const string SETTINGS_FILE = DATA_DIR + SLASH + "settings";
const char BACKSPACE_CHAR = (char)8;

const lint BLOCK_SIZE = 32;

struct Colour {
  Colour() { };
  Colour(usint _R, usint _G, usint _B, usint _A)
    : R(_R), G(_G), B(_B), A(_A) { };
  usint R = 255;
  usint G = 255;
  usint B = 255;
  usint A = 255;
};

struct string_pair {
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
  sz size();
  const char* c_str();
  string X;
  string Y;
};

struct size_t_pair {
  size_t_pair() { };
  size_t_pair(sz _X, sz _Y) : X(_X), Y(_Y) { };
  sz X = 0;
  sz Y = 0;
};
typedef size_t_pair sz_pair;
typedef const size_t_pair csz_pair;

struct real_pair {
  real_pair() { };
  real_pair(real _X, real _Y) : X(_X), Y(_Y) { };
  real X = 0;
  real Y = 0;
};

struct Rect {
  Rect() : W(0), H(0), X(0), Y(0) { };
  Rect(lint _W, lint _H, lint _X = 0, lint _Y = 0)
    : W(_W), H(_H), X(_X), Y(_Y) { };
  bool operator!= (const Rect& other) const {
    return !(*this == other);
  };
  bool operator== (const Rect& other) const {
    return W == other.W && H == other.H && X == other.X && Y == other.Y;
  };
  // align size to block size
  void Blockify() {
    W = max(W, 2*(lint)BLOCK_SIZE);
    H = max(H, 2*(lint)BLOCK_SIZE);
    lint newW = W - (W % BLOCK_SIZE);
    lint newH = H - (H % BLOCK_SIZE);
    X += (W - newW) / 2;
    Y += (H - newH) / 2;
    W = newW;
    H = newH;
  };
  lint W;
  lint H;
  lint X;
  lint Y;
};

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

template <typename T> inline sz IntoSizeT(const T& Thing)
{
  stringstream stream;
  sz integer;
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
                        csz Start,
                        csz End = string::npos)
{
  return Start < End? Text.substr(Start, End - Start) : string();
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
