#pragma once

#include "common.h"
#include "panel.h"
#include "catalog_view.h"

FR_NAMESPACE_BEGIN

class MainScreen : public nanogui::Screen {
public:
    MainScreen();

    virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;

private:
    void update_layout();

    Panel *_left_panel;
    Panel *_right_panel;
    Panel *_top_panel;
    Panel *_bottom_panel;

    CatalogView *_catalog_view;
};

FR_NAMESPACE_END
