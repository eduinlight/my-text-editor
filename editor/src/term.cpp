#include "term.h"
#include "io.h"
#include <ios>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>

namespace Term {

termios orig_termios;

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &Term::orig_termios) == -1)
    IO::die("tcsetattr");
}

void enableRawMode() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);

  if (tcgetattr(STDIN_FILENO, &Term::orig_termios) == -1)
    IO::die("tcgetattr");

  struct termios raw = Term::orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    IO::die("tcsetattr");
}

int getWindowSizeFallback(int *rows, int *cols) {
  if (std::cout.write(Term::TERM_MOVE_CURSOR_BOTTOM_RIGHT, 12).fail())
    return -1;

  char buf[32];
  size_t i = 0;

  if (std::cout.write(Term::TERM_REPORT_CURSOR_POSITION, 4).fail())
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
    return getWindowSizeFallback(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

} // namespace Term
