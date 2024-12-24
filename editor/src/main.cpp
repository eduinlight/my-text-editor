/*** includes ***/

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
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
};

struct ScreenPosition {
  int x;
  int y;

  ScreenPosition(const int &_x, const int &&_y) : x(_x), y(_y) {}
  ScreenPosition() {}
};

struct EditorConfig {
  ScreenPosition cursor;
  int screenrows;
  int screencols;
  struct termios orig_termios;

  EditorConfig() {}
};

/*** data ***/

EditorConfig E;

std::basic_string<char> current_buffer;

/*** terminal ***/

void die(const char *s) {
  std::cout << "\x1b[2J";
  std::cout << "\x1b[H";

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

  if (std::cout.write("\x1b[6n", 4).fail())
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
    if (std::cout.write("\x1b[999C\x1b[999B", 12).fail())
      return -1;
    return getCursorPositionFallback(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

/*** output ***/

void editorDrawRows() {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    current_buffer.append("~");
    current_buffer.append("\x1b[K");
    if (y < E.screenrows - 1)
      current_buffer.append("\r\n");
  }
}

void editorRefreshScreen() {
  current_buffer.clear();

  current_buffer.append("\x1b[?25l");
  current_buffer.append("\x1b[H");

  editorDrawRows();

  current_buffer.append(
      std::format("\x1b[{:d};{:d}H", E.cursor.y + 1, E.cursor.x + 1));
  current_buffer.append("\x1b[?25h");

  std::cout << current_buffer;
  std::cout.flush();
}

/*** input ***/

/*** init ***/

void initEditor() {
  E.cursor.x = 0;
  E.cursor.y = 0;

  if (getWindowSize(&E.screenrows, &E.screencols) == -1)
    die("getWindowSize");
}

void moveCursor(int c) {
  switch (c) {
  case 'h':
  case Key::ARROW_LEFT:
    if (E.cursor.x > 0)
      E.cursor.x--;
    break;
  case 'l':
  case Key::ARROW_RIGHT:
    if (E.cursor.x < E.screencols - 1)
      E.cursor.x++;
    break;
  case 'k':
  case Key::ARROW_UP:
    if (E.cursor.y > 0)
      E.cursor.y--;
    break;
  case 'j':
  case Key::ARROW_DOWN:
    if (E.cursor.y < E.screenrows - 1)
      E.cursor.y++;
    break;
  case Key::PAGE_UP:
  case Key::PAGE_DOWN: {
    int times = E.screenrows;
    while (times--)
      moveCursor(c == Key::PAGE_UP ? Key::ARROW_UP : Key::ARROW_DOWN);
  } break;
  case Key::HOME_KEY:
    E.cursor.x = 0;
    break;
  case Key::END_KEY:
    E.cursor.x = E.screencols - 1;
    break;
  }
}

void editorProcessKeypress() {
  int c = editorReadKey();
  switch (c) {
  case CTRL_KEY('q'):
    std::cout << "\x1b[2J";
    std::cout << "\x1b[H";
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

int main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  enableRawMode();
  initEditor();

  while (true) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return EXIT_SUCCESS;
}
