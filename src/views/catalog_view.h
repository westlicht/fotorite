#pragma once

#include "core/defs.h"

#include <nanogui/nanogui.h>

namespace ng = nanogui;

FR_NAMESPACE_BEGIN

class CatalogView : public ng::Widget {
public:
    CatalogView(Widget *parent) : ng::Widget(parent) {}

};

FR_NAMESPACE_END
