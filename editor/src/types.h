#pragma once

#include <string>

typedef std::basic_string<char> Buffer;

struct Point {
  int x;
  int y;

  Point(int _x, int _y) : x(_x), y(_y) {}
};

struct Size {
  int width;
  int height;

  Size(int _width, int _height) : width(_width), height(_height) {}
};

struct RGBColor {
  int red;
  int green;
  int blue;

  RGBColor(int red, int green, int blue) : red(red), green(green), blue(blue) {}
};
