#pragma once

#include "common.h"

FR_NAMESPACE_BEGIN

class Panel : public Widget {
public:
    Panel(Widget *parent, Color background_color = Color(0.5f)) : Widget(parent), m_background_color(background_color)
    {
    }

    const Color &background_color() const { return m_background_color; }
    void set_background_color(Color color) { m_background_color = color; }

    virtual void draw(NVGcontext *ctx) override;

private:
    Color m_background_color;
};

FR_NAMESPACE_END
