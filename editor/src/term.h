#include <format>
#include <string>

struct Term {
  static constexpr auto TERM_DISABLE_CURSOR = "\x1b[?25l";
  static constexpr auto TERM_ENABLE_CURSOR = "\x1b[?25h";
  static constexpr auto TERM_REPORT_CURSOR_POSITION = "\x1b[6n";

  static inline constexpr std::string TERM_MOVE_CURSOR(int y, int x) {
    return std::string(std::format("\x1b[{:d};{:d}H", (y), (x)));
  }
  static constexpr auto TERM_MOVE_CURSOR_TOP_LEFT = "\x1b[H";
  static constexpr auto TERM_MOVE_CURSOR_BOTTOM_RIGHT = "\x1b[999C\x1b[999B";
  static constexpr auto TERM_MOVE_CURSOR_TO_START_NEXT_LINE = "\r\n";

  static constexpr auto TERM_CLEAR_SCREEN = "\x1b[2J";
  static constexpr auto TERM_CLEAR_ROW_FROM_CURSOR_TO_END = "\x1b[K";
  static constexpr auto TERM_EMPTY_LINE = "~\x1b[K";
};
