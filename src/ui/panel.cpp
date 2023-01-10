#include "panel.h"

#include <nanogui/opengl.h>

FR_NAMESPACE_BEGIN

void Panel::draw(NVGcontext *ctx) {
    nvgBeginPath(ctx);
    nvgRect(ctx, m_pos.x() - 0.5f, m_pos.y() - 0.5f, m_size.x() + 1, m_size.y() + 1);

    nvgFillColor(ctx, _background_color);
    nvgFill(ctx);

    nvgStrokeWidth(ctx, 2.0f);
    // nvgStrokeColor(ctx, _background_color.contrasting_color());
    nvgStrokeColor(ctx, Color(1.f));
    nvgStroke(ctx);

    Widget::draw(ctx);
}

FR_NAMESPACE_END
