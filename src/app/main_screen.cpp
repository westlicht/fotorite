#include "main_screen.h"

#include "views/catalog_view.h"

#include <spdlog/spdlog.h>

FR_NAMESPACE_BEGIN

MainScreen::MainScreen() : ng::Screen(ng::Vector2i(500, 700), "fotorite") {
    _catalog_view = add<CatalogView>();
    auto label = add<ng::Label>("FOTORITE");
    label->set_position(ng::Vector2i(10, 10));
    label->set_font_size(30);
}

bool MainScreen::keyboard_event(int key, int scancode, int action, int modifiers) {
    ignore_unused(scancode, modifiers);
    spdlog::info("key {} action {}", key, action);
    return true;
}

FR_NAMESPACE_END
