/*** includes ***/

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/ioctl.h>
#include <termio.h>
#include <unistd.h>
#include <vector>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)
#define SZ(x) ((int)(x.size()))

#define TERM_DISABLE_CURSOR "\x1b[?25l"
#define TERM_ENABLE_CURSOR "\x1b[?25h"
#define TERM_MOVE_CURSOR_TOP_LEFT "\x1b[H"
#define TERM_MOVE_CURSOR_BOTTOM_RIGHT "\x1b[999C\x1b[999B"
#define TERM_CLEAR_SCREEN "\x1b[2J"
#define TERM_REPORT_CURSOR_POSITION "\x1b[6n"
#define TERM_MOVE_CURSOR_TO_START_NEXT_LINE "\r\n"
#define TERM_CLEAR_FROM_COL_TO_END "\x1b[K"

typedef std::vector<char> buffer_t;

enum Key {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  PAGE_UP,
  PAGE_DOWN,
  HOME_KEY,
  END_KEY,
  DEL_KEY,
};

struct ScreenPosition {
  int x;
  int y;

  ScreenPosition(const int &_x, const int &&_y) : x(_x), y(_y) {}
  ScreenPosition() {}
};

struct EditorConfig {
  ScreenPosition cursor;
  int offsetY;
  int offsetX;
  int screenRows;
  int screenCols;
  struct termios orig_termios;
  std::vector<std::string> rows;

  EditorConfig() {}
};

/*** data ***/

EditorConfig E;

std::basic_string<char> currentBuffer;

/*** terminal ***/

void die(const char *s) {
  std::cout << TERM_CLEAR_SCREEN;
  std::cout << TERM_MOVE_CURSOR_TOP_LEFT;

  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    die("tcgetattr");

  atexit(disableRawMode);

  struct termios raw = E.orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

int editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }

  if (c == '\x1b') {
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1)
      return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
      return '\x1b';

    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1)
          return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
          case '1':
            return Key::HOME_KEY;
          case '3':
            return DEL_KEY;
          case '4':
            return Key::END_KEY;
          case '5':
            return Key::PAGE_UP;
          case '6':
            return Key::PAGE_DOWN;
          case '7':
            return Key::HOME_KEY;
          case '8':
            return Key::END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
        case 'A':
          return Key::ARROW_UP;
        case 'B':
          return Key::ARROW_DOWN;
        case 'C':
          return Key::ARROW_RIGHT;
        case 'D':
          return Key::ARROW_LEFT;
        case 'H':
          return Key::HOME_KEY;
        case 'F':
          return Key::END_KEY;
        }
      }
    } else if (seq[0] == 'O') {
      switch (seq[1]) {
      case 'H':
        return HOME_KEY;
      case 'F':
        return END_KEY;
      }
    }
    return '\x1b';
  }
  return c;
}

int getCursorPositionFallback(int *rows, int *cols) {
  char buf[32];
  size_t i = 0;

  if (std::cout.write(TERM_REPORT_CURSOR_POSITION, 4).fail())
    return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }
  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[')
    return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
    return -1;
  return 0;
}

