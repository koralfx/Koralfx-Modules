#include "Koralfx.hpp"
#include "dsp/digital.hpp"

#include <sstream>
#include <iomanip>

struct Beatovnik : Module {
	enum ParamIds {
		WIDTH_PARAM,
		TIME_PARAM,
		TIME_T_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUTPUT,
		BEAT2X_MUL_OUTPUT,
		BEAT4X_MUL_OUTPUT,
		BEAT2X_DIV_OUTPUT,
		BEAT4X_DIV_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CLOCK_LIGHT,
		CLOCK_LOCK_LIGHT,
		BEAT2X_MUL_LIGHT,
		BEAT4X_MUL_LIGHT,
		BEAT2X_DIV_LIGHT,
		BEAT4X_DIV_LIGHT,
		NOTE_LIGHT,
		NUM_LIGHTS = NOTE_LIGHT + 9
	};

	enum DynamicViewMode {
   	ACTIVE_LOW,
   	ACTIVE_HIGH
	};
	int panelStyle = 0;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

    bool inMemory = false;
    
    bool beatLock = false;
    float beatTime = 0;
    int beatCount = 0;
    int beatCountMemory = 0;
    float beatMemory = 0;

    int stepper =0;
    bool stepperInc = false;

    float gateWidth2xMul = 0;
    float gateWidth4xMul = 0;
    float gateWidth2xDiv = 0;
    float gateWidth4xDiv = 0;

    bool gateDec2xMul = false;
    bool gateDec4xMul = false;
    bool gateDec2xDiv = false;
    bool gateDec4xDiv = false;

	int tempo = 0;
	SchmittTrigger clockTrigger;
	PulseGenerator PulseGenerator;

