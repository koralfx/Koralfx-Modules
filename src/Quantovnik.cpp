#include "Koralfx.hpp"
//#include "DynamicPanelWidget.cpp"


struct Quantovnik : Module {
	enum ParamIds {
		OCTAVE_PARAM,
		COARSE_PARAM,
		CV_IN_PARAM,
		CV_OUT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CV_PITCH_INPUT,
		CV_COARSE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CV_PITCH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NOTE_LIGHT,
		OCTAVE_LIGHT = NOTE_LIGHT + 12,
		NUM_LIGHTS = OCTAVE_LIGHT + 7
	};

	enum DynamicViewMode {
   	ACTIVE_LOW,
   	ACTIVE_HIGH
	};
	int panelStyle = 0;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

	Quantovnik() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

//------------------------------------------------------------------------------------------------------
//------------------------------------------------- MAIN  ----------------------------------------------
//------------------------------------------------------------------------------------------------------

void Quantovnik::step() {
	float octave = params[OCTAVE_PARAM].value;
	float cv = inputs[CV_PITCH_INPUT].value + inputs[CV_COARSE_INPUT].value + params[COARSE_PARAM].value;

	//Convert to Unipolar
	if (params[CV_IN_PARAM].value == 0) cv += 5;

	float note = round(cv  * 12);
	int noteKey = int(note) % 12;

	cv = round(octave) + (note / 12);
	int octaveNumber = floor(cv);
	//Convert to Bipolar
	if (params[CV_OUT_PARAM].value == 0) cv -= 5;

	outputs[CV_PITCH_OUTPUT].value = cv;
	

	//Light the right note light
	for (int i = 0; i < 12; i++) {
		lights[NOTE_LIGHT + i].value = (noteKey ==  i) ? 1.0 : 0.0;
	}

	//Light the right octave light
	for (int i = 0; i < 7; i++) {
		lights[OCTAVE_LIGHT + i].value = (octaveNumber ==  i+2) ? 1.0 : 0.0;
	}

}


json_t *Quantovnik::toJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));

    return rootJ;
}

void Quantovnik::fromJson(json_t *rootJ) {
    json_t *j_panelStyle = json_object_get(rootJ, "panelStyle");
    panelStyle = json_integer_value(j_panelStyle);
}


//------------------------------------------------------------------------------------------------------
//-------------------------------------------------- GUI  ----------------------------------------------
//------------------------------------------------------------------------------------------------------


//Panel Border and Dynamic panel code is adapted from The Dexter by Dale Johnson
//https://github.com/ValleyAudio

struct PanelBorder : TransparentWidget {
	void draw(NVGcontext *vg) override {
		NVGcolor borderColor = nvgRGBAf(0.5, 0.5, 0.5, 0.5);
		nvgBeginPath(vg);
		nvgRect(vg, 0.5, 0.5, box.size.x - 1.0, box.size.y - 1.0);
		nvgStrokeColor(vg, borderColor);
		nvgStrokeWidth(vg, 1.0);
		nvgStroke(vg);
	}
};

struct DynamicPanelQuantovnik : FramebufferWidget {
    int* mode;
    int oldMode;
    std::vector<std::shared_ptr<SVG>> panels;
    SVGWidget* panel;

    DynamicPanelQuantovnik() {
        mode = nullptr;
        oldMode = -1;
        panel = new SVGWidget();
        addChild(panel);
        addPanel(SVG::load(assetPlugin(plugin, "res/Quantovnik-Dark.svg")));
        addPanel(SVG::load(assetPlugin(plugin, "res/Quantovnik-Light.svg")));
        PanelBorder *pb = new PanelBorder();
        pb->box.size = box.size;
        addChild(pb);
    }

    void addPanel(std::shared_ptr<SVG> svg) {
        panels.push_back(svg);
        if(!panel->svg) {
            panel->setSVG(svg);
            box.size = panel->box.size.div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);
        }
    }

    void step() override {
        if (nearf(gPixelRatio, 1.f)) {
            oversample = 2.f;
        }
        if(mode != nullptr && *mode != oldMode) {
            panel->setSVG(panels[*mode]);
            oldMode = *mode;
            dirty = true;
        }
    }
};


