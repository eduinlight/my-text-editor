#include "ui-component.h"

namespace ui::base {
UIComponent::UIComponent(Point m_position, Size m_size)
    : m_position(m_position), m_size(m_size) {}
} // namespace ui::base
