#pragma once

#define CTRL_KEY(k) ((k) & 0x1f)

namespace Keyboard {
enum Key {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  PAGE_UP,
  PAGE_DOWN,
  HOME_KEY,
  END_KEY,
  DEL_KEY,
};

int readKey();
} // namespace Keyboard
