#include "Koralfx.hpp"

struct Quantovnik : Module {
	enum ParamIds {
		OCTAVE_PARAM,
		COURSE_PARAM,
		CV_IN_PARAM,
		CV_OUT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CV_PITCH_INPUT,
		CV_COURSE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CV_PITCH_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NOTE_LIGHT,
		NUM_LIGHTS = NOTE_LIGHT + 12
	};

	Quantovnik() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void Quantovnik::step() {
	float octave = params[OCTAVE_PARAM].value;
	float cv = inputs[CV_PITCH_INPUT].value + inputs[CV_COURSE_INPUT].value + params[COURSE_PARAM].value;

	//Convert to Unipolar
	if (params[CV_IN_PARAM].value == 0) cv += 5;

	float note = floor(cv  * 12);
	int noteKey = int(note) % 12;

	cv = round(octave) + (note / 12);

	//Convert to Bipolar
	if (params[CV_OUT_PARAM].value == 0) cv -= 5;

	outputs[CV_PITCH_OUTPUT].value = cv;

	//Light the right light
	for (int i = 0; i < 12; i++) {
		lights[NOTE_LIGHT + i].value = (noteKey ==  i) ? 1.0 : 0.0;
	}

}


QuantovnikWidget::QuantovnikWidget() {
	Quantovnik *module = new Quantovnik();
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/Quantovnik.svg")));
		addChild(panel);
	}
	//Standard screws
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	//Knobs
	addParam(createParam<RoundBlackKnob>(Vec(26, 45),	module, Quantovnik::OCTAVE_PARAM,	-4.5, 4.5, 0.0));
	addParam(createParam<RoundBlackKnob>(Vec(47, 113),	module, Quantovnik::COURSE_PARAM,	-1, 1, 0.0));

	//Switches
	addParam(createParam<Koralfx_Switch_Red>(Vec(18, 253),	module, Quantovnik::CV_IN_PARAM,	0.0, 1.0, 1.0));
	addParam(createParam<Koralfx_Switch_Red>(Vec(58, 253),	module, Quantovnik::CV_OUT_PARAM,	0.0, 1.0, 0.0));

	//Inputs
	addInput(createInput<PJ301MPort>	(Vec(13, 298),	module, Quantovnik::CV_PITCH_INPUT));
	addInput(createInput<PJ301MPort>	(Vec(10, 121),	module, Quantovnik::CV_COURSE_INPUT));

	//Outputs
	addOutput(createOutput<PJ301MPort>(Vec(52, 298), module, Quantovnik::CV_PITCH_OUTPUT));

	//Note lights - set base position
	float xPos		=   9;
	float yPos1		= 204;
	float yPos2		= 187;
	float xDelta	=  11;
	
	float lightPos [12]	= {0.0, 0.5, 1.0, 1.5, 2.0, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0};
	float rowPos [12]	= {yPos1, yPos2, yPos1, yPos2, yPos1, yPos1, yPos2, yPos1, yPos2, yPos1, yPos2, yPos1};

	for (int i = 0; i < 12; i++) {
		addChild(createLight<SmallLight<RedLight>>(Vec(xPos + lightPos[i] * xDelta, rowPos[i]), module, Quantovnik::NOTE_LIGHT +  i));
	}

}
