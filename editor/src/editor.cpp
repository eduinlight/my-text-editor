#include <csignal>
#include <format>
#include <iostream>

#include "editor.h"
#include "io.h"
#include "keyboard.h"
#include "macros.h"
#include "term.h"

Editor::Editor()
    : m_cursor(Point(0, 0)), m_offset(Point(0, 0)), m_currentBuffer("") {
  refresh();
}

void Editor::openFile(const std::string &file) {
  m_rows = IO::readFileToVector(file);
  refresh();
}

void Editor::refresh() {
  if (Term::getWindowSize(&m_screenRows, &m_screenCols) == -1)
    IO::die("getWindowSize");

  int n = SZ(m_rows);
  m_lineNumberColumnSize = 0;
  while (n) {
    n /= 10;
    m_lineNumberColumnSize++;
  }
  m_lineLeftPadding = 1;
  m_lineStatusSize = 2;
  m_rowStartSize =
      m_lineStatusSize + m_lineNumberColumnSize + m_lineLeftPadding;
  m_editableScreenCols = m_screenCols - m_rowStartSize;
}

void Editor::drawRows() {
  std::string leftPadding = std::string(m_lineLeftPadding, ' ');

  for (int y = 0; y < m_screenRows; y++) {
    int fileY = m_offset.y + y;
    if (fileY < SZ(m_rows)) {
      if (y > 0)
        m_currentBuffer.append(Term::TERM_MOVE_CURSOR_TO_START_NEXT_LINE);

      std::string lineStatus = std::string(m_lineStatusSize, ' ');
      std::string lineNumber = std::format("{:d}", m_offset.y + y + 1);
      std::string lineNumberLeftPad =
          std::string(m_lineNumberColumnSize - SZ(lineNumber), ' ');
      std::string rowStart =
          lineStatus + lineNumberLeftPad + lineNumber + leftPadding;
      m_currentBuffer.append(rowStart);
      auto row = m_rows[fileY];
      std::string visibleRow;
      if (!row.empty() && SZ(row) >= m_offset.x)
        visibleRow = row.substr(m_offset.x, m_editableScreenCols);
      if (!visibleRow.empty())
        m_currentBuffer.append(visibleRow);
      if (SZ(visibleRow) < m_editableScreenCols)
        m_currentBuffer.append(Term::TERM_CLEAR_ROW_FROM_CURSOR_TO_END);
    } else {
      // if (y == SZ(m_rows))
      // m_currentBuffer.append(Term::TERM_DISABLE_MOUSE);
      if (y > 0)
        m_currentBuffer.append(Term::TERM_MOVE_CURSOR_TO_START_NEXT_LINE);
      m_currentBuffer.append(Term::TERM_EMPTY_LINE);
    }
  }
}

void Editor::updateScroll() {
  if (m_cursor.y < m_offset.y)
    m_offset.y = m_cursor.y;
  else if (m_cursor.y >= m_offset.y + m_screenRows)
    m_offset.y = m_cursor.y - m_screenRows + 1;

  if (m_cursor.x < m_offset.x)
    m_offset.x = m_cursor.x;
  else if (m_cursor.x >= m_offset.x + m_editableScreenCols)
    m_offset.x = m_cursor.x - m_editableScreenCols + 1;
}

void Editor::draw() {
  updateScroll();

  m_currentBuffer.clear();

  m_currentBuffer.append(Term::TERM_DISABLE_CURSOR);
  m_currentBuffer.append(Term::TERM_MOVE_CURSOR_TOP_LEFT);

  drawRows();

  m_currentBuffer.append(
      Term::TERM_MOVE_CURSOR((m_cursor.y - m_offset.y) + 1,
                             m_rowStartSize + (m_cursor.x - m_offset.x) + 1));
  m_currentBuffer.append(Term::TERM_ENABLE_CURSOR);

  std::cout << m_currentBuffer;
  std::cout.flush();
}

void Editor::fixXCursor() {
  if (!m_rows.empty() && m_cursor.x >= SZ(m_rows[m_cursor.y]))
    m_cursor.x = std::max(SZ(m_rows[m_cursor.y]) - 1, 0);
}

void Editor::processKeypress() {
  int c = Keyboard::readKey();
  switch (c) {
  case CTRL_KEY('q'):
    std::cout << Term::TERM_CLEAR_SCREEN;
    std::cout << Term::TERM_MOVE_CURSOR_TOP_LEFT;
    exit(0);
    break;
  case CTRL_KEY('u'): {
    int steps = m_screenRows / 2;
    if (m_offset.y == 0)
      m_cursor.y = std::max(m_cursor.y - steps, 0);
    else {
      int screenPos = m_cursor.y - m_offset.y;
      m_offset.y = std::max(m_offset.y - steps, 0);
      m_cursor.y = std::min(m_offset.y + screenPos, SZ(m_rows) - 1);
    }
    fixXCursor();
    break;
  }
  case CTRL_KEY('d'): {
    int cursorEnd = SZ(m_rows) - 1;
    bool isLastPage = m_offset.y + m_screenRows > cursorEnd;
    int steps = m_screenRows / 2;
    int lastPageStart = cursorEnd - m_screenRows + 1;
    if (isLastPage) {
      m_cursor.y = std::min(m_cursor.y + steps, cursorEnd);
    } else {
      int screenPos = m_cursor.y - m_offset.y;
      m_offset.y = std::min(m_offset.y + steps, cursorEnd);
      m_cursor.y = std::min(m_offset.y + screenPos, cursorEnd);
      if (m_offset.y > lastPageStart)
        m_offset.y = lastPageStart;
    }
    fixXCursor();
    break;
  }
  case 'h':
  case Keyboard::Key::ARROW_LEFT:
    if (m_cursor.x > 0)
      m_cursor.x--;
    break;
  case 'l':
  case Keyboard::Key::ARROW_RIGHT:
    if (!m_rows.empty() && !m_rows[m_cursor.y].empty() &&
        m_cursor.x < SZ(m_rows[m_cursor.y]) - 1)
      m_cursor.x++;
    break;
  case 'k':
  case Keyboard::Key::ARROW_UP:
    if (m_cursor.y > 0) {
      m_cursor.y--;
      fixXCursor();
    }
    break;
  case 'j':
  case Keyboard::Key::ARROW_DOWN:
    if (m_cursor.y < SZ(m_rows) - 1) {
      m_cursor.y++;
      fixXCursor();
    }
    break;
  case Keyboard::Key::PAGE_UP:
    if (m_offset.y > 0) {
      m_offset.y = std::max(m_offset.y - m_screenRows - 2, 0);
      m_cursor.y = std::min(m_offset.y + m_screenRows - 1, SZ(m_rows) - 1);
      fixXCursor();
    }
    break;
  case Keyboard::Key::PAGE_DOWN:
    if (m_offset.y < SZ(m_rows) - 1) {
      m_offset.y = std::min(m_offset.y + m_screenRows - 2, SZ(m_rows) - 1);
      m_cursor.y = m_offset.y;
      fixXCursor();
    }
    break;
  case Keyboard::Key::HOME_KEY:
    m_cursor.x = 0;
    break;
  case Keyboard::Key::END_KEY:
    if (!m_rows.empty() && !m_rows[m_cursor.y].empty()) {
      m_cursor.x = SZ(m_rows[m_cursor.y]) - 1;
    }
    break;
  }
}
