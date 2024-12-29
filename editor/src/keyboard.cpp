#include <cerrno>
#include <unistd.h>

#include "io.h"
#include "keyboard.h"

int Keyboard::readKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      IO::die("read");
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
