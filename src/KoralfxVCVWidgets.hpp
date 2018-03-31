#ifndef KORALFXVCV_WIDGETS_HPP
#define KORALFXVCV_WIDGETS_HPP

#include "KoralfxVCV.hpp"

///////////////////////////////////////////////////////////////////////////////
// Dynamic Panel
///////////////////////////////////////////////////////////////////////////////

struct PanelBorderWidget : TransparentWidget {
	void draw(NVGcontext *vg) override;
};

struct DynamicPanelWidget : FramebufferWidget {
    int* mode;
    int oldMode;
    std::vector<std::shared_ptr<SVG>> panels;
    SVGWidget* visiblePanel;
    PanelBorderWidget* border;

    DynamicPanelWidget();
    void addPanel(std::shared_ptr<SVG> svg);
    void step() override;
};

///////////////////////////////////////////////////////////////////////////////

enum DynamicViewMode {
    ACTIVE_HIGH_VIEW,
    ACTIVE_LOW_VIEW,
    ALWAYS_ACTIVE_VIEW
};

///////////////////////////////////////////////////////////////////////////////

#endif