	Beatovnik() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

//------------------------------------------------------------------------------------------------------
//------------------------------------------------- MAIN  ----------------------------------------------
//------------------------------------------------------------------------------------------------------

void Beatovnik::step() {

float deltaTime = engineGetSampleTime();

if (inputs[CLOCK_INPUT].active) {

	float clockInput = inputs[CLOCK_INPUT].value;

	//A rising slope
	if ((clockTrigger.process(inputs[CLOCK_INPUT].value)) && !inMemory) {
		stepper = 0;
		beatCount ++;
		PulseGenerator.trigger(0.1);
		lights[CLOCK_LIGHT].value = 1;
		inMemory = true;

		//Set divide with tempo knob 
		int choice = round(params[TIME_PARAM].value);
		float ratio =0;

		switch(choice) {
			case 0:
			ratio = 0.25; 
			break;
			
			case 1:
			ratio = 0.5; 
			break;
			
			case 2:
			ratio = 0.75; 
			break;

			case 3:
			ratio = 1.0; 
			break;

			case 4:
			ratio = 1.25 ;
			break;

			case 5:
			ratio = 1.5 ;
			break;

			case 6:
			ratio = 2 ;
			break;

			case 7:
			ratio = 4 ;
			break;



		}

		float divide = ratio * ((params[TIME_T_PARAM].value) ? 0.333 : 1);

		float CV = (log(beatMemory * 1000 * divide) /log(10 / 1e-3));
		outputs[CV_OUTPUT].value = CV * 10;

		for (int i = 0; i < 9; i++) {
			lights[NOTE_LIGHT + i].value = ((choice == i) ? 1 : 0);
		}

			
		//BPM is locked
		if (beatCount == 2) {
			lights[CLOCK_LOCK_LIGHT].value = 1;
			beatLock = true;
			beatMemory = beatTime;
			tempo = std::round(60/beatMemory);
			}

		//BPM lost
		if (beatCount > 2) {
			if (fabs(beatMemory - beatTime) > 0.005) {
				beatLock = false;
				beatCount = 0;
				lights[CLOCK_LOCK_LIGHT].value = 0;
				stepper = 0;
				stepperInc = false;
				for (int i = 0; i < 13; i++) {
				lights[CLOCK_LIGHT + i].value = 0;
				tempo = std::round(0);
		}
			}
		}
		beatTime = 0;
	}



	//Falling slope
	if (clockInput <= 0 && inMemory) {
		//lights[CLOCK_LIGHT].value = 0;
		inMemory = false;
	}

	//When BPM is locked...
	if (beatLock) {
		int division = 16;
		int bM = round((beatMemory / division) * 1000000);
		int bT = round(beatTime * 1000000);
		
		if ( bT % bM <= deltaTime * 1000000) {
			stepperInc = true;
		}

		//MULTIPLY 4X
		int noteRate = 4;
		if (stepperInc) {
			if ( stepper % noteRate == 0) {
				lights[BEAT4X_MUL_LIGHT].value = 1;
				outputs[BEAT4X_MUL_OUTPUT].value = 10;
				gateDec4xMul = true;
			}
		}

		//led off
		if (gateDec4xMul) gateWidth4xMul -= 1;
		if (gateWidth4xMul <= 0 ) {
			gateDec4xMul = false;
			gateWidth4xMul = params[WIDTH_PARAM].value * (beatMemory / deltaTime / noteRate);
			lights[BEAT4X_MUL_LIGHT].value = 0;
			outputs[BEAT4X_MUL_OUTPUT].value = 0;
		}


		//MULTIPLY 2X
		noteRate = 8;
		if (stepperInc) {
			if ( stepper % noteRate == 0) {
				lights[BEAT2X_MUL_LIGHT].value = 1;
				outputs[BEAT2X_MUL_OUTPUT].value = 10;
				gateDec2xMul = true;
			}
		}

		//led off
		if (gateDec2xMul) gateWidth2xMul -= 1;
		if (gateWidth2xMul <= 0 ) {
			gateDec2xMul = false;
			gateWidth2xMul = params[WIDTH_PARAM].value * (beatMemory / deltaTime / noteRate) * 4;
			lights[BEAT2X_MUL_LIGHT].value = 0;
			outputs[BEAT2X_MUL_OUTPUT].value = 0;
		}



		if (beatCountMemory != beatCount) {

			//DIV 2X
			if (stepperInc) {
				if ( beatCount % 2 == 0) {
					lights[BEAT2X_DIV_LIGHT].value = 1;
					outputs[BEAT2X_DIV_OUTPUT].value = 10;
					gateDec2xDiv = true;
				}
			}

			//DIV 4X
			if (stepperInc) {
				if ( beatCount % 4 == 0) {
					lights[BEAT4X_DIV_LIGHT].value = 1;
					outputs[BEAT4X_DIV_OUTPUT].value = 10;
					gateDec4xDiv = true;
				}
			}

		}


		//DIV 2X led off
		if (gateDec2xDiv) gateWidth2xDiv -= 1;
		if (gateWidth2xDiv <= 0 ) {
			gateDec2xDiv = false;
			gateWidth2xDiv = params[WIDTH_PARAM].value * (beatMemory / deltaTime / noteRate) * 16;
			lights[BEAT2X_DIV_LIGHT].value = 0;
			outputs[BEAT2X_DIV_OUTPUT].value = 0;
		}

		//DIV 4X led off
		if (gateDec4xDiv) gateWidth4xDiv -= 1;
		if (gateWidth4xDiv <= 0 ) {
			gateDec4xDiv = false;
			gateWidth4xDiv = params[WIDTH_PARAM].value * (beatMemory / deltaTime / noteRate) * 32;
			lights[BEAT4X_DIV_LIGHT].value = 0;
			outputs[BEAT4X_DIV_OUTPUT].value = 0;
		}


		//Nest step of stepper
		if (stepperInc) {
			stepper++;
 			stepperInc = false;
		}


	} //end of beatlock routine



	beatTime += deltaTime;

	//when beat is lost
	if (beatTime > 2 ) {
		beatLock = false;
		beatCount = 0;
		lights[CLOCK_LOCK_LIGHT].value = 0;
		stepper = 0;
		stepperInc = false;
		tempo = std::round(0);
		for (int i = 0; i < 13; i++) {
			lights[CLOCK_LIGHT + i].value = 0;
		}
	}


	beatCountMemory = beatCount;


	} else {

		beatLock = false;
		beatCount = 0;
		//lights[CLOCK_LOCK_LIGHT].value = 0;
		//lights[BEAT4X_MUL_LIGHT].value = 0;

		for (int i = 0; i < 13; i++) {
			lights[CLOCK_LIGHT + i].value = 0;
		}




	} //end of input active routine
	bool pulse = PulseGenerator.process(1.0 / engineGetSampleRate());
	if (pulse == 0) lights[CLOCK_LIGHT].value = 0;

}


json_t *Beatovnik::toJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));

    return rootJ;
}

