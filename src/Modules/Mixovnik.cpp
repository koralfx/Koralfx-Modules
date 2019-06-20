#include "../Koralfx-Modules.hpp"
#include "../KoralfxComponents.hpp"

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
		PAN_PARAM  			= LINK_PARAM + 8,
		AUX1_PARAM 			= PAN_PARAM + 16,
		AUX2_PARAM 			= AUX1_PARAM + 16,
		VOLUME_PARAM 		= AUX2_PARAM + 16,
		MUTE_PARAM 			= VOLUME_PARAM + 16,
		SOLO_PARAM 			= MUTE_PARAM + 16,
		NUM_PARAMS 			= SOLO_PARAM + 16
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
		NUM_INPUTS 			= STRIPE_CV_VOL_INPUT + 16
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
		NUM_LIGHTS 			= SIGNAL_LIGHT_NORMAL + 32
	};


    float antiPop [16]		= {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float antiPopCurrentSpeed [16]		= {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float antiPopMixLeft	= 0.0f;
    float antiPopMixRight	= 0.0f;
    float antiPopAux1		= 0.0f;
    float antiPopAux2		= 0.0f;
    float antiPopSpeed		= 0.0005f;


	Mixovnik() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(AUX1_VOLUME, 0.0f, 1.0f, 0.9f );
		configParam(AUX2_VOLUME, 0.0f, 1.0f, 0.9f );
		configParam(MIX_L_VOLUME, 0.0f, 1.0f, 0.9f );
		configParam(MIX_R_VOLUME, 0.0f, 1.0f, 0.9f );
		configParam(MIX_LINK, 0, 1, 0 );
		configParam(AUX1_MUTE, 0, 1, 0 );
		configParam(AUX2_MUTE, 0, 1, 0 );
		configParam(MIX_L_MUTE, 0, 1, 0 );
		configParam(MIX_R_MUTE, 0, 1, 0 );

		for (int i = 0; i < 16; i++) {
			configParam(PAN_PARAM + i,  -1.0f, 1.0f, 0.0f);
			configParam(AUX1_PARAM + i, 0.0f, 1.0f, 0.0f );
			configParam(AUX2_PARAM + i, 0.0f, 1.0f, 0.0f );
			configParam(VOLUME_PARAM + i, 0.0f, 1.0f, 0.9f );
			configParam(MUTE_PARAM + i, 0, 1, 0 );
			configParam(SOLO_PARAM + i, 0, 1, 0 );
			if (i%2 == 0) {
				configParam(LINK_PARAM + (i/2), 0, 1, 0 );
			}
		}

	}

	void process(const ProcessArgs &args) override {
		
		float SUM_L = 0.0;
		float SUM_R = 0.0;

		float SUM_AUX1_L = 0.0;
		float SUM_AUX1_R = 0.0;
		float SUM_AUX2_L = 0.0;
		float SUM_AUX2_R = 0.0;

		float cvVolumeRatio[16];
		

		//First Solo test
		bool soloTest = false;
		int step = 0;
		do  {
			if (params[SOLO_PARAM + step].getValue() == 1) soloTest = true;
			step++;
		} while (!soloTest && step < 16);



		//Anti Pop Mix
		antiPopMixLeft -= (params[MIX_L_MUTE].getValue() ==1) ? antiPopSpeed : -antiPopSpeed;
		if (antiPopMixLeft <0  )  antiPopMixLeft = 0;
		if (antiPopMixLeft >1  )  antiPopMixLeft = 1;
		
		antiPopMixRight -= (params[MIX_R_MUTE].getValue() ==1) ? antiPopSpeed : -antiPopSpeed;
		if (antiPopMixRight <0  )  antiPopMixRight = 0;
		if (antiPopMixRight >1  )  antiPopMixRight = 1;

		//Anti Pop Auxes
		antiPopAux1 -= (params[AUX1_MUTE].getValue() ==1) ? antiPopSpeed : -antiPopSpeed;
		if (antiPopAux1 <0  )  antiPopAux1 = 0;
		if (antiPopAux1 >1  )  antiPopAux1 = 1;
		
		antiPopAux2 -= (params[AUX2_MUTE].getValue() ==1) ? antiPopSpeed : -antiPopSpeed;
		if (antiPopAux2 <0  )  antiPopAux2 = 0;
		if (antiPopAux2 >1  )  antiPopAux2 = 1;



		// Main loop
		for (int i = 0; i < 16; i++) {

			//Get signal
			float INPUT_SIGNAL = 0;
			cvVolumeRatio[i] = 1;



			if (inputs[STRIPE_CV_VOL_INPUT + i].isConnected()) cvVolumeRatio[i] = inputs[STRIPE_CV_VOL_INPUT + i].getVoltage()/10;


			bool nieparzystyStripe = (i%2 == 0) ? true : false;
			bool linkActive = (params[LINK_PARAM + ((i-1)/2)].getValue() == 1) ? true : false;
			
			//stripes 1,3,5,7,9,11,13,15
			if (nieparzystyStripe) {
				INPUT_SIGNAL = inputs[STRIPE_INPUT + i].getVoltage() * cvVolumeRatio[i] * params[VOLUME_PARAM + i].getValue();
			//stripes 2,4,6,8,10,12,14,16
			} else {
				//link with left stripes?
				if (linkActive) {
					INPUT_SIGNAL = inputs[STRIPE_INPUT + i].getVoltage() * cvVolumeRatio[i-1] * params[VOLUME_PARAM + (i - 1)].getValue() ;
				} else {
					INPUT_SIGNAL = inputs[STRIPE_INPUT + i].getVoltage() * cvVolumeRatio[i] * params[VOLUME_PARAM + i].getValue();
				}
			}


			//MUTE SOLO test for Anti Pop

			//SOLO
			if (soloTest) {
				if (nieparzystyStripe) {
					antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i].getValue() == 0 && params[SOLO_PARAM + i].getValue() == 1)
						? +antiPopSpeed : -antiPopSpeed;
				} else {
					if (linkActive) {
						antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i -1].getValue() == 0 && params[SOLO_PARAM + i -1].getValue() == 1)
							? +antiPopSpeed : -antiPopSpeed;
					} else {
						antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i].getValue() == 0 && params[SOLO_PARAM + i].getValue() == 1)
							? +antiPopSpeed : -antiPopSpeed;
					}
				}
			//NO SOLO	
			} else {
				if (nieparzystyStripe) {
					antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i].getValue() == 1) ? -antiPopSpeed : +antiPopSpeed;
				} else {
					if (linkActive) {
						antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i -1].getValue() == 1) ? -antiPopSpeed : +antiPopSpeed;
					} else {
						antiPopCurrentSpeed[i] = (params[MUTE_PARAM + i].getValue() == 1) ? -antiPopSpeed : +antiPopSpeed;
					}
				}
			}


			//Anti Pop final

			if (!inputs[STRIPE_INPUT + i].isConnected()) antiPopCurrentSpeed[i] = -antiPopSpeed;

			antiPop[i] += antiPopCurrentSpeed[i]; 
			if (antiPop[i] <0  )  antiPop[i] = 0;
			if (antiPop[i] >1  )  antiPop[i] = 1;
			INPUT_SIGNAL *= antiPop[i];

			//Constant-power panning
			float KNOB_PAN_POS = params[PAN_PARAM + i].getValue() + (inputs[STRIPE_CV_PAN_INPUT + i].getVoltage() / 5);

			//Anti invert phase
			if (KNOB_PAN_POS < -1) KNOB_PAN_POS = -1;
			if (KNOB_PAN_POS > 1) KNOB_PAN_POS = 1;

			double angle = KNOB_PAN_POS * PI_4;
			float GAIN_SIGNAL_L = (float) (SQRT2_2 * (cos(angle) - sin(angle)));
			float GAIN_SIGNAL_R = (float) (SQRT2_2 * (cos(angle) + sin(angle)));

			
			SUM_L += INPUT_SIGNAL * GAIN_SIGNAL_L;
			SUM_R += INPUT_SIGNAL * GAIN_SIGNAL_R;




			//AUX1 stripe send
			float KNOB_AUX1_POS = params[AUX1_PARAM + i].getValue(); 
			SUM_AUX1_L += KNOB_AUX1_POS * INPUT_SIGNAL * GAIN_SIGNAL_L ;
			SUM_AUX1_R += KNOB_AUX1_POS * INPUT_SIGNAL * GAIN_SIGNAL_R;

			//AUX2 stripe send
			float KNOB_AUX2_POS = params[AUX2_PARAM + i].getValue();
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
		outputs[AUX1_OUTPUT_L].setVoltage(SUM_AUX1_L);
		outputs[AUX1_OUTPUT_R].setVoltage(SUM_AUX1_R);

		outputs[AUX2_OUTPUT_L].setVoltage(SUM_AUX2_L);
		outputs[AUX2_OUTPUT_R].setVoltage(SUM_AUX2_R);

		//AUX1 and AUX2 returns
		float AUX1_SUM_L =
			inputs[AUX1_INPUT_L].getVoltage() * params[AUX1_VOLUME].getValue() * antiPopAux1;

		float AUX1_SUM_R =
			inputs[AUX1_INPUT_R].getVoltage() * params[AUX1_VOLUME].getValue() * antiPopAux1;
		
		float AUX2_SUM_L =
			inputs[AUX2_INPUT_L].getVoltage() * params[AUX2_VOLUME].getValue() * antiPopAux2;
			
		float AUX2_SUM_R =
			inputs[AUX2_INPUT_R].getVoltage() * params[AUX2_VOLUME].getValue() * antiPopAux2;
		
		SUM_L += AUX1_SUM_L + AUX2_SUM_L;
		SUM_R += AUX1_SUM_R + AUX2_SUM_R;


		//Exteranal mix
		SUM_L +=  inputs[STEREO_INPUT_L].getVoltage();
		SUM_R +=  inputs[STEREO_INPUT_R].getVoltage();

		//Mix sliders
		SUM_L *=  params[MIX_L_VOLUME].getValue();
		SUM_R *= ((params[MIX_LINK].getValue() ==  0) ? params[MIX_R_VOLUME].getValue() : params[MIX_L_VOLUME].getValue());

		//Final mix with mute switches
		SUM_L *= antiPopMixLeft;

		if (params[MIX_LINK].getValue() == 1) {
			SUM_R *= antiPopMixLeft;
		} else {
			SUM_R *= antiPopMixRight;
		}


		//Final out
		outputs[STEREO_OUTPUT_L].setVoltage(SUM_L);
		outputs[STEREO_OUTPUT_R].setVoltage(SUM_R);

		//Mix lights
		lights[MIX_LIGHT_L].value = (fabs(SUM_L) > 5) ? 1.0 : 0.0;
		lights[MIX_LIGHT_R].value = (fabs(SUM_R) > 5) ? 1.0 : 0.0;

		lights[AUX1_LIGHT].value = (fabs(AUX1_SUM_L) > 5 || fabs(AUX1_SUM_R)  > 5) ? 1.0 : 0.0;
		lights[AUX2_LIGHT].value = (fabs(AUX2_SUM_L) > 5 || fabs(AUX2_SUM_R)  > 5) ? 1.0 : 0.0;
	
	}

};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct MixovnikWidget : ModuleWidget {

    MixovnikWidget(Mixovnik *module) {
        setModule(module);


	box.size = Vec(58 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Mixovnik-Dark.svg")));
	

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
			addChild(createLight<SmallLight<GreenRedLight>>	(Vec(xPos + 27 + i*xDelta,	yPos + 110),
				module, Mixovnik::SIGNAL_LIGHT_NORMAL + i*2));
			
			addParam(createParam<Koralfx_RoundBlackKnob>(Vec(xPos - 0.5 + i*xDelta,	yPos -   0.5),
				module, Mixovnik::AUX1_PARAM + i));
			
			addParam(createParam<Koralfx_RoundBlackKnob>(Vec(xPos - 0.5 + i*xDelta,	yPos +  36.5),
				module, Mixovnik::AUX2_PARAM + i));
			
			addParam(createParam<Koralfx_RoundBlackKnob>(Vec(xPos - 0.5 + i*xDelta,	yPos +  75.5),
				module, Mixovnik::PAN_PARAM + i));
			
			addParam(createParam<Koralfx_SliderPot>(Vec(xPos + 3 + i*xDelta,	yPos + 110),
				module, Mixovnik::VOLUME_PARAM + i));
			
			addParam(createParam<Koralfx_Switch_Red>(Vec(xPos + 8 + i*xDelta,	yPos + 228),
				module, Mixovnik::MUTE_PARAM + i));
			
			if (i%2 == 0) addParam(createParam<Koralfx_Switch_Blue>(Vec(xPos + 8 + (i+0.5)*xDelta,	yPos + 228),
				module, Mixovnik::LINK_PARAM + (i/2)));
			
			addInput(createInput<PJ301MPort>(Vec(xPos + 3 + i*xDelta,	yPos + 266),
				module, Mixovnik::STRIPE_INPUT + i));
			
			addInput(createInput<PJ301MPort>(Vec(xPos + 3 + i*xDelta,	yPos + 292),
				module, Mixovnik::STRIPE_CV_PAN_INPUT + i));
			
			addInput(createInput<PJ301MPort>(Vec(xPos + 3 + i*xDelta,	yPos + 318),
				module, Mixovnik::STRIPE_CV_VOL_INPUT + i));
			
			addParam(createParam<Koralfx_Switch_Green>(Vec(xPos + 8 + i*xDelta,	yPos + 245),
				module, Mixovnik::SOLO_PARAM + i));
		}

		//Final volume sliders
		addParam(createParam<Koralfx_SliderPot>(Vec(xPos - 2 + 17*xDelta, yPos + 110),
			module, Mixovnik::AUX1_VOLUME));

		addParam(createParam<Koralfx_SliderPot>(Vec(xPos - 2 + 18*xDelta, yPos + 110),
			module, Mixovnik::AUX2_VOLUME));

		addParam(createParam<Koralfx_SliderPot>(Vec(xPos + 3 + 19*xDelta, yPos + 110),
			module, Mixovnik::MIX_L_VOLUME));

		addParam(createParam<Koralfx_SliderPot>(Vec(xPos + 1 + 20*xDelta, yPos + 110),
			module, Mixovnik::MIX_R_VOLUME));

		//Final mute switches
		addParam(createParam<Koralfx_Switch_Red>(Vec(xPos + 3 + 17*xDelta, yPos + 227),
			module, Mixovnik::AUX1_MUTE));

		addParam(createParam<Koralfx_Switch_Red>(Vec(xPos + 3 + 18*xDelta, yPos + 227),
			module, Mixovnik::AUX2_MUTE));

		addParam(createParam<Koralfx_Switch_Red>(Vec(xPos + 8 + 19*xDelta, yPos + 227),
			module, Mixovnik::MIX_L_MUTE));

		addParam(createParam<Koralfx_Switch_Red>(Vec(xPos + 6 + 20*xDelta, yPos + 227),
			module, Mixovnik::MIX_R_MUTE));

		//Stereo mix link switch
		addParam(createParam<Koralfx_Switch_Blue>(Vec(xPos + 7 + 19.5*xDelta, yPos + 227),
			module, Mixovnik::MIX_LINK));

		//Final mix lights
		addChild(createLight<SmallLight<RedLight>>	(Vec(703,120), module, Mixovnik::AUX1_LIGHT));
		addChild(createLight<SmallLight<RedLight>>	(Vec(743,120), module, Mixovnik::AUX2_LIGHT));

		addChild(createLight<SmallLight<RedLight>>	(Vec(788,120), module, Mixovnik::MIX_LIGHT_L));
		addChild(createLight<SmallLight<RedLight>>	(Vec(826,120), module, Mixovnik::MIX_LIGHT_R));
		
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////

Model *modelMixovnik = createModel<Mixovnik, MixovnikWidget>("Mixovnik");
