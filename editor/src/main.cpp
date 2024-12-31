#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include "editor.h"
#include "term.h"

Editor editor;

void handleResize(int) { editor.refresh(); }

int main(int argc, char *argv[]) {
  Term::enableRawMode();
  atexit(Term::disableRawMode);

  signal(SIGWINCH, handleResize);
  std::signal(SIGTTOU, SIG_IGN);

  std::atomic<bool> keepMonitoring = true;
  std::thread monitorThread = std::thread([&]() {
    while (keepMonitoring) {
      editor.refresh();
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  });

  if (argc >= 2) {
    editor.editorOpen(std::string(argv[1]));
  }

  while (true) {
    editor.editorRefreshScreen();
    editor.editorProcessKeypress();
  }

  keepMonitoring = false;
  if (monitorThread.joinable())
    monitorThread.join();

  return EXIT_SUCCESS;
}
