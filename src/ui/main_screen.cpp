#include "main_screen.h"

#include "catalog_view.h"

#include <spdlog/spdlog.h>

FR_NAMESPACE_BEGIN

MainScreen::MainScreen() : nanogui::Screen(Vector2i(500, 700), "fotorite") {
    _left_panel = add<Panel>();
    _left_panel->set_fixed_width(200);

    _right_panel = add<Panel>();
    _right_panel->set_fixed_width(100);

    _top_panel = add<Panel>();
    _top_panel->set_fixed_height(50);

    _bottom_panel = add<Panel>();
    _bottom_panel->set_fixed_height(30);

    _main_panel = add<Panel>(Color(0.1f, 1.f));

    auto label = _top_panel->add<Label>("FOTORITE");
    label->set_position(Vector2i(10, 10));
    label->set_font("sans-bold");
    label->set_font_size(30);

    _catalog_view = _main_panel->add<CatalogView>();

    set_resize_callback([this](Vector2i) { update_layout(); });

    update_layout();
}

bool MainScreen::keyboard_event(int key, int scancode, int action, int modifiers) {
    ignore_unused(scancode, modifiers);
    spdlog::info("key {} action {}", key, action);
    return true;
}

void MainScreen::update_layout() {
    int screen_width = this->width();
    int screen_height = this->height();

    int left_width = _left_panel->fixed_width();
    int right_width = _right_panel->fixed_width();
    int top_height = _top_panel->fixed_height();
    int bottom_height = _bottom_panel->fixed_height();

    _left_panel->set_position(Vector2i(0, top_height));
    _left_panel->set_fixed_height(screen_height - top_height - bottom_height);

    _right_panel->set_position(Vector2i(screen_width - right_width, top_height));
    _right_panel->set_fixed_height(screen_height - top_height - bottom_height);

    _top_panel->set_position(Vector2i(0, 0));
    _top_panel->set_fixed_width(screen_width);

    _bottom_panel->set_position(Vector2i(0, screen_height - bottom_height));
    _bottom_panel->set_fixed_width(screen_width);

    _main_panel->set_position(Vector2i(left_width, top_height));
    _main_panel->set_fixed_width(screen_width - left_width - right_width);
    _main_panel->set_fixed_height(screen_height - top_height - bottom_height);

    if (auto child = _main_panel->child_at(0)) {
        child->set_fixed_size(_main_panel->fixed_size());
    }

    this->perform_layout();

    // _left_panel->perform_layout();
}

FR_NAMESPACE_END
