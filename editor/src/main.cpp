#include <csignal>

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "editor.h"
#include "term.h"

Editor editor;

void handleResize(int) {
  editor.refresh();
  editor.draw();
}

int main(int argc, char *argv[]) {
  Term::enableRawMode();
  atexit(Term::disableRawMode);

  signal(SIGWINCH, handleResize);
  std::signal(SIGTTOU, SIG_IGN);

  if (argc >= 2) {
    editor.openFile(std::string(argv[1]));
  }

  while (true) {
    editor.draw();
    editor.processKeypress();
  }

  return EXIT_SUCCESS;
}
