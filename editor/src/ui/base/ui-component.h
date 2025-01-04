#pragma once

#include "../../types.h"

namespace ui::base {
class UIComponent {
public:
  UIComponent();
  UIComponent(Point m_position, Size m_size);

  inline void setX(int x) { m_position.x = x; }
  inline void setY(int y) { m_position.y = y; };
  inline void setWidth(int width) { m_size.width = width; };
  inline void setHeight(int height) { m_size.height = height; };
  inline int getX() const { return m_position.x; };
  inline int getY() const { return m_position.y; };
  inline int getWidth() const { return m_size.width; };
  inline int getHeight() const { return m_size.height; };

  virtual void draw();

protected:
  Point m_position;
  Size m_size;
};
} // namespace ui::base
