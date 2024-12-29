#pragma once

#include <format>
#include <string>
#include <termios.h>

namespace Term {

constexpr auto TERM_DISABLE_CURSOR = "\x1b[?25l";
constexpr auto TERM_ENABLE_CURSOR = "\x1b[?25h";
constexpr auto TERM_REPORT_CURSOR_POSITION = "\x1b[6n";

inline constexpr std::string TERM_MOVE_CURSOR(int y, int x) {
  return std::string(std::format("\x1b[{:d};{:d}H", (y), (x)));
}
constexpr auto TERM_MOVE_CURSOR_TOP_LEFT = "\x1b[H";
constexpr auto TERM_MOVE_CURSOR_BOTTOM_RIGHT = "\x1b[999C\x1b[999B";
constexpr auto TERM_MOVE_CURSOR_TO_START_NEXT_LINE = "\r\n";

constexpr auto TERM_CLEAR_SCREEN = "\x1b[2J";
constexpr auto TERM_CLEAR_ROW_FROM_CURSOR_TO_END = "\x1b[K";
constexpr auto TERM_EMPTY_LINE = "~\x1b[K";

void disableRawMode();

void enableRawMode();

int getWindowSize(int *rows, int *cols);

}; // namespace Term
