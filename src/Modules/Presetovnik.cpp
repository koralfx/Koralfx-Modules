#include "app.hpp"
#include "../Koralfx-Modules.hpp"
#include "../KoralfxComponents.hpp"


struct Presetovnik : Module {
	enum ParamIds {
		KNOB_PARAM,
		LED_BUTTON_PRESET_PARAM = KNOB_PARAM + 8,
		LED_UNI_PARAM  = LED_BUTTON_PRESET_PARAM + 10,
		NUM_PARAMS = LED_UNI_PARAM + 8
	};
	enum InputIds {
		CV_PRESET_INPUT,
		CV_PARAM_INPUT,
		NUM_INPUTS = CV_PARAM_INPUT + 8
	};
	enum OutputIds {
		CV_PRESET_OUTPUT,
		CV_PARAM_OUTPUT,
		NUM_OUTPUTS = CV_PARAM_OUTPUT + 8
	};
	enum LightIds {
		PRESET_LIGHT,
		UNI_LIGHT = PRESET_LIGHT + 10*3,
		NUM_LIGHTS = UNI_LIGHT + 8
	};

///////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////


    float pointerKnob [8] = {10.0, 10.0,10.0,10.0,10.0,10.0,10.0,10.0};
    NVGcolor colorPointer [8] ={nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff), nvgRGB(0x55, 0xaa, 0xff)};
    float presetKnobMemory [10][8];
    bool presetUniMemory [10][8];

    int preset = 0;
    int presetOld = 0;
    float cvPresetInputOld =0;
    bool presetChange = true;
    bool cvParamsInputDisconnect [8];
    bool unipolarChange = false;
    bool cvPresetInputActiveOld= false;
    int sourcePreset = 2; //2=internal Preset, 1= CV Preset


	dsp::SchmittTrigger presetTrigger [10];
	dsp::SchmittTrigger unipolarTrigger [8];

	Presetovnik() {
			config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

	configParam(KNOB_PARAM, 0.0f, 1.0f, 0.5f);
	configParam(LED_BUTTON_PRESET_PARAM, 0, 1, 0 );
	configParam(LED_UNI_PARAM, 0, 1, 0 );

		onReset();
	}


	void onReset() override {
		//Default values
		for (int i = 0; i < 10 ; i += 1) {
			for (int k = 0; k < 8 ; k += 1) {
				presetKnobMemory[i][k] = 0.5f;
				presetUniMemory[i][k] = true;
			}
		}
		presetChange = true;
	}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Step ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	void process(const ProcessArgs &args) override {

		//Test change unipolar led buttons
		int unipolarLed = -1;
		for (int i = 0; i < 8 ; i += 1) {
			if (unipolarTrigger[i].process(params[LED_UNI_PARAM + i].getValue())) {
				unipolarChange = true;
				unipolarLed = i;
			}
		}

		//Some unipolar button is pressed
		if (unipolarChange) {
			lights[UNI_LIGHT + unipolarLed].value = (lights[UNI_LIGHT + unipolarLed].value == 1) ? 0 : 1;
			presetUniMemory [preset][unipolarLed] = (lights[UNI_LIGHT + unipolarLed].value == 1) ? true : false;
			unipolarChange = false;
		}

		//CV Preset input active changed
		if (inputs[CV_PRESET_INPUT].isConnected() != cvPresetInputActiveOld) {
			for (int i = 0; i < 30 ; i += 1) {
				lights[PRESET_LIGHT + i ].value = 0;
			}
			presetChange = true;
			cvPresetInputActiveOld = !cvPresetInputActiveOld;
			sourcePreset = (cvPresetInputActiveOld) ? 1 : 2;
		}

		//Preset internal
		if (!inputs[CV_PRESET_INPUT].isConnected()) {
			for (int i = 0; i < 10 ; i += 1) {
				if (presetTrigger[i].process(params[LED_BUTTON_PRESET_PARAM + i].getValue())) {
					preset = i;
					presetChange = true;
					sourcePreset = 2;
				}
			}
		} else {
			//preset external
			if (cvPresetInputOld != inputs[CV_PRESET_INPUT].getVoltage()) {
				sourcePreset = 1;
				cvPresetInputOld = inputs[CV_PRESET_INPUT].getVoltage();
				preset = floor(cvPresetInputOld) - 1;
				if (preset == -1) preset = 0;
				presetChange = true;
				sourcePreset = 1;

			}
		}

		//Preset is changed
		if (presetChange) {
			lights[PRESET_LIGHT + (presetOld) * 3 + sourcePreset ].value = 0;
			lights[PRESET_LIGHT + (preset) * 3 + sourcePreset ].value = 1;
			presetChange = false;
			presetOld = preset;
			outputs[CV_PRESET_OUTPUT].setVoltage(preset+1);
			//Restore knob pointers and uni leds from preset memory
			for (int i = 0; i < 8 ; i += 1) {
				pointerKnob [i] = presetKnobMemory [preset][i];
				lights[UNI_LIGHT + i].value = presetUniMemory [preset][i];
			}
		}

		//Display and store knobs pointers
		for (int i = 0; i < 8 ; i += 1) {
			float knobValue = params[KNOB_PARAM + i].getValue();
			float pointerValue = pointerKnob [i];
			if (fabs(knobValue - pointerValue)<0.001) {
				pointerKnob [i] = params[KNOB_PARAM + i].getValue();
				presetKnobMemory [preset][i] =  pointerKnob [i];
			}
		}

		for (int i = 0; i < 8 ; i += 1) {
			float output = pointerKnob [i] * 10;
			float uniOutput = (lights[UNI_LIGHT + i].value == 0) ? 5: 0;
			if (!inputs[CV_PARAM_INPUT + i].isConnected()) {
				outputs[CV_PARAM_OUTPUT+ i].setVoltage(output - uniOutput);
				colorPointer[i] = nvgRGB(0x55, 0xaa, 0xff);
			} else {
				outputs[CV_PARAM_OUTPUT+ i].setVoltage(inputs[CV_PARAM_INPUT + i].getVoltage() - uniOutput);
				pointerKnob [i] = inputs[CV_PARAM_INPUT + i].getVoltage() / 10;
				colorPointer[i] = nvgRGB(0x55, 0xff, 0x55);
				//Store value in preset memory ?
				//presetKnobMemory [preset][i] =  pointerKnob [i];
			}
		}

	}

