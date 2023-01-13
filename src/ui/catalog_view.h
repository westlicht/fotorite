#pragma once

#include "common.h"
#include "panel.h"

#include <vector>

FR_NAMESPACE_BEGIN

class CatalogViewItem;

class CatalogView : public Widget {
public:
    CatalogView(Widget *parent);

    virtual void draw(NVGcontext *ctx) override;
    virtual void perform_layout(NVGcontext *ctx) override;

private:
    VScrollPanel *m_scroll_panel;
    Panel *m_panel;
    GridLayout *m_grid_layout;

    std::vector<CatalogViewItem *> m_items;
};

FR_NAMESPACE_END
