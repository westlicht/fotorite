#pragma once

#include "common.h"
#include "panel.h"

#include <vector>

FR_NAMESPACE_BEGIN

class CatalogViewItem;

class CatalogView : public Widget {
public:
    CatalogView(Widget *parent);

    virtual void perform_layout(NVGcontext *ctx) override;

private:
    VScrollPanel *_scroll_panel;
    Panel *_panel;
    GridLayout *_grid_layout;

    std::vector<CatalogViewItem *> _items;
};

FR_NAMESPACE_END
