#include "Plateovnik6.hpp"

Plateovnik6::Plateovnik6() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Step ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Plateovnik6::step() {
}

///////////////////////////////////////////////////////////////////////////////
// Store variables
///////////////////////////////////////////////////////////////////////////////

json_t *Plateovnik6::toJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));

    return rootJ;
}

void Plateovnik6::fromJson(json_t *rootJ) {
	json_t *j_panelStyle = json_object_get(rootJ, "panelStyle");
	panelStyle = json_integer_value(j_panelStyle);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Plateovnik6Widget::Plateovnik6Widget(Plateovnik6 *module) : ModuleWidget(module){

	box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		DynamicPanelWidget *panel = new DynamicPanelWidget();
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Plateovnik6-Dark.svg")));
		panel->addPanel(SVG::load(assetPlugin(plugin, "res/Plateovnik6-Light.svg")));
		box.size = panel->box.size;
		panel->mode = &module->panelStyle;
		addChild(panel);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Context Menu ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

//Context menu code is adapted from The Dexter by Dale Johnson
//https://github.com/ValleyAudio

struct Plateovnik6PanelStyleItem : MenuItem {
    Plateovnik6* module;
    int panelStyle;
    void onAction(EventAction &e) override {
        module->panelStyle = panelStyle;
    }
    void step() override {
        rightText = (module->panelStyle == panelStyle) ? "✔" : "";
        MenuItem::step();
    }
};

void Plateovnik6Widget::appendContextMenu(Menu *menu) {
    Plateovnik6 *module = dynamic_cast<Plateovnik6*>(this->module);
    assert(module);

    // Panel style
    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Frame of mind"));
    menu->addChild(construct<Plateovnik6PanelStyleItem>(&MenuItem::text, "Dark Calm Night",
    	&Plateovnik6PanelStyleItem::module, module, &Plateovnik6PanelStyleItem::panelStyle, 0));
    menu->addChild(construct<Plateovnik6PanelStyleItem>(&MenuItem::text, "Happy Bright Day",
    	&Plateovnik6PanelStyleItem::module, module, &Plateovnik6PanelStyleItem::panelStyle, 1));

}

////////////////////////////////////////////////////////////////////////////////////////////////////

Model *modelPlateovnik6 = Model::create<Plateovnik6, Plateovnik6Widget>("KoralfxVCV", "Plateovnik6", "Plateovnik 6",
	BLANK_TAG);
