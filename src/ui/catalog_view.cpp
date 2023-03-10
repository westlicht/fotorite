#include "catalog_view.h"

#include <nanogui/opengl.h>

FR_NAMESPACE_BEGIN

class CatalogViewItem : public Widget {
public:
    CatalogViewItem(Widget *parent, Color background_color = Color(0.5f))
        : Widget(parent), m_background_color(background_color), m_skip_drawing(false)
    {
    }

    const Color &background_color() const { return m_background_color; }
    void set_background_color(Color color) { m_background_color = color; }

    bool skip_drawing() const { return m_skip_drawing; }
    void set_skip_drawing(bool skip_drawing) { m_skip_drawing = skip_drawing; }

    virtual void draw(NVGcontext *ctx) override
    {
        if (m_skip_drawing)
            return;

        nvgBeginPath(ctx);
        nvgRect(ctx, m_pos.x() - 0.5f, m_pos.y() - 0.5f, m_size.x() + 1, m_size.y() + 1);

        nvgFillColor(ctx, m_background_color);
        nvgFill(ctx);

        nvgStrokeWidth(ctx, 2.0f);
        // nvgStrokeColor(ctx, m_background_color.contrasting_color());
        nvgStrokeColor(ctx, Color(1.f));
        nvgStroke(ctx);

        Widget::draw(ctx);
    }

private:
    Color m_background_color;
    bool m_skip_drawing;
};

CatalogView::CatalogView(Widget *parent) : Widget(parent)
{
    m_scroll_panel = add<VScrollPanel>();

    m_panel = m_scroll_panel->add<Panel>(Color(0.1f, 1.f));

    m_grid_layout = new GridLayout();
    m_grid_layout->set_margin(10);
    m_grid_layout->set_orientation(Orientation::Horizontal);
    m_grid_layout->set_resolution(3);
    m_grid_layout->set_spacing(10);
    m_panel->set_layout(m_grid_layout);

    for (int i = 0; i < 1000; ++i) {
        auto item = m_panel->add<CatalogViewItem>(Color((i % 11) * 0.1f, 1.f));
        // item->set_position(Vector2i(10, 10 + i * 110));
        item->set_fixed_size(Vector2i(100, 75));
    }
}

void CatalogView::draw(NVGcontext *ctx)
{
    Widget::draw(ctx);

    for (Widget *child : m_panel->children()) {
        auto position = m_panel->position() + child->position();
        auto size = child->size();
        bool visible = position.y() + size.y() >= 0 && position.y() <= m_scroll_panel->height();

        CatalogViewItem *item = static_cast<CatalogViewItem *>(child);
        item->set_skip_drawing(!visible);
    }

    // TODO: find better solution to draw items that just appeared
    // problem is that scroll panel does layout during draw and m_panel->position() is
    // not update until after being drawn.
    Widget::draw(ctx);
}

void CatalogView::perform_layout(NVGcontext *ctx)
{
    m_scroll_panel->set_fixed_size(m_parent->fixed_size());
    m_panel->set_fixed_width(m_parent->fixed_width());

    // recompute grid layout resolution
    int resolution =
        std::max(1, (m_panel->fixed_width() - (2 * m_grid_layout->margin())) / (100 + m_grid_layout->spacing(0)));
    m_grid_layout->set_resolution(resolution);

    Widget::perform_layout(ctx);
}

FR_NAMESPACE_END
