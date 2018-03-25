#include "Koralfx.hpp"

#define PI_4 0.78539816339
#define SQRT2_2 0.70710678118

	
struct Mixovnik : Module {
	enum ParamIds {
		AUX1_VOLUME,
		AUX2_VOLUME,
		MIX_L_VOLUME,
		MIX_R_VOLUME,
		MIX_LINK,
		AUX1_MUTE,
		AUX2_MUTE,
		MIX_L_MUTE,
		MIX_R_MUTE,
		LINK_PARAM,
		PAN_PARAM  = LINK_PARAM + 8,
		AUX1_PARAM = PAN_PARAM + 16,
		AUX2_PARAM = AUX1_PARAM + 16,
		VOLUME_PARAM = AUX2_PARAM + 16,
		MUTE_PARAM = VOLUME_PARAM + 16,
		SOLO_PARAM = MUTE_PARAM + 16,
		NUM_PARAMS = SOLO_PARAM + 16
	};
	enum InputIds {
		STEREO_INPUT_L,
		STEREO_INPUT_R,
		AUX1_INPUT_L,
		AUX1_INPUT_R,
		AUX2_INPUT_L,
		AUX2_INPUT_R,
		STRIPE_INPUT,
		STRIPE_CV_PAN_INPUT = STRIPE_INPUT + 16,
		STRIPE_CV_VOL_INPUT = STRIPE_CV_PAN_INPUT + 16,
		NUM_INPUTS = STRIPE_CV_VOL_INPUT + 16
	};
	enum OutputIds {
		STEREO_OUTPUT_L,
		STEREO_OUTPUT_R,
		AUX1_OUTPUT_L,
		AUX1_OUTPUT_R,
		AUX2_OUTPUT_L,
		AUX2_OUTPUT_R,
		NUM_OUTPUTS
	};
	enum LightIds {
		MIX_LIGHT_L,
		MIX_LIGHT_R,
		AUX1_LIGHT,
		AUX2_LIGHT,
		SIGNAL_LIGHT_NORMAL,
		SIGNAL_LIGHT_OVER,
		NUM_LIGHTS = SIGNAL_LIGHT_NORMAL + 32
	};


	enum DynamicViewMode {
   	ACTIVE_LOW,
   	ACTIVE_HIGH
	};
	int panelStyle = 0;
    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

