#pragma once

#include "core/defs.h"
#include "views/catalog_view.h"

#include <nanogui/nanogui.h>

namespace ng = nanogui;

FR_NAMESPACE_BEGIN

class MainScreen : public ng::Screen {
public:
    MainScreen();

    virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;

private:
    CatalogView *_catalog_view;

};

FR_NAMESPACE_END
