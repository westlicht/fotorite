#include "main_screen.h"

#include "catalog_view.h"

#include <spdlog/spdlog.h>

FR_NAMESPACE_BEGIN

MainScreen::MainScreen() : nanogui::Screen(Vector2i(500, 700), "fotorite")
{
    m_left_panel = add<Panel>();
    m_left_panel->set_fixed_width(200);

    m_right_panel = add<Panel>();
    m_right_panel->set_fixed_width(100);

    m_top_panel = add<Panel>();
    m_top_panel->set_fixed_height(50);

    m_bottom_panel = add<Panel>();
    m_bottom_panel->set_fixed_height(30);

    m_main_panel = add<Panel>(Color(0.1f, 1.f));

    auto label = m_top_panel->add<Label>("FOTORITE");
    label->set_position(Vector2i(10, 10));
    label->set_font("sans-bold");
    label->set_font_size(30);

    m_catalog_view = m_main_panel->add<CatalogView>();

    set_resize_callback([this](Vector2i) { update_layout(); });

    update_layout();
}

bool MainScreen::keyboard_event(int key, int scancode, int action, int modifiers)
{
    ignore_unused(scancode, modifiers);
    spdlog::info("key {} action {}", key, action);
    return true;
}

void MainScreen::update_layout()
{
    int screen_width = this->width();
    int screen_height = this->height();

    int left_width = m_left_panel->fixed_width();
    int right_width = m_right_panel->fixed_width();
    int top_height = m_top_panel->fixed_height();
    int bottom_height = m_bottom_panel->fixed_height();

    m_left_panel->set_position(Vector2i(0, top_height));
    m_left_panel->set_fixed_height(screen_height - top_height - bottom_height);

    m_right_panel->set_position(Vector2i(screen_width - right_width, top_height));
    m_right_panel->set_fixed_height(screen_height - top_height - bottom_height);

    m_top_panel->set_position(Vector2i(0, 0));
    m_top_panel->set_fixed_width(screen_width);

    m_bottom_panel->set_position(Vector2i(0, screen_height - bottom_height));
    m_bottom_panel->set_fixed_width(screen_width);

    m_main_panel->set_position(Vector2i(left_width, top_height));
    m_main_panel->set_fixed_width(screen_width - left_width - right_width);
    m_main_panel->set_fixed_height(screen_height - top_height - bottom_height);

    if (auto child = m_main_panel->child_at(0)) {
        child->set_fixed_size(m_main_panel->fixed_size());
    }

    this->perform_layout();

    // m_left_panel->perform_layout();
}

FR_NAMESPACE_END