	Mixovnik() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

//------------------------------------------------------------------------------------------------------
//------------------------------------------------- MAIN  ----------------------------------------------
//------------------------------------------------------------------------------------------------------

void Mixovnik::step() {

	float SUM_L = 0.0;
	float SUM_R = 0.0;

	float SUM_AUX1_L = 0.0;
	float SUM_AUX1_R = 0.0;
	float SUM_AUX2_L = 0.0;
	float SUM_AUX2_R = 0.0;

	float cvVolumeRatio[16];


	//Solo test
	bool soloTest = false;
	int step = 0;
	do  {
		if (params[SOLO_PARAM + step].value == 1) soloTest = true;
		step++;
	} while (!soloTest && step < 16);




	// Main loop
	for (int i = 0; i < 16; i++) {

		//Get signal
	float INPUT_SIGNAL = 0;
	cvVolumeRatio[i] = 1;

	if (inputs[STRIPE_CV_VOL_INPUT + i].active) cvVolumeRatio[i] = inputs[STRIPE_CV_VOL_INPUT + i].value/10;

		if (i%2 == 0) {
			if (soloTest && params[LINK_PARAM + (i/2)].value == 1) {
				params[SOLO_PARAM + i +1].value = params[SOLO_PARAM + i].value;
			}
			INPUT_SIGNAL = inputs[STRIPE_INPUT + i].value * cvVolumeRatio[i] * params[VOLUME_PARAM + i].value * ((params[MUTE_PARAM + i].value ==  0) ? 1.0 : 0.0);
		} else {
			if (params[LINK_PARAM + ((i-1)/2)].value == 0) {
				INPUT_SIGNAL = inputs[STRIPE_INPUT + i].value * cvVolumeRatio[i] * params[VOLUME_PARAM + i].value * ((params[MUTE_PARAM + i].value ==  0) ? 1.0 : 0.0);
			} else {
				INPUT_SIGNAL = inputs[STRIPE_INPUT + i].value * cvVolumeRatio[i-1] * params[VOLUME_PARAM + (i - 1)].value * ((params[MUTE_PARAM + i -1].value ==  0) ? 1.0 : 0.0);
			}

		}

		if (soloTest) {
			if (params[SOLO_PARAM + i].value == 0) INPUT_SIGNAL = 0;
		}

		//Constant-power panning
		float KNOB_PAN_POS = params[PAN_PARAM + i].value + (inputs[STRIPE_CV_PAN_INPUT + i].value / 5);

		//Anti invert phase
		if (KNOB_PAN_POS < -1) KNOB_PAN_POS = -1;
		if (KNOB_PAN_POS > 1) KNOB_PAN_POS = 1;

		double angle = KNOB_PAN_POS * PI_4;
		float GAIN_SIGNAL_L = (float) (SQRT2_2 * (cos(angle) - sin(angle)));
		float GAIN_SIGNAL_R = (float) (SQRT2_2 * (cos(angle) + sin(angle)));

		
		SUM_L += INPUT_SIGNAL * GAIN_SIGNAL_L;
		SUM_R += INPUT_SIGNAL * GAIN_SIGNAL_R;




		//AUX1 stripe send
		float KNOB_AUX1_POS = params[AUX1_PARAM + i].value; 
		SUM_AUX1_L += KNOB_AUX1_POS * INPUT_SIGNAL * GAIN_SIGNAL_L ;
		SUM_AUX1_R += KNOB_AUX1_POS * INPUT_SIGNAL * GAIN_SIGNAL_R;

		//AUX2 stripe send
		float KNOB_AUX2_POS = params[AUX2_PARAM + i].value;
		SUM_AUX2_L += KNOB_AUX2_POS * INPUT_SIGNAL * GAIN_SIGNAL_L;
		SUM_AUX2_R += KNOB_AUX2_POS * INPUT_SIGNAL * GAIN_SIGNAL_R;

		//Lights
		float SIGNAL_LEVEL_ABS = fabs(INPUT_SIGNAL);

		if (SIGNAL_LEVEL_ABS == 0) {
			lights[SIGNAL_LIGHT_NORMAL + i*2 +0].value = 0;
			lights[SIGNAL_LIGHT_NORMAL + i*2 +1].value = 0;
		} 

		if (SIGNAL_LEVEL_ABS > 0 && SIGNAL_LEVEL_ABS < 5.0) {
			lights[SIGNAL_LIGHT_NORMAL + i*2 +0].value = 1;
			lights[SIGNAL_LIGHT_NORMAL + i*2 +1].value = 0;
		} 

		if (SIGNAL_LEVEL_ABS > 5) {
			lights[SIGNAL_LIGHT_NORMAL + i*2 +0].value = 0;
			lights[SIGNAL_LIGHT_NORMAL + i*2 +1].value = 1;
		} 

		
	}

	//AUX1 and AUX2 sends
	outputs[AUX1_OUTPUT_L].value = SUM_AUX1_L;
	outputs[AUX1_OUTPUT_R].value = SUM_AUX1_R;

	outputs[AUX2_OUTPUT_L].value = SUM_AUX2_L;
	outputs[AUX2_OUTPUT_R].value = SUM_AUX2_R;

	//AUX1 and AUX2 returns
	float AUX1_SUM_L = inputs[AUX1_INPUT_L].value * params[AUX1_VOLUME].value * ((params[AUX1_MUTE].value ==  0) ? 1.0 : 0.0);
	float AUX1_SUM_R = inputs[AUX1_INPUT_R].value * params[AUX1_VOLUME].value * ((params[AUX1_MUTE].value ==  0) ? 1.0 : 0.0);
	
	float AUX2_SUM_L = inputs[AUX2_INPUT_L].value * params[AUX2_VOLUME].value * ((params[AUX2_MUTE].value ==  0) ? 1.0 : 0.0);
	float AUX2_SUM_R = inputs[AUX2_INPUT_R].value * params[AUX2_VOLUME].value * ((params[AUX2_MUTE].value ==  0) ? 1.0 : 0.0);
	
	SUM_L += AUX1_SUM_L + AUX2_SUM_L;
	SUM_R += AUX1_SUM_R + AUX2_SUM_R;


	//Exteranal mix
	SUM_L +=  inputs[STEREO_INPUT_L].value;
	SUM_R +=  inputs[STEREO_INPUT_R].value;

	//Mix sliders
	SUM_L *=  params[MIX_L_VOLUME].value;
	SUM_R *= ((params[MIX_LINK].value ==  0) ? params[MIX_R_VOLUME].value : params[MIX_L_VOLUME].value);

	//Final mix with mute switches
	SUM_L *= ((params[MIX_L_MUTE].value ==  0) ? 1.0 : 0.0);

	if (params[MIX_LINK].value == 1) {
		SUM_R *= ((params[MIX_L_MUTE].value ==  0) ? 1.0 : 0.0);
	} else {
		SUM_R *= ((params[MIX_R_MUTE].value ==  0) ? 1.0 : 0.0);
	}

	//Final out
	outputs[STEREO_OUTPUT_L].value = SUM_L;
	outputs[STEREO_OUTPUT_R].value = SUM_R;

	//Mix lights
	lights[MIX_LIGHT_L].value = (fabs(SUM_L) > 5) ? 1.0 : 0.0;
	lights[MIX_LIGHT_R].value = (fabs(SUM_R) > 5) ? 1.0 : 0.0;

	lights[AUX1_LIGHT].value = (fabs(AUX1_SUM_L) > 5 || fabs(AUX1_SUM_R)  > 5) ? 1.0 : 0.0;
	lights[AUX2_LIGHT].value = (fabs(AUX2_SUM_L) > 5 || fabs(AUX2_SUM_R)  > 5) ? 1.0 : 0.0;
}


json_t *Mixovnik::toJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "panelStyle", json_integer(panelStyle));

    return rootJ;
}

