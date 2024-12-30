#pragma once

#include <string>
#include <vector>

struct Point {
  int x;
  int y;

  Point(int _x, int _y) : x(_x), y(_y) {}
};

typedef std::basic_string<char> buffer;

struct Editor {
private:
  Point m_cursor;
  Point m_offset;
  int m_screenRows;
  int m_screenCols;
  std::vector<std::string> m_rows;
  buffer m_currentBuffer;
  int m_lineLeftPadding;
  int m_lineStatusSize;
  int m_lineNumberColumnSize;
  int m_rowStartSize;
  int m_editableScreenCols;

  void refreshLineParams();

public:
  Editor();
  void editorOpen(const std::string &file);
  void editorDrawRows();
  void updateEditorScroll();
  void editorRefreshScreen();
  void editorProcessKeypress();
};
