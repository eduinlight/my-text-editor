#include "io.h"
#include "term.h"
#include <fstream>
#include <iostream>

void IO::die(const char *s) {
  std::cout << Term::TERM_CLEAR_SCREEN;
  std::cout << Term::TERM_MOVE_CURSOR_TOP_LEFT;

  perror(s);
  exit(1);
}

std::vector<std::string> IO::readFileToVector(const std::string &filePath) {
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