void Mixovnik::fromJson(json_t *rootJ) {
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

struct DynamicPanelMixovnik : FramebufferWidget {
    int* mode;
    int oldMode;
    std::vector<std::shared_ptr<SVG>> panels;
    SVGWidget* panel;

    DynamicPanelMixovnik() {
        mode = nullptr;
        oldMode = -1;
        panel = new SVGWidget();
        addChild(panel);
        addPanel(SVG::load(assetPlugin(plugin, "res/Mixovnik-Dark.svg")));
        addPanel(SVG::load(assetPlugin(plugin, "res/Mixovnik-Light.svg")));
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




MixovnikWidget::MixovnikWidget() {
	Mixovnik *module = new Mixovnik();
	setModule(module);
	box.size = Vec(58 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		DynamicPanelMixovnik *panel = new DynamicPanelMixovnik();
        panel->box.size = box.size;
        panel->mode = &module->panelStyle;
        addChild(panel);
	}

	//Standard screws
	//addChild(createScrew<ScrewSilver>(Vec(0, 0)));
	//addChild(createScrew<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, 0)));
	//addChild(createScrew<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	//addChild(createScrew<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	//Set base position
	float xPos = 17;
	float yPos = 20;
	float xDelta = 40;
	
	//AUX1 inputs and outputs
	addInput(createInput<PJ301MPort>(Vec(694, yPos +7), module, Mixovnik::AUX1_INPUT_L));
	addInput(createInput<PJ301MPort>(Vec(733, yPos +7), module, Mixovnik::AUX1_INPUT_R));

	addOutput(createOutput<PJ301MPort>(Vec(780, yPos +7), module, Mixovnik::AUX1_OUTPUT_L));
	addOutput(createOutput<PJ301MPort>(Vec(815, yPos +7), module, Mixovnik::AUX1_OUTPUT_R));

	//AUX2 inputs and outputs
	addInput(createInput<PJ301MPort>(Vec(694, yPos + 45), module, Mixovnik::AUX2_INPUT_L));
	addInput(createInput<PJ301MPort>(Vec(733, yPos + 45), module, Mixovnik::AUX2_INPUT_R));

	addOutput(createOutput<PJ301MPort>(Vec(780, yPos + 45), module, Mixovnik::AUX2_OUTPUT_L));
	addOutput(createOutput<PJ301MPort>(Vec(815, yPos + 45), module, Mixovnik::AUX2_OUTPUT_R));

	//External stereo inputs
	addInput(createInput<PJ301MPort>(Vec(699, yPos + 285), module, Mixovnik::STEREO_INPUT_L));
	addInput(createInput<PJ301MPort>(Vec(733, yPos + 285), module, Mixovnik::STEREO_INPUT_R));

	//Stereo mix outputs
	addOutput(createOutput<PJ301MPort>(Vec(782, yPos + 285), module, Mixovnik::STEREO_OUTPUT_L));
	addOutput(createOutput<PJ301MPort>(Vec(816, yPos + 285), module, Mixovnik::STEREO_OUTPUT_R));

  
    //Stripes elements
	for (int i = 0; i < 16; i++) {
		addChild(createLight<SmallLight<GreenRedLight>>	(Vec(xPos + 27 + i*xDelta,	yPos + 110), module, Mixovnik::SIGNAL_LIGHT_NORMAL + i*2));
		addParam(createParam<RoundSmallBlackKnob>		(Vec(xPos + 0 + i*xDelta,	yPos +   0), module, Mixovnik::AUX1_PARAM + i, 0, 1, 0.0));
		addParam(createParam<RoundSmallBlackKnob>		(Vec(xPos + 0 + i*xDelta,	yPos +  37), module, Mixovnik::AUX2_PARAM + i, 0, 1, 0.0));
		addParam(createParam<RoundSmallBlackKnob>		(Vec(xPos + 0 + i*xDelta,	yPos +  76), module, Mixovnik::PAN_PARAM + i, -1, 1, 0.0));
		addParam(createParam<Koralfx_SliderPot>			(Vec(xPos + 3 + i*xDelta,	yPos + 110), module, Mixovnik::VOLUME_PARAM + i, 0.0f, 1.0f, 0.9f));
		addParam(createParam<Koralfx_Switch_Red>		(Vec(xPos + 8 + i*xDelta,	yPos + 228), module, Mixovnik::MUTE_PARAM + i, 0.0, 1.0, 0.0));
		if (i%2 == 0) addParam(createParam<Koralfx_Switch_Blue>(Vec(xPos + 8 + (i+0.5)*xDelta,	yPos + 228), module, Mixovnik::LINK_PARAM + (i/2), 0.0, 1.0, 0.0));
		addInput(createInput<PJ301MPort>				(Vec(xPos + 3 + i*xDelta,	yPos + 266), module, Mixovnik::STRIPE_INPUT + i));
		addInput(createInput<PJ301MPort>				(Vec(xPos + 3 + i*xDelta,	yPos + 292), module, Mixovnik::STRIPE_CV_PAN_INPUT + i));
		addInput(createInput<PJ301MPort>				(Vec(xPos + 3 + i*xDelta,	yPos + 318), module, Mixovnik::STRIPE_CV_VOL_INPUT + i));
		addParam(createParam<Koralfx_Switch_Green>		(Vec(xPos + 8 + i*xDelta,	yPos + 245), module, Mixovnik::SOLO_PARAM + i, 0.0, 1.0, 0.0));
	}

	//Final volume sliders
	addParam(createParam<Koralfx_SliderPot>(Vec(xPos - 2 + 17*xDelta, yPos + 110), module, Mixovnik::AUX1_VOLUME, 0.0f, 1.0f, 0.9f));
	addParam(createParam<Koralfx_SliderPot>(Vec(xPos - 2 + 18*xDelta, yPos + 110), module, Mixovnik::AUX2_VOLUME, 0.0f, 1.0f, 0.9f));
	addParam(createParam<Koralfx_SliderPot>(Vec(xPos + 3 + 19*xDelta, yPos + 110), module, Mixovnik::MIX_L_VOLUME, 0.0f, 1.0f, 0.9f));
	addParam(createParam<Koralfx_SliderPot>(Vec(xPos + 1 + 20*xDelta, yPos + 110), module, Mixovnik::MIX_R_VOLUME, 0.0f, 1.0f, 0.9f));

	//Final mute switches
	addParam(createParam<Koralfx_Switch_Red>(Vec(xPos + 3 + 17*xDelta, yPos + 227), module, Mixovnik::AUX1_MUTE, 0, 1, 0));
	addParam(createParam<Koralfx_Switch_Red>(Vec(xPos + 3 + 18*xDelta, yPos + 227), module, Mixovnik::AUX2_MUTE, 0, 1, 0));
	addParam(createParam<Koralfx_Switch_Red>(Vec(xPos + 8 + 19*xDelta, yPos + 227), module, Mixovnik::MIX_L_MUTE, 0, 1, 0));
	addParam(createParam<Koralfx_Switch_Red>(Vec(xPos + 6 + 20*xDelta, yPos + 227), module, Mixovnik::MIX_R_MUTE, 0, 1, 0));

	//Stereo mix link switch
	addParam(createParam<Koralfx_Switch_Blue>(Vec(xPos + 7 + 19.5*xDelta, yPos + 227), module, Mixovnik::MIX_LINK, 0, 1, 0));

	//Final mix lights
	addChild(createLight<SmallLight<RedLight>>	(Vec(703,120), module, Mixovnik::AUX1_LIGHT));
	addChild(createLight<SmallLight<RedLight>>	(Vec(743,120), module, Mixovnik::AUX2_LIGHT));

	addChild(createLight<SmallLight<RedLight>>	(Vec(788,120), module, Mixovnik::MIX_LIGHT_L));
	addChild(createLight<SmallLight<RedLight>>	(Vec(826,120), module, Mixovnik::MIX_LIGHT_R));


}

//------------------------------------------------------------------------------------------------------
//-------------------------------------------- Context Menu --------------------------------------------
//------------------------------------------------------------------------------------------------------

//Context menu code is adapted from The Dexter by Dale Johnson
//https://github.com/ValleyAudio

struct MixovnikPanelStyleItem : MenuItem {
   Mixovnik* mixovnik;
    int panelStyle;
    void onAction(EventAction &e) override {
       mixovnik->panelStyle = panelStyle;
    }
    void step() override {
        rightText = (mixovnik->panelStyle == panelStyle) ? "✔" : "";
    }
};

Menu* MixovnikWidget::createContextMenu() {
    Menu* menu = ModuleWidget::createContextMenu();
    Mixovnik* mixovnik = dynamic_cast<Mixovnik*>(module);
    assert(mixovnik);

    // Panel Style
    MenuLabel *panelStyleSpacerLabel = new MenuLabel();
    menu->addChild(panelStyleSpacerLabel);
    MenuLabel *panelStyleLabel = new MenuLabel();
    panelStyleLabel->text = "Frame of mind";
    menu->addChild(panelStyleLabel);

    MixovnikPanelStyleItem *darkPanelStyleItem = new MixovnikPanelStyleItem();
    darkPanelStyleItem->text = "Dark Calm Night";
    darkPanelStyleItem->mixovnik = mixovnik;
    darkPanelStyleItem->panelStyle = 0;
    menu->addChild(darkPanelStyleItem);

    MixovnikPanelStyleItem *lightPanelStyleItem = new MixovnikPanelStyleItem();
    lightPanelStyleItem->text = "Happy Bright Day";
    lightPanelStyleItem->mixovnik = mixovnik;
    lightPanelStyleItem->panelStyle = 1;
    menu->addChild(lightPanelStyleItem);



    return menu;
}

