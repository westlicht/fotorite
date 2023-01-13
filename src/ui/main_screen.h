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

    Panel *m_left_panel;
    Panel *m_right_panel;
    Panel *m_top_panel;
    Panel *m_bottom_panel;
    Panel *m_main_panel;

    CatalogView *m_catalog_view;
};

FR_NAMESPACE_END
