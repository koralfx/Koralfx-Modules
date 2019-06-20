#include "../Koralfx-Modules.hpp"
#include "../KoralfxComponents.hpp"

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
		OCTAVE_LIGHT 	= NOTE_LIGHT + 12,
		NUM_LIGHTS 		= OCTAVE_LIGHT + 7
	};




    Quantovnik() {
        // Configure the module
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(OCTAVE_PARAM,  -4, 4, 0);
        configParam(COARSE_PARAM,  -12, 12, 0);
        configParam(CV_IN_PARAM,  0, 1, 1);
        configParam(CV_OUT_PARAM,  0, 1, 0);

    }




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Step ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	void process(const ProcessArgs &args) override {
		float octave 	= params[OCTAVE_PARAM].getValue();
		float cv 		= inputs[CV_PITCH_INPUT].getVoltage() + inputs[CV_COARSE_INPUT].getVoltage() + (params[COARSE_PARAM].getValue()/12.0);

		//Convert to Unipolar
		if (params[CV_IN_PARAM].getValue() == 0) cv += 5;

		float note 			= round(cv  * 12);
		int noteKey 		= int(note) % 12;
		cv 					= round(octave) + (note / 12);
		int octaveNumber 	= floor(cv);
		//Convert to Bipolar
		if (params[CV_OUT_PARAM].getValue() == 0) cv -= 5;
		outputs[CV_PITCH_OUTPUT].setVoltage(cv);

		//Light the right note light
		for (int i = 0; i < 12; i++) {
			lights[NOTE_LIGHT + i].value = (noteKey ==  i) ? 1.0 : 0.0;
		}
		//Light the right octave light
		for (int i = 0; i < 7; i++) {
			lights[OCTAVE_LIGHT + i].value = (octaveNumber ==  i+2) ? 1.0 : 0.0;
		}
	}


};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct QuantovnikWidget : ModuleWidget {

    QuantovnikWidget(Quantovnik *module) {
        setModule(module);

		box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		//box.size = panel->box.size;
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Quantovnik-Dark.svg")));


		//Knobs
		addParam(createParam<Koralfx_StepRoundLargeBlackKnob>(Vec(26, 45),
			module, Quantovnik::OCTAVE_PARAM));
		addParam(createParam<Koralfx_StepRoundLargeBlackKnob>(Vec(45, 113),
			module, Quantovnik::COARSE_PARAM));

		//Switches
		addParam(createParam<Koralfx_Switch_Red>(Vec(18, 253),	module, Quantovnik::CV_IN_PARAM));
		addParam(createParam<Koralfx_Switch_Red>(Vec(58, 253),	module, Quantovnik::CV_OUT_PARAM));

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
			addChild(createLight<SmallLight<RedLight>>(Vec(xPos + lightPos[i] * xDelta, rowPos[i]),
				module, Quantovnik::NOTE_LIGHT +  i));
		}

		for (int i = 0; i < 7; i++) {
			addChild(createLight<SmallLight<BlueLight>>(Vec(xPos + i * xDelta, 211),
				module, Quantovnik::OCTAVE_LIGHT + i));
		}

	}

};

////////////////////////////////////////////////////////////////////////////////////////////////////

Model *modelQuantovnik = createModel<Quantovnik, QuantovnikWidget>("Quantovnik");
