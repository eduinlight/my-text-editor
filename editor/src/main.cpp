#include <csignal>

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "term/term.h"
#include "ui/ui.h"

ui::components::Editor editor;

void handleResize(int) {
  editor.refresh();
  editor.draw();
}

int main(int argc, char *argv[]) {
  term::enableRawMode();
  atexit(term::disableRawMode);

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
