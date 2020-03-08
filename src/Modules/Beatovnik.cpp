#include "../KoralFX.hpp"
#include "../KoralfxComponents.hpp"
#include <string>



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
		NUM_LIGHTS 			= NOTE_LIGHT + 9
	};






///////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////

    NVGcolor colorDisplay 	= nvgRGB(0xff, 0xcc, 0x00);

    bool 	inMemory 		= false;
    
    bool 	beatLock 		= false;
    float 	beatTime 		= 0;
    int 	beatCount 		= 0;
    int 	beatCountMemory = 0;
    float 	beatOld 		= 0;

    int 	stepper 		= 0;
    bool 	stepperInc 		= false;

    float 	gateWidth2xMul	= 0;
    float 	gateWidth4xMul	= 0;
    float 	gateWidth2xDiv	= 0;
    float 	gateWidth4xDiv	= 0;

    bool 	gateDec2xMul 	= false;
    bool 	gateDec4xMul 	= false;
    bool 	gateDec2xDiv 	= false;
    bool 	gateDec4xDiv 	= false;

	std::string tempo;
	dsp::SchmittTrigger clockTrigger;
	dsp::PulseGenerator LightPulseGenerator;










	Beatovnik() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(WIDTH_PARAM,  0.01f, 0.99f, 0.5f);
		configParam(TIME_PARAM,   0,       7,   2);
		configParam(TIME_T_PARAM, 0,       1,   0);


	}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Step ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


	void process(const ProcessArgs &args) override {
		float deltaTime = APP->engine->getSampleTime();
		if (inputs[CLOCK_INPUT].isConnected()) {
			float clockInput = inputs[CLOCK_INPUT].getVoltage();
			//A rising slope
			if ((clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())) && !inMemory) {
				beatCount ++;
				stepper 					= 0;
				lights[CLOCK_LIGHT].value 	= 1;
				inMemory 					= true;
				LightPulseGenerator.trigger(0.1);

				//Set divide with tempo knob 
				int choice 	= round(params[TIME_PARAM].getValue());
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

				float divide 	= ratio * ((params[TIME_T_PARAM].getValue()) ? 0.333 : 1);
				float CV 		= (log(beatOld * 1000 * divide) /log(10 / 1e-3));

				outputs[CV_OUTPUT].value 	= CV * 10;

				for (int i = 0; i < 9; i++) {
					lights[NOTE_LIGHT + i].value = ((choice == i) ? 1 : 0);
				}

				//BPM is locked
				if (beatCount == 2) {
					lights[CLOCK_LOCK_LIGHT].value = 1;
					beatLock = true;
					beatOld = beatTime;
					tempo = std::to_string((int)round(60/beatOld));
					colorDisplay 	= nvgRGB(0xff, 0xcc, 0x00);
				}

				//BPM lost
				if (beatCount > 2) {
					if (fabs(beatOld - beatTime) > 0.005) {
						beatLock = false;
						beatCount = 0;
						lights[CLOCK_LOCK_LIGHT].value = 0;
						stepper = 0;
						stepperInc = false;
						for (int i = 0; i < 13; i++) {
							lights[CLOCK_LIGHT + i].value = 0;
							tempo = "---" ;
							colorDisplay 	= nvgRGB(0xff, 0x00, 0x00);
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
				int bM = round((beatOld / division) * 1000000);
				int bT = round(beatTime * 1000000);
				
				if ( bT % bM <= deltaTime * 1000000) {
					stepperInc = true;
				}

				//MULTIPLY 4X
				int noteRate = 4;
				if (stepperInc) {
					if ( stepper % noteRate == 0) {
						lights[BEAT4X_MUL_LIGHT].value 			= 1;
						outputs[BEAT4X_MUL_OUTPUT].value 		= 10;
						gateDec4xMul 							= true;
					}
				}

				//led off
				if (gateDec4xMul) gateWidth4xMul -= 1;
				if (gateWidth4xMul <= 0 ) {
					gateDec4xMul 	= false;
					gateWidth4xMul 	= params[WIDTH_PARAM].getValue() * (beatOld / deltaTime / noteRate);
					lights[BEAT4X_MUL_LIGHT].value 				= 0;
					outputs[BEAT4X_MUL_OUTPUT].value 			= 0;
				}

				//MULTIPLY 2X
				noteRate = 8;
				if (stepperInc) {
					if ( stepper % noteRate == 0) {
						lights[BEAT2X_MUL_LIGHT].value 			= 1;
						outputs[BEAT2X_MUL_OUTPUT].value 		= 10;
						gateDec2xMul 							= true;
					}
				}

				//led off
				if (gateDec2xMul) gateWidth2xMul -= 1;
				if (gateWidth2xMul <= 0 ) {
					gateDec2xMul 	= false;
					gateWidth2xMul 	= params[WIDTH_PARAM].getValue() * (beatOld / deltaTime / noteRate) * 4;
					lights[BEAT2X_MUL_LIGHT].value 				= 0;
					outputs[BEAT2X_MUL_OUTPUT].value 			= 0;
				}

				if (beatCountMemory != beatCount) {
					//DIV 2X
					if (stepperInc) {
						if ( beatCount % 2 == 0) {
							lights[BEAT2X_DIV_LIGHT].value 		= 1;
							outputs[BEAT2X_DIV_OUTPUT].value 	= 10;
							gateDec2xDiv 						= true;
						}
					}

					//DIV 4X
					if (stepperInc) {
						if ( beatCount % 4 == 0) {
							lights[BEAT4X_DIV_LIGHT].value 		= 1;
							outputs[BEAT4X_DIV_OUTPUT].value 	= 10;
							gateDec4xDiv 						= true;
						}
					}

				}

				//DIV 2X led off
				if (gateDec2xDiv) gateWidth2xDiv 				-= 1;
				if (gateWidth2xDiv <= 0 ) {
					gateDec2xDiv = false;
					gateWidth2xDiv = params[WIDTH_PARAM].getValue() * (beatOld / deltaTime / noteRate) * 16;
					lights[BEAT2X_DIV_LIGHT].value 				= 0;
					outputs[BEAT2X_DIV_OUTPUT].value 			= 0;
				}

				//DIV 4X led off
				if (gateDec4xDiv) gateWidth4xDiv 				-= 1;
				if (gateWidth4xDiv <= 0 ) {
					gateDec4xDiv 								= false;
					gateWidth4xDiv = params[WIDTH_PARAM].getValue() * (beatOld / deltaTime / noteRate) * 32;
					lights[BEAT4X_DIV_LIGHT].value 				= 0;
					outputs[BEAT4X_DIV_OUTPUT].value 			= 0;
				}

				//Next step of stepper
				if (stepperInc) {
					stepper++;
					stepperInc = false;
				}

			} //end of beatLock routine



			beatTime += deltaTime;

			//when beat is lost
			if (beatTime > 2 ) {
				beatLock = false;
				beatCount = 0;
				lights[CLOCK_LOCK_LIGHT].value = 0;
				stepper = 0;
				stepperInc = false;
				tempo = "---" ;
				colorDisplay 	= nvgRGB(0xff, 0x00, 0x00);
				for (int i = 0; i < 13; i++) {
					lights[CLOCK_LIGHT + i].value = 0;
				}
			}
			beatCountMemory = beatCount;
			} else {
				beatLock = false;
				beatCount = 0;
				tempo = "OFF" ;
				colorDisplay 	= nvgRGB(0x00, 0xcc, 0xff);
				for (int i = 0; i < 13; i++) {
					lights[CLOCK_LIGHT + i].value = 0;
			}

		} //end of input active routine

		bool pulse = LightPulseGenerator.process(1.0 / APP->engine->getSampleTime());
		if (pulse == 0) lights[CLOCK_LIGHT].value = 0;
		
	}

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct BeatovnikWidget : ModuleWidget {

    BeatovnikWidget(Beatovnik *module) {
        setModule(module);
		box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Beatovnik-Dark.svg")));

		///////////////////////////////////////////////////////////////////////////////
		// Displays
		///////////////////////////////////////////////////////////////////////////////

 	if (module) {
		Seg3DisplayWidget *display = new Seg3DisplayWidget();
		display->box.pos = Vec(6,290);
		display->box.size = Vec(45, 20);
		display->value = &module->tempo;
		display->colorDisplay = &module->colorDisplay;
		addChild(display);
 	}

 	
		//Knobs
		addParam(createParam<Koralfx_RoundBlackKnob>(Vec(50.5, 248.5), module, Beatovnik::WIDTH_PARAM));
		addParam(createParam<Koralfx_StepRoundLargeBlackKnob>(Vec(6, 73), module, Beatovnik::TIME_PARAM));

		//Buttons
		addParam(createParam<Koralfx_Switch_Blue>(Vec(54,60), module, Beatovnik::TIME_T_PARAM));


		//Inputs
		addInput(createInput<PJ301MPort>(Vec(13, 251), module, Beatovnik::CLOCK_INPUT));

		//Outputs
		addOutput(createOutput<PJ301MPort>(Vec(52, 88), module, Beatovnik::CV_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(14, 146), module, Beatovnik::BEAT2X_MUL_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(52, 146), module, Beatovnik::BEAT4X_MUL_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(14, 206), module, Beatovnik::BEAT2X_DIV_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(52, 206), module, Beatovnik::BEAT4X_DIV_OUTPUT));

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
};



////////////////////////////////////////////////////////////////////////////////////////////////////

Model *modelBeatovnik = createModel<Beatovnik, BeatovnikWidget>("Beatovnik");