///////////////////////////////////////////////////////////////////////////////
// Store variables
///////////////////////////////////////////////////////////////////////////////

json_t *dataToJson() override {

	json_t *rootJ = json_object();
    //preset

    json_object_set_new(rootJ, "preset", json_integer(preset));

	// knobs
	int count = 0;
	json_t *knobsJ = json_array();
	for (int i = 0; i < 10; i++) {
		for (int k = 0; k < 8; k++) {
			json_array_insert_new(knobsJ, count, json_real(presetKnobMemory[i][k]));
			count++;
		}
	}
	json_object_set_new(rootJ, "knobs", knobsJ);

	// uni
	count = 0;
	json_t *uniJ = json_array();
	for (int i = 0; i < 10; i++) {
		for (int k = 0; k < 8; k++) {
			json_array_insert_new(uniJ, count , json_integer(presetUniMemory[i][k]));
			count++;
		}
	}
	json_object_set_new(rootJ, "uni", uniJ);

    return rootJ;
}

void dataFromJson(json_t *rootJ) override {



	json_t *j_preset = json_object_get(rootJ, "preset");
	preset = json_integer_value(j_preset);


	int count = 0;
	// knobs
	json_t *knobsJ = json_object_get(rootJ, "knobs");
	if (knobsJ) {
		for (int i = 0; i < 10; i++) {
			for (int k = 0; k < 8; k++) {
				json_t *knobJ = json_array_get(knobsJ, count);
				presetKnobMemory[i][k] = json_real_value(knobJ);
				count++;
			}
		}
	}


	count = 0;
	json_t *unisJ = json_object_get(rootJ, "uni");
	if (unisJ) {
		for (int i = 0; i < 10; i++) {
			for (int k = 0; k < 8; k++) {
				json_t *uniJ = json_array_get(unisJ, count);
				presetUniMemory[i][k] = (json_integer_value(uniJ) == 1) ? true : false;
				count++;
			}
		}
	}

}

};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


struct PresetovnikWidget : ModuleWidget {

    PresetovnikWidget(Presetovnik *module) {
        setModule(module);

		box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Presetovnik-Dark.svg")));
		
		float leftPos = 127;
		float topPos = 96;
		float deltaY = 50;
		
		for (int i = 0; i < 4 ; i += 1) {
			for (int k = 0; k < 2 ; k += 1) {

				if (module) {	
					Koralfx_knobRing *scale = new Koralfx_knobRing();
					scale->box.pos = Vec(leftPos + 16.5 + (k * 65), topPos + 18.5 + i *deltaY);
					scale->pointerKnob = &module->pointerKnob[i * 2 + k];
					scale->colorPointer = &module->colorPointer[i * 2 + k];
					addChild(scale);
				}



					addParam(createParam<RoundLargeBlackKnob>(Vec(leftPos - 2 + (k * 65), topPos+ i * deltaY),
						module, Presetovnik::KNOB_PARAM + (i * 2 + k)));
					addInput(createInput<PJ301MPort>(Vec(leftPos - 116 + (k * 27), topPos + 4 + i * deltaY),
						module, Presetovnik::CV_PARAM_INPUT + (i * 2 + k)));

					addParam(createParam<Koralfx_LEDButton>
						(Vec(leftPos - 60 + (k * 17), topPos + 10 + i * deltaY),
							module, Presetovnik::LED_UNI_PARAM + (i * 2 + k)));

					addChild(createLight<SmallLight<BlueLight>>
						(Vec(leftPos - 56 + (k * 17), topPos + 14 + i * deltaY),
							module, Presetovnik::UNI_LIGHT + (i * 2 + k)));

				
			}
		}
		for (int i = 0; i < 8 ; i += 1) {
			addOutput(createOutput<PJ301MPort>(Vec(13 + (i * 27), 302),
				module, Presetovnik::CV_PARAM_OUTPUT + i));
		}

		addInput(createInput<PJ301MPort>	(Vec(10, 47),	module, Presetovnik::CV_PRESET_INPUT));
		addOutput(createOutput<PJ301MPort>	(Vec(205, 47),	module, Presetovnik::CV_PRESET_OUTPUT));

		for (int i = 0; i < 10 ; i += 1) {
			addParam(createParam<Koralfx_LEDButton>(Vec(40 + (i * 16), 56),
				module, Presetovnik::LED_BUTTON_PRESET_PARAM + i));

			addChild(createLight<SmallLight<RedGreenBlueLight>>(Vec(44 + (i * 16), 60),
				module, Presetovnik::PRESET_LIGHT + (i * 3)));
		}
		
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////

Model *modelPresetovnik = createModel<Presetovnik, PresetovnikWidget>("Presetovnik");
