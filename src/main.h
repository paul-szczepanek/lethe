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

typedef float real;
typedef unsigned short int usint;
typedef unsigned long int ulint;
typedef long int lint;
typedef unsigned int uint;
typedef unsigned char uchar;
typedef const size_t cszt;
typedef size_t szt;

#ifdef DEVBUILD
extern string GLog;
extern string GTrace;
extern szt GTraceIndent;
#define LOG(t) { string log = (t); \
  if (log.size() > 0) { GLog = GLog + "\n" + log; cout << log << endl; } };
#else
#define LOG(t);
#endif

#if defined(__ANDROID__) && ! defined (FAKEANDROID)
const string DATA_DIR = "/sdcard/lethe/data";
#else
const string DATA_DIR = "data";
#endif

const string SLASH = "/";
const string STORY_DIR = DATA_DIR + SLASH + "books";
const string MENU_DIR = DATA_DIR + SLASH + "menu";
const string FONTS_DIR = DATA_DIR + SLASH + "fonts";
const string FRAMES_DIR = DATA_DIR + SLASH + "frames";
const string BUTTONS_DIR = DATA_DIR + SLASH + "buttons";
const string SESSION_EXT = ".session";
const string STORY_EXT = ".story";
const string STORY_FILE = "story";
const string SESSION_MAP = "session";
const string SETTINGS_FILE = DATA_DIR + SLASH + "settings";
const char BACKSPACE_CHAR = (char)8;

struct Colour {
  Colour() { };
  Colour(usint aR, usint aG, usint aB, usint aA)
    : R(aR), G(aG), B(aB), A(aA) { };
  usint R = 255;
  usint G = 255;
  usint B = 255;
  usint A = 255;
};

struct string_pair {
  string_pair(string aX, string aY) : X(aX), Y(aY) { };
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
  szt size();
  const char* c_str();
  string X;
  string Y;
};

struct int_pair {
  int_pair() : X(0), Y(0) { };
  int_pair(int aX, int aY) : X(aX), Y(aY) { };
  int X;
  int Y;
};

struct szt_pair {
  szt_pair() : X(0), Y(0) { };
  szt_pair(szt aX, szt aY) : X(aX), Y(aY) { };
  szt X;
  szt Y;
};

typedef const szt_pair cszt_pair;

struct real_pair {
  real_pair() { };
  real_pair(real aX, real aY) : X(aX), Y(aY) { };
  real X = 0;
  real Y = 0;
};

struct Rect {
  Rect() : W(0), H(0), X(0), Y(0) { };
  Rect(lint aW, lint aH, lint aX = 0, lint aY = 0)
    : W(aW), H(aH), X(aX), Y(aY) { };
  bool operator!= (const Rect& other) const {
    return !(*this == other);
  };
  bool operator== (const Rect& other) const {
    return W == other.W && H == other.H && X == other.X && Y == other.Y;
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
  real result;
  stream << Thing;
  stream >> result;
  return result;
}

template <typename T> inline int IntoInt(const T& Thing)
{
  stringstream stream;
  int integer;
  stream << Thing;
  stream >> integer;
  return integer;
}

template <> inline int IntoInt(const string& Thing)
{
  return std::stoi(Thing);
}

template <typename T> inline szt IntoSizeT(const T& Thing)
{
  stringstream stream;
  szt integer;
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
                        cszt Start,
                        cszt End = string::npos)
{
  return Start < End? Text.substr(Start, End - Start) : string();
}

template <typename T> inline void Clamp(T& Value,
                                        const T& Min,
                                        const T& Max)
{
  Value = Value < Min? Min : (Value > Max? Max : Value);
}

// android
template <typename T> inline T Abs(const T& Value)
{
  return Value < 0? -Value : Value;
}

#endif // MAIN_H_INCLUDED
