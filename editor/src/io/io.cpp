#include "io.h"
#include "../term/term.h"
#include <fstream>
#include <iostream>

namespace io {

void die(const char *s) {
  std::cout << term::TERM_CLEAR_SCREEN;
  std::cout << term::TERM_MOVE_CURSOR_TOP_LEFT;

  perror(s);
  exit(1);
}

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
} // namespace io
