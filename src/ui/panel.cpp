#include "panel.h"

#include <nanogui/opengl.h>

FR_NAMESPACE_BEGIN

void Panel::draw(NVGcontext *ctx) {
    Widget::draw(ctx);

    nvgStrokeWidth(ctx, 5.0f);
    nvgBeginPath(ctx);
    nvgRect(ctx, m_pos.x() - 0.5f, m_pos.y() - 0.5f, m_size.x() + 1, m_size.y() + 1);
    nvgStrokeColor(ctx, nvgRGBA(255, 0, 0, 255));
    nvgStroke(ctx);
}

FR_NAMESPACE_END