void Beatovnik::fromJson(json_t *rootJ) {
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

struct DynamicPanelBeatovnik : FramebufferWidget {
    int* mode;
    int oldMode;
    std::vector<std::shared_ptr<SVG>> panels;
    SVGWidget* panel;

    DynamicPanelBeatovnik() {
        mode = nullptr;
        oldMode = -1;
        panel = new SVGWidget();
        addChild(panel);
        addPanel(SVG::load(assetPlugin(plugin, "res/Beatovnik-Dark.svg")));
        addPanel(SVG::load(assetPlugin(plugin, "res/Beatovnik-Light.svg")));
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
//BMP Display widget code is adapted from BPMClock by Alfredo Santamaria
//https://github.com/AScustomWorks/AS
////////////////////////////////////
struct BpmDisplayWidget : TransparentWidget {
  int *value;
  std::shared_ptr<Font> font;

  BpmDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  void draw(NVGcontext *vg) override
  {

    // text 
    nvgFontSize(vg, 13);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.0);

    std::stringstream to_display;   
    to_display << std::setw(3) << *value;

    Vec textPos = Vec(4.0f, 17.0f); 

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\", NULL);

    textColor = nvgRGB(0xff, 0xcc, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};




BeatovnikWidget::BeatovnikWidget() {
	Beatovnik *module = new Beatovnik();
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		DynamicPanelBeatovnik *panel = new DynamicPanelBeatovnik();
        panel->box.size = box.size;
        panel->mode = &module->panelStyle;
        addChild(panel);
	}
	//Standard screws
	//addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	//addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	//addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	//addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	//Display
	BpmDisplayWidget *display = new BpmDisplayWidget();
	display->box.pos = Vec(6,290);
	display->box.size = Vec(45, 20);
	display->value = &module->tempo;
	addChild(display);

	//Knobs
	addParam(createParam<RoundSmallBlackKnob>(Vec(51, 249),	module, Beatovnik::WIDTH_PARAM,	0.01, 0.99, 0.5));
	addParam(createParam<RoundBlackKnob>(Vec(6, 73),	module, Beatovnik::TIME_PARAM,	-0.3, 7.3, 2));

	//Buttons
	addParam(createParam<Koralfx_Switch_Blue>(Vec(54,60), module, Beatovnik::TIME_T_PARAM, 0, 1, 0));


	//Inputs
	addInput(createInput<PJ301MPort>	(Vec(13, 251),	module, Beatovnik::CLOCK_INPUT));

	//Outputs
	addOutput(createOutput<PJ301MPort>(Vec(52, 88), module, Beatovnik::CV_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(14, 145), module, Beatovnik::BEAT2X_MUL_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(52, 145), module, Beatovnik::BEAT4X_MUL_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(14, 205), module, Beatovnik::BEAT2X_DIV_OUTPUT));
	addOutput(createOutput<PJ301MPort>(Vec(52, 205), module, Beatovnik::BEAT4X_DIV_OUTPUT));

	//Lights
	addChild(createLight<SmallLight<RedLight>>(Vec(17, 323), module, Beatovnik::CLOCK_LIGHT));
	addChild(createLight<MediumLight<GreenLight>>(Vec(69, 321), module, Beatovnik::CLOCK_LOCK_LIGHT));


	addChild(createLight<SmallLight<BlueLight>>(Vec(30, 176), module, Beatovnik::BEAT2X_MUL_LIGHT));
	addChild(createLight<SmallLight<BlueLight>>(Vec(69, 176), module, Beatovnik::BEAT4X_MUL_LIGHT));
	addChild(createLight<SmallLight<BlueLight>>(Vec(30, 236), module, Beatovnik::BEAT2X_DIV_LIGHT));
	addChild(createLight<SmallLight<BlueLight>>(Vec(69, 236), module, Beatovnik::BEAT4X_DIV_LIGHT));


	for (int i = 0; i < 8; i++) {
		addChild(createLight<SmallLight<RedLight>>(Vec(9+i*9.5, 53), module, Beatovnik::NOTE_LIGHT + i));
	}




}


//------------------------------------------------------------------------------------------------------
//-------------------------------------------- Context Menu --------------------------------------------
//------------------------------------------------------------------------------------------------------

//Context menu code is adapted from The Dexter by Dale Johnson
//https://github.com/ValleyAudio

struct BeatovnikPanelStyleItem : MenuItem {
   Beatovnik* teatovnik;
    int panelStyle;
    void onAction(EventAction &e) override {
       teatovnik->panelStyle = panelStyle;
    }
    void step() override {
        rightText = (teatovnik->panelStyle == panelStyle) ? "✔" : "";
    }
};

Menu* BeatovnikWidget::createContextMenu() {
    Menu* menu = ModuleWidget::createContextMenu();
    Beatovnik* teatovnik = dynamic_cast<Beatovnik*>(module);
    assert(teatovnik);

    // Panel Style
    MenuLabel *panelStyleSpacerLabel = new MenuLabel();
    menu->addChild(panelStyleSpacerLabel);
    MenuLabel *panelStyleLabel = new MenuLabel();
    panelStyleLabel->text = "Frame of mind";
    menu->addChild(panelStyleLabel);

    BeatovnikPanelStyleItem *darkPanelStyleItem = new BeatovnikPanelStyleItem();
    darkPanelStyleItem->text = "Dark Calm Night";
    darkPanelStyleItem->teatovnik = teatovnik;
    darkPanelStyleItem->panelStyle = 0;
    menu->addChild(darkPanelStyleItem);

    BeatovnikPanelStyleItem *lightPanelStyleItem = new BeatovnikPanelStyleItem();
    lightPanelStyleItem->text = "Happy Bright Day";
    lightPanelStyleItem->teatovnik = teatovnik;
    lightPanelStyleItem->panelStyle = 1;
    menu->addChild(lightPanelStyleItem);



    return menu;
}




