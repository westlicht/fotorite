#pragma once

#include "common.h"

FR_NAMESPACE_BEGIN

class Panel : public Widget {
public:
    Panel(Widget *parent, Color background_color = Color(0.5f)) : Widget(parent), _background_color(background_color) {}

    const Color &background_color() const { return _background_color; }
    void set_background_color(Color color) { _background_color = color; }

    virtual void draw(NVGcontext *ctx) override;

private:
    Color _background_color;
};

FR_NAMESPACE_END