QuantovnikWidget::QuantovnikWidget() {
	Quantovnik *module = new Quantovnik();
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		DynamicPanelQuantovnik *panel = new DynamicPanelQuantovnik();
        panel->box.size = box.size;
        panel->mode = &module->panelStyle;
        addChild(panel);
	}
	//Standard screws
	//addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	//addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	//addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	//addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	//Knobs
	addParam(createParam<RoundBlackKnob>(Vec(26, 45),	module, Quantovnik::OCTAVE_PARAM,	-4.5, 4.5, 0.0));
	addParam(createParam<RoundBlackKnob>(Vec(45, 113),	module, Quantovnik::COARSE_PARAM,	-1, 1, 0.0));

	//Switches
	addParam(createParam<Koralfx_Switch_Red>(Vec(18, 253),	module, Quantovnik::CV_IN_PARAM,	0.0, 1.0, 1.0));
	addParam(createParam<Koralfx_Switch_Red>(Vec(58, 253),	module, Quantovnik::CV_OUT_PARAM,	0.0, 1.0, 0.0));

	//Inputs
	addInput(createInput<PJ301MPort>	(Vec(13, 298),	module, Quantovnik::CV_PITCH_INPUT));
	addInput(createInput<PJ301MPort>	(Vec(10, 121),	module, Quantovnik::CV_COARSE_INPUT));

	//Outputs
	addOutput(createOutput<PJ301MPort>(Vec(52, 298), module, Quantovnik::CV_PITCH_OUTPUT));

	//Note lights - set base position
	float xPos		=   9;
	float yPos1		= 192;
	float yPos2		= 175;
	float xDelta	=  11;
	

	float lightPos [12]	= {0.0, 0.5, 1.0, 1.5, 2.0, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0};
	float rowPos [12]	= {yPos1, yPos2, yPos1, yPos2, yPos1, yPos1, yPos2, yPos1, yPos2, yPos1, yPos2, yPos1};

	for (int i = 0; i < 12; i++) {
		addChild(createLight<SmallLight<RedLight>>(Vec(xPos + lightPos[i] * xDelta, rowPos[i]), module, Quantovnik::NOTE_LIGHT +  i));
	}

	for (int i = 0; i < 7; i++) {
		addChild(createLight<SmallLight<BlueLight>>(Vec(xPos + i * xDelta, 211), module, Quantovnik::OCTAVE_LIGHT + i));
	}


}

//------------------------------------------------------------------------------------------------------
//-------------------------------------------- Context Menu --------------------------------------------
//------------------------------------------------------------------------------------------------------

//Context menu code is adapted from The Dexter by Dale Johnson
//https://github.com/ValleyAudio

struct QuantovnikPanelStyleItem : MenuItem {
   Quantovnik* quantovnik;
    int panelStyle;
    void onAction(EventAction &e) override {
       quantovnik->panelStyle = panelStyle;
    }
    void step() override {
        rightText = (quantovnik->panelStyle == panelStyle) ? "✔" : "";
    }
};

Menu* QuantovnikWidget::createContextMenu() {
    Menu* menu = ModuleWidget::createContextMenu();
    Quantovnik* quantovnik = dynamic_cast<Quantovnik*>(module);
    assert(quantovnik);

    // Panel Style
    MenuLabel *panelStyleSpacerLabel = new MenuLabel();
    menu->addChild(panelStyleSpacerLabel);
    MenuLabel *panelStyleLabel = new MenuLabel();
    panelStyleLabel->text = "Frame of mind";
    menu->addChild(panelStyleLabel);

    QuantovnikPanelStyleItem *darkPanelStyleItem = new QuantovnikPanelStyleItem();
    darkPanelStyleItem->text = "Dark Calm Night";
    darkPanelStyleItem->quantovnik = quantovnik;
    darkPanelStyleItem->panelStyle = 0;
    menu->addChild(darkPanelStyleItem);

    QuantovnikPanelStyleItem *lightPanelStyleItem = new QuantovnikPanelStyleItem();
    lightPanelStyleItem->text = "Happy Bright Day";
    lightPanelStyleItem->quantovnik = quantovnik;
    lightPanelStyleItem->panelStyle = 1;
    menu->addChild(lightPanelStyleItem);



    return menu;
}