int getWindowSize(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (std::cout.write(TERM_MOVE_CURSOR_BOTTOM_RIGHT, 12).fail())
      return -1;
    return getCursorPositionFallback(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/*** file i/o ***/

std::vector<std::string> readFileToVector(const std::string &filePath) {
  std::vector<std::string> lines;
  std::ifstream fd(filePath);

  if (!fd.is_open()) {
    die("ifstream");
  }

  std::string line;
  while (std::getline(fd, line)) {
    lines.push_back(line);
  }

  fd.close();
  return lines;
}

void editorOpen(const std::string &file) { E.rows = readFileToVector(file); }

/*** output ***/

void editorDrawRows() {
  int y;
  for (y = E.offsetY; y < E.offsetY + E.screenRows; y++) {
    if (y >= SZ(E.rows)) {
      if (y > 0)
        currentBuffer.append(TERM_MOVE_CURSOR_TO_START_NEXT_LINE);
      currentBuffer.append("~");
      currentBuffer.append(TERM_CLEAR_FROM_COL_TO_END);
    } else {
      auto row = E.rows[y].substr(0, E.screenCols);
      currentBuffer.append(row);
      if (SZ(row) < E.screenCols)
        currentBuffer.append(TERM_CLEAR_FROM_COL_TO_END);
      if (y < SZ(E.rows) - 1)
        currentBuffer.append(TERM_MOVE_CURSOR_TO_START_NEXT_LINE);
    }
  }
}

void editorRefreshScreen() {
  currentBuffer.clear();

  currentBuffer.append(TERM_DISABLE_CURSOR);
  currentBuffer.append(TERM_MOVE_CURSOR_TOP_LEFT);

  editorDrawRows();

  currentBuffer.append(
      std::format("\x1b[{:d};{:d}H", E.cursor.y + 1, E.cursor.x + 1));
  currentBuffer.append(TERM_ENABLE_CURSOR);

  std::cout << currentBuffer;
  std::cout.flush();
}

/*** input ***/

/*** init ***/

void initEditor() {
  E.cursor.x = 0;
  E.cursor.y = 0;
  E.offsetX = 0;
  E.offsetY = 0;

  if (getWindowSize(&E.screenRows, &E.screenCols) == -1)
    die("getWindowSize");
}

void moveCursor(int c) {
  switch (c) {
  case 'h':
  case Key::ARROW_LEFT:
    E.cursor.x--;
    break;
  case 'l':
  case Key::ARROW_RIGHT:
    E.cursor.x++;
    break;
  case 'k':
  case Key::ARROW_UP:
    E.cursor.y--;
    break;
  case 'j':
  case Key::ARROW_DOWN:
    E.cursor.y++;
    break;
  case Key::PAGE_UP:
  case Key::PAGE_DOWN: {
    int times = E.screenRows;
    while (times--)
      moveCursor(c == Key::PAGE_UP ? Key::ARROW_UP : Key::ARROW_DOWN);
  } break;
  case Key::HOME_KEY:
    E.cursor.x = 0;
    break;
  case Key::END_KEY:
    E.cursor.x = E.screenCols - 1;
    break;
  }
}

void editorProcessKeypress() {
  int c = editorReadKey();
  switch (c) {
  case CTRL_KEY('q'):
    std::cout << TERM_CLEAR_SCREEN;
    std::cout << TERM_MOVE_CURSOR_TOP_LEFT;
    exit(0);
    break;
  case Key::ARROW_UP:
  case Key::ARROW_DOWN:
  case Key::ARROW_LEFT:
  case Key::ARROW_RIGHT:
  case Key::PAGE_UP:
  case Key::PAGE_DOWN:
  case Key::END_KEY:
  case Key::HOME_KEY:
  case 'h':
  case 'l':
  case 'j':
  case 'k':
    moveCursor(c);
    break;
  }
}

void updateEditorScroll() {
  E.offsetY = std::max(E.cursor.y - E.screenRows, 0);

  // fix cursot y position
  if (E.rows.empty())
    E.cursor.y = 0;
  else if (E.cursor.y >= SZ(E.rows))
    E.cursor.y = SZ(E.rows) - 1;
  else if (E.cursor.y < 0)
    E.cursor.y = 0;

  // fix cursot x position
  if (E.rows.empty() || E.rows[E.cursor.y].empty())
    E.cursor.x = 0;
  else if (E.cursor.x < 0)
    E.cursor.x = 0;
  else if (E.cursor.x >= SZ(E.rows[E.cursor.y]))
    E.cursor.x = SZ(E.rows[E.cursor.y]) - 1;
}

int main(int argc, char *argv[]) {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  enableRawMode();
  initEditor();
  if (argc >= 2) {
    editorOpen(std::string(argv[1]));
  }

  while (true) {
    editorRefreshScreen();
    editorProcessKeypress();
    updateEditorScroll();
  }

  return EXIT_SUCCESS;
}
