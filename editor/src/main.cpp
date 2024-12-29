/*** includes ***/

#include "keyboard.h"
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "editor.h"
#include "term.h"

int main(int argc, char *argv[]) {
  Term::enableRawMode();
  atexit(Term::disableRawMode);

  Editor editor;

  if (argc >= 2) {
    editor.editorOpen(std::string(argv[1]));
  }

  while (true) {
    editor.editorRefreshScreen();
    editor.editorProcessKeypress();
  }

  return EXIT_SUCCESS;
}
