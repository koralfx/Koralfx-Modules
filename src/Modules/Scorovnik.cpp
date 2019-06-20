#include "../Koralfx-Modules.hpp"
#include "../KoralfxComponents.hpp"


#include <string>


struct Scorovnik : Module {
	enum ParamIds {
		SLIDER_NOTE_LENGTH_PARAM,
		SLIDER_NOTE_PITCH_PARAM		= SLIDER_NOTE_LENGTH_PARAM + 32,
		SWITCH_NOTE_DOT_TRI_PARAM	= SLIDER_NOTE_PITCH_PARAM + 32,
		SWITCH_NOTE_PAUSE_ACC_PARAM	= SWITCH_NOTE_DOT_TRI_PARAM + 32,
		SWITCH_NOTE_STACCATO_PARAM	= SWITCH_NOTE_PAUSE_ACC_PARAM + 32,
		LED_BUTTON_PARAM			= SWITCH_NOTE_STACCATO_PARAM + 32,
		TEMPO_PARAM					= LED_BUTTON_PARAM + 32,
		TRANSPOSE_PARAM,
		SWITCH_MODE_PARAM,
		KNOB_LOOP_NUMBER_PARAM,
		SWITCH_START_PARAM,
		SWITCH_STOP_PARAM,
		SWITCH_RESET_PARAM,
		SWITCH_MONITOR_PARAM,
		SWITCH_UNI_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		EXT_CLK_INPUT,
		START_INPUT,
		STOP_INPUT = START_INPUT + 4,
		RESET_INPUT = STOP_INPUT + 4,
		CV_TRANSPOSE_INPUT = RESET_INPUT + 4,		
		NUM_INPUTS 
	};
	enum OutputIds {
		CLK4_OUTPUT,
		CLK16_OUTPUT,
		GATE_OUTPUT,
		PITCH_OUTPUT = GATE_OUTPUT + 4,
		ACCENT_OUTPUT = PITCH_OUTPUT + 4,
		LAST_OUTPUT = ACCENT_OUTPUT + 4,	
		NUM_OUTPUTS  = LAST_OUTPUT + 4
	};
	enum LightIds {
		GROUP_LIGHT,
		SELECTED_STEP_LIGHT = GROUP_LIGHT + 8,
		GROUP_LENGTH_LIGHT = SELECTED_STEP_LIGHT + 32,
		NUM_LIGHTS = GROUP_LENGTH_LIGHT + 32
	};



///////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////

    int panelStyle 			= 0;
    NVGcolor colorDisplay 	= nvgRGB(0x56, 0xdc, 0xff);


    int tempoBPM 			= 0;
    int tempoBPMOld 		= 0;

	int globalTranspose 	= 0;
	int globalTransposeOld 	= -1;

	int loopLimit 			= 0;
	int loopLimitOld 		= -1;

    std::string tempo;
    std::string transpose;
    std::string loopNumber;

    int oldMode 			=-1;
    float externalTime 		= 0;

    bool initStep = true;

    std::string stepNumber;
    std::string notePitchDisplay;

    float sliderValue [32];
	float sliderValueOld [32];

	std::string noteNames [12] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "};

	float clockPhase;
	int clock16;
	int clockBPM;
	int groupActive [4]		= {0,0,0,0};
	int groupPlay [4]		= {0,0,0,0};
	int groupStep [4] 		= {0,0,0,0};

	int mode1Seq  			= 0;
	int mode2SeqA 			= 0;
	int mode2SeqC 			= 16;
	int mode3SeqA 			= 0;
	int mode3SeqB 			= 8;
	int mode3SeqC 			= 16;
	int mode3SeqD 			= 24;


	int mode1SeqCounter  	= 0;
	int mode2SeqCounterA 	= 0;
	int mode2SeqCounterC 	= 0;
	int mode3SeqCounterA 	= 0;
	int mode3SeqCounterB 	= 0;
	int mode3SeqCounterC 	= 0;
	int mode3SeqCounterD 	= 0;

	int mode1SeqEnd  		= 32;
	int mode2SeqEndA 		= 16;
	int mode2SeqEndC 		= 16;
	int mode3SeqEndA 		= 8;
	int mode3SeqEndB 		= 8;
	int mode3SeqEndC 		= 8;
	int mode3SeqEndD 		= 8;

	int mode1LoopCounter  	= 0;
	int mode2LoopCounterA 	= 0;
	int mode2LoopCounterC 	= 0;
	int mode3LoopCounterA 	= 0;
	int mode3LoopCounterB 	= 0;
	int mode3LoopCounterC 	= 0;
	int mode3LoopCounterD 	= 0;

///////////////////////////////////////////////////////////////////////////////
// Schmitt Triggers
///////////////////////////////////////////////////////////////////////////////

    dsp::SchmittTrigger extClockTrigger;

    dsp::SchmittTrigger startTriggerA;
    dsp::SchmittTrigger startTriggerB;
    dsp::SchmittTrigger startTriggerC;
    dsp::SchmittTrigger startTriggerD;

    dsp::SchmittTrigger stopTriggerA;
    dsp::SchmittTrigger stopTriggerB;
    dsp::SchmittTrigger stopTriggerC;
    dsp::SchmittTrigger stopTriggerD;

    dsp::SchmittTrigger resetTriggerA;
    dsp::SchmittTrigger resetTriggerB;
    dsp::SchmittTrigger resetTriggerC;
    dsp::SchmittTrigger resetTriggerD;


    dsp::SchmittTrigger globalStartTriger;
    dsp::SchmittTrigger globalStopTriger;
    dsp::SchmittTrigger globalResetTriger;

    dsp::SchmittTrigger touchTriger [32];


///////////////////////////////////////////////////////////////////////////////
// Pulse Genearators
///////////////////////////////////////////////////////////////////////////////

    dsp::PulseGenerator gatePulseGeneratorA;
    dsp::PulseGenerator gatePulseGeneratorB;
    dsp::PulseGenerator gatePulseGeneratorC;
    dsp::PulseGenerator gatePulseGeneratorD;

    dsp::PulseGenerator pitchPulseGeneratorA;
    dsp::PulseGenerator pitchPulseGeneratorB;
    dsp::PulseGenerator pitchPulseGeneratorC;
    dsp::PulseGenerator pitchPulseGeneratorD;

    dsp::PulseGenerator lastPulseGeneratorA;
    dsp::PulseGenerator lastPulseGeneratorB;
    dsp::PulseGenerator lastPulseGeneratorC;
    dsp::PulseGenerator lastPulseGeneratorD;


	dsp::PulseGenerator clockPulseGenerator;
	dsp::PulseGenerator bpmPulseGenerator;

	dsp::PulseGenerator notePulseGeneratorA;
	dsp::PulseGenerator notePulseGeneratorB;
	dsp::PulseGenerator notePulseGeneratorC;
	dsp::PulseGenerator notePulseGeneratorD;



	Scorovnik() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		for (int i = 0; i < 32; i++) {
			configParam(SLIDER_NOTE_LENGTH_PARAM + i, 0, 4, 0);
			configParam(SLIDER_NOTE_PITCH_PARAM + i, -24, 24, 0);
			configParam(SWITCH_NOTE_DOT_TRI_PARAM + i, 0, 2, 0);
			configParam(SWITCH_NOTE_PAUSE_ACC_PARAM + i, 0, 2, 1);
			configParam(SWITCH_NOTE_STACCATO_PARAM + i, 0, 1, 0);
		}
		configParam(LED_BUTTON_PARAM, 0, 1, 0);
		configParam(TEMPO_PARAM, 40, 250, 120);
		configParam(TRANSPOSE_PARAM, -24, 24, 0);
		configParam(SWITCH_MODE_PARAM, 0, 2, 0);
		configParam(KNOB_LOOP_NUMBER_PARAM, 0, 128, 0);
		configParam(SWITCH_START_PARAM, 0, 1, 0);
		configParam(SWITCH_STOP_PARAM, 0, 1, 0);
		configParam(SWITCH_RESET_PARAM, 0, 1, 0);
		configParam(SWITCH_MONITOR_PARAM, 0, 1, 1);
		configParam(SWITCH_UNI_PARAM, 0, 1, 0);
	}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Step ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void process(const ProcessArgs &args) override {

	if (initStep)  {
		for (int i = 0; i <32; i++) {
			sliderValueOld [i] = floor(params[SLIDER_NOTE_PITCH_PARAM + i].getValue());
			sliderValue [i] = sliderValueOld [i];
		}
		stepNumber 			= "--";
		notePitchDisplay 	= "--";
	}
	
	float deltaTime 		= APP->engine->getSampleRate();
	bool clockGateOn 		= false;
	float noteTime 			= 0;
	int uniOut 				= 0 + params[SWITCH_UNI_PARAM].getValue() * 5 ;
	int mode 				= params[SWITCH_MODE_PARAM].getValue();
	globalTranspose 		= round(params[TRANSPOSE_PARAM].getValue() + (inputs[CV_TRANSPOSE_INPUT].getVoltage() * 12));

	//display transpose only if change
	if (globalTranspose != globalTransposeOld) {
		if (fabs(globalTranspose) < 100) {
			transpose = (globalTranspose > 0) ? "+" + std::to_string(globalTranspose) : std::to_string(globalTranspose);
		} else {
			transpose = "---";
		}
		globalTransposeOld = globalTranspose;
	}

	//if zero loop endless (1000000)
	loopLimit = (params[KNOB_LOOP_NUMBER_PARAM].getValue() == 0) ? 100000 : params[KNOB_LOOP_NUMBER_PARAM].getValue();
	if (loopLimit != loopLimitOld) {
		loopNumber = (loopLimit == 100000) ? "+++": std::to_string(loopLimit);
		loopLimitOld = loopLimit;
	}
	
	switch(mode) {
		case 0:
		groupActive [0] = 1; groupActive [1] = 0; groupActive [2] = 0; groupActive [3] = 0;
		break;
		
		case 1:
		groupActive [0] = 1; groupActive [1] = 0; groupActive [2] = 1; groupActive [3] = 0;
		break;
		
		case 2:
		groupActive [0] = 1; groupActive [1] = 1; groupActive [2] = 1; groupActive [3] = 1;
		break;
	}

	//Set Group length
	int touchSelected = -1;
	for (int i = 0; i <32; i++) {
		if (touchTriger[i].process(params[LED_BUTTON_PARAM + i].getValue())) {
			touchSelected = i;
		}
	}
	if (touchSelected >= 0) {
		switch(mode) {

			case 0:
			mode1SeqEnd  = touchSelected + 1;
			break;
			
			case 1:
			if (touchSelected <16) {
				mode2SeqEndA  = touchSelected + 1;
			} else {
				mode2SeqEndC  = touchSelected + 1 - 16;
			}
			break;

			case 2:
			if (touchSelected < 8) mode3SeqEndA  = touchSelected + 1;
			if (touchSelected >= 24) mode3SeqEndD  = touchSelected + 1 - 24;
			if (touchSelected >= 8 && touchSelected < 24) {
				if (touchSelected < 16) {
					mode3SeqEndB  = touchSelected + 1 - 8;
				} else {
					mode3SeqEndC  = touchSelected + 1 - 16;
				}
			}			
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// External triggers
	///////////////////////////////////////////////////////////////////////////////

	//start triggers
	if (startTriggerA.process(inputs[START_INPUT + 0].getVoltage())) {
		groupPlay[0] = 1;
		mode1LoopCounter 		= loopLimit;
		mode2LoopCounterA 		= loopLimit;
		mode3LoopCounterA 		= loopLimit;
	}
	if (startTriggerB.process(inputs[START_INPUT + 1].getVoltage()) && mode == 2) {
		groupPlay[1] 			= 1;
		mode3LoopCounterB 		= loopLimit;
	}
	if (startTriggerC.process(inputs[START_INPUT + 2].getVoltage()) && mode > 0) {
		groupPlay[2] 			= 1;
		mode2LoopCounterC 		= loopLimit;
		mode3LoopCounterC 		= loopLimit;}
	if (startTriggerD.process(inputs[START_INPUT + 3].getVoltage()) && mode == 2) {
		groupPlay[3] 			= 1;
		mode3LoopCounterD 		= loopLimit;
	}

	//stop triggers
	if (stopTriggerA.process(inputs[STOP_INPUT + 0].getVoltage())) {
		groupPlay[0] 			= 0;
		mode1SeqCounter			= 0;
		mode2SeqCounterA		= 0;
		mode3SeqCounterA		= 0;
	}
	if (stopTriggerB.process(inputs[STOP_INPUT + 1].getVoltage())) {
		groupPlay[1] 			= 0;
		mode3SeqCounterB		= 0;
	}
	if (stopTriggerC.process(inputs[STOP_INPUT + 2].getVoltage())) {
		groupPlay[2] = 0;
		mode2SeqCounterC		= 0;
		mode3SeqCounterC		= 0;
	}
	if (stopTriggerD.process(inputs[STOP_INPUT + 3].getVoltage())) {
		groupPlay[3] = 0;
		mode3SeqCounterD		= 0;
	}

	//reset triggers
	if (resetTriggerA.process(inputs[RESET_INPUT + 0].getVoltage())) {
		mode1Seq 				= -1;
		mode2SeqA 				= -1;
		mode3SeqA 				= -1;
	}
	if (resetTriggerB.process(inputs[RESET_INPUT + 1].getVoltage())) {
		mode3SeqB 				= 7;
	}
	if (resetTriggerC.process(inputs[RESET_INPUT + 2].getVoltage())) {
		mode2SeqC 				= 15;
		mode3SeqC 				= 15;
	}
	if (resetTriggerD.process(inputs[RESET_INPUT + 3].getVoltage())) {
		mode3SeqD 				= 23;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Internal triggers
	///////////////////////////////////////////////////////////////////////////////

	switch(mode) {
		case 0:
		if (globalStartTriger.process(params[SWITCH_START_PARAM].getValue())) {
			groupPlay[0] 		= 1;
			mode1LoopCounter 	= loopLimit;
		}

		if (globalStopTriger.process(params[SWITCH_STOP_PARAM].getValue())) {
			groupPlay[0] 		= 0;
			mode1SeqCounter		= 0;
			outputs[GATE_OUTPUT + 0].setVoltage(0.0f);
		}

		if (globalResetTriger.process(params[SWITCH_RESET_PARAM].getValue())) {
			mode1Seq 			= -1;
			mode1SeqCounter		= 0;
			outputs[GATE_OUTPUT + 0].setVoltage(0.0f);
		}
		break;
		
		case 1:
		if (globalStartTriger.process(params[SWITCH_START_PARAM].getValue())) {
			groupPlay[0] 		= 1;
			groupPlay[2] 		= 1;
			mode2LoopCounterA 	= loopLimit;
			mode2LoopCounterC 	= loopLimit;
		}
		if (globalStopTriger.process(params[SWITCH_STOP_PARAM].getValue())) {
			groupPlay[0] 		= 0;
			groupPlay[2] 		= 0;
			mode2SeqCounterA	= 0;
			mode2SeqCounterC	= 0;
			outputs[GATE_OUTPUT + 0].setVoltage(0);
			outputs[GATE_OUTPUT + 2].setVoltage(0);
		}
		if (globalResetTriger.process(params[SWITCH_RESET_PARAM].getValue())) {
			mode2SeqA 			= -1;
			mode2SeqC 			= 15;
			mode2SeqCounterA	= 0;
			mode2SeqCounterC	= 0;
			outputs[GATE_OUTPUT + 0].setVoltage(0.0f);
			outputs[GATE_OUTPUT + 2].setVoltage(0.0f);
		}
		break;
		
		case 2:
		if (globalStartTriger.process(params[SWITCH_START_PARAM].getValue())) {
			groupPlay[0] 		= 1;
			groupPlay[1] 		= 1;
			groupPlay[2] 		= 1;
			groupPlay[3] 		= 1;
			mode3LoopCounterA 	= loopLimit;
			mode3LoopCounterB 	= loopLimit;
			mode3LoopCounterC 	= loopLimit;
			mode3LoopCounterD 	= loopLimit;
		}
		if (globalStopTriger.process(params[SWITCH_STOP_PARAM].getValue())) {
			groupPlay[0] 		= 0;
			groupPlay[1] 		= 0;
			groupPlay[2] 		= 0;
			groupPlay[3] 		= 0;
			mode3SeqCounterA	= 0;
			mode3SeqCounterB	= 0;
			mode3SeqCounterC	= 0;
			mode3SeqCounterD	= 0;
			outputs[GATE_OUTPUT + 0].setVoltage(0);
			outputs[GATE_OUTPUT + 1].setVoltage(0);
			outputs[GATE_OUTPUT + 2].setVoltage(0);
			outputs[GATE_OUTPUT + 3].setVoltage(0);
		}

		if (globalResetTriger.process(params[SWITCH_RESET_PARAM].getValue())) {
			mode3SeqA 			= -1;
			mode3SeqB 			= 7;
			mode3SeqC 			= 15;
			mode3SeqD 			= 23;
			mode3SeqCounterA	= 0;
			mode3SeqCounterB	= 0;
			mode3SeqCounterC	= 0;
			mode3SeqCounterD	= 0;
			outputs[GATE_OUTPUT + 0].setVoltage(0.0f);
			outputs[GATE_OUTPUT + 1].setVoltage(0.0f);
			outputs[GATE_OUTPUT + 2].setVoltage(0.0f);
			outputs[GATE_OUTPUT + 3].setVoltage(0.0f);
		}
		break;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Clocks process
	///////////////////////////////////////////////////////////////////////////////

	if (inputs[EXT_CLK_INPUT].isConnected()) {
		externalTime 		+= deltaTime;
		colorDisplay 		= nvgRGB(0x00, 0xdc, 0x00);
		//external clock
		if (extClockTrigger.process(inputs[EXT_CLK_INPUT].getVoltage())) {
			clockGateOn 	= true;
			clock16 		+= 1;
			if (clock16 == 32 ) clock16 = 0;
			clockPulseGenerator.trigger(0.01);
			noteTime 		= externalTime;
	
			tempoBPM = round((60/externalTime)/4);
			//display tempo only if change
			if (tempoBPM != tempoBPMOld && tempoBPM < 251) {
				tempo 		= std::to_string(tempoBPM);
				tempoBPMOld = tempoBPM;
			}
			externalTime = 0;
		}
	} else {
		//internal clock
		tempoBPM = round(params[TEMPO_PARAM].getValue());
		colorDisplay 	= nvgRGB(0x56, 0xdc, 0xff);
		//display tempo only if change
		if (tempoBPM != tempoBPMOld) {
			tempo 			= std::to_string(tempoBPM);
			tempoBPMOld 	= tempoBPM;
		}

		if (clockPhase == 0) {
			clockPulseGenerator.trigger(0.01);
		}
		float pulseTime 	= (120.0 / tempoBPM) / 8.0;
		noteTime 			= pulseTime; 
		clockPhase 			+= deltaTime;
		if (clockPhase >= pulseTime) {
			clock16 		+= 1;
			if (clock16 == 32 ) clock16 = 0;
			clockPhase 		= 0;
			clockGateOn 	= true; 
		}
	}

	// clock next step by note length
	if (clockGateOn) {
		if (clock16 % 4 == 0) {
			clockBPM += 1;
			bpmPulseGenerator.trigger(0.1);
		}
		if (clockBPM == 8) clockBPM = 0;

		///////////////////////////////////////////////////////////////////////////////
		// Rotate steps
		///////////////////////////////////////////////////////////////////////////////

		switch(mode) {
			
			case 0:
			if (groupPlay [0] == 1) {
				if (mode1SeqCounter > 0)  			mode1SeqCounter -= 1;
				if (mode1SeqCounter == 0) 			mode1Seq += 1;
				if (mode1Seq >= mode1SeqEnd) 		{mode1Seq = 0; if (mode1LoopCounter > 0) mode1LoopCounter --;}
				if (mode1LoopCounter == 0) 			{groupPlay[0] = 0; mode1Seq = -1; }
				if (mode1LoopCounter == 1 && (mode1Seq == mode1SeqEnd -1) ) lastPulseGeneratorA.trigger(0.1);
				if (mode1LoopCounter > loopLimit) 	mode1LoopCounter = loopLimit;
			}
			break;
			
			case 1:
			if (groupPlay [0] == 1) {
				if (mode2SeqCounterA > 0)  			mode2SeqCounterA -= 1;
				if (mode2SeqCounterA == 0) 			mode2SeqA += 1;
				if (mode2SeqA >= mode2SeqEndA) 		{mode2SeqA = 0; if (mode2LoopCounterA > 0) mode2LoopCounterA --;}
				if (mode2LoopCounterA == 0) 		{groupPlay[0] = 0; mode2SeqA = -1;}
				if (mode2LoopCounterA == 1 && (mode2SeqA == mode2SeqEndA -1) ) lastPulseGeneratorA.trigger(0.1);
				if (mode2LoopCounterA > loopLimit) 	mode2LoopCounterA = loopLimit;
			}
			if (groupPlay [2] == 1) {
				if (mode2SeqCounterC > 0)  			mode2SeqCounterC -= 1;
				if (mode2SeqCounterC == 0) 			mode2SeqC += 1;
				if (mode2SeqC >= 16 + mode2SeqEndC) {mode2SeqC = 16; if (mode2LoopCounterC > 0) mode2LoopCounterC --;}
				if (mode2LoopCounterC == 0) 		{groupPlay[2] = 0; mode2SeqC = 15; }
				if (mode2LoopCounterC == 1 && (mode2SeqC == mode2SeqEndC -1 + 16) ) lastPulseGeneratorC.trigger(0.1);
				if (mode2LoopCounterC > loopLimit) 	mode2LoopCounterC = loopLimit;
			}
			break;
			
			case 2:
			if (groupPlay [0] == 1) {
				if (mode3SeqCounterA > 0)  			mode3SeqCounterA -= 1;
				if (mode3SeqCounterA == 0) 			mode3SeqA += 1;
				if (mode3SeqA >= mode3SeqEndA) 		{mode3SeqA = 0; if (mode3LoopCounterA > 0) mode3LoopCounterA --;}
				if (mode3LoopCounterA == 0) 		{groupPlay[0] = 0; mode3SeqA = -1; }
				if (mode3LoopCounterA == 1 && (mode3SeqA == mode3SeqEndA -1) ) lastPulseGeneratorA.trigger(0.1);
				if (mode3LoopCounterA > loopLimit)	 mode3LoopCounterA = loopLimit;
			}
			if (groupPlay [1] == 1) {
				if (mode3SeqCounterB > 0)  			mode3SeqCounterB -= 1;
				if (mode3SeqCounterB == 0) 			mode3SeqB += 1;
				if (mode3SeqB >= 8 + mode3SeqEndB) 	{mode3SeqB = 8; if (mode3LoopCounterB > 0) mode3LoopCounterB --;}
				if (mode3LoopCounterB == 0) 		{groupPlay[1] = 0; mode3SeqB = 7;}
				if (mode3LoopCounterB == 1 && (mode3SeqB == mode3SeqEndB -1 + 8) ) lastPulseGeneratorB.trigger(0.1);
				if (mode3LoopCounterB > loopLimit) 	mode3LoopCounterB = loopLimit;
			}
			if (groupPlay [2] == 1) {
				if (mode3SeqCounterC > 0)  			mode3SeqCounterC -= 1;
				if (mode3SeqCounterC == 0) 			mode3SeqC += 1;
				if (mode3SeqC >= 16 + mode3SeqEndC)	 {mode3SeqC = 16; if (mode3LoopCounterC > 0) mode3LoopCounterC --;}
				if (mode3LoopCounterC == 0) 		{groupPlay[2] = 0; mode3SeqC = 15;}
				if (mode3LoopCounterC == 1 && (mode3SeqC == mode3SeqEndC -1 + 16) ) lastPulseGeneratorC.trigger(0.1);
				if (mode3LoopCounterC > loopLimit) 	mode3LoopCounterC = loopLimit;
			}
			if (groupPlay [3] == 1) {
				if (mode3SeqCounterD > 0)  			mode3SeqCounterD -= 1;
				if (mode3SeqCounterD == 0) 			mode3SeqD += 1;
				if (mode3SeqD >= 24 + mode3SeqEndD) {mode3SeqD = 24; if (mode3LoopCounterD > 0) mode3LoopCounterD --;}
				if (mode3LoopCounterD == 0) 		{groupPlay[3] = 0; mode3SeqD = 23;}
				if (mode3LoopCounterD == 1 && (mode3SeqD == mode3SeqEndD -1 + 24) ) lastPulseGeneratorD.trigger(0.1);
				if (mode3LoopCounterD > loopLimit) 	mode3LoopCounterD = loopLimit;
			}

			break;
		}

		///////////////////////////////////////////////////////////////////////////////
		// Signals output
		///////////////////////////////////////////////////////////////////////////////

		float notePitch 	= 0.0;
		int noteStaccato	= 0;
		float noteGate 		= 0;
		int noteLength 		= 0;
		float noteLengthTime = 0;
		int noteMultiply 	= 0;

		switch(mode) {

			//A+B+C+D group
			case 0:
			if (groupPlay[0] == 1) {
				notePitch 				= (params[SLIDER_NOTE_PITCH_PARAM 		+ mode1Seq].getValue())*(10/120.0) ;
				outputs[PITCH_OUTPUT + 0].setVoltage(notePitch + globalTranspose * (10/120.0) + uniOut);

				noteStaccato 			= params[SWITCH_NOTE_STACCATO_PARAM 	+ mode1Seq].getValue() ;
				noteGate 				= params[SWITCH_NOTE_PAUSE_ACC_PARAM 	+ mode1Seq].getValue() ;
				noteLength 				= params[SLIDER_NOTE_LENGTH_PARAM 		+ mode1Seq].getValue() ;
				noteMultiply 			= params[SWITCH_NOTE_DOT_TRI_PARAM 		+ mode1Seq].getValue() ;
				noteLengthTime 			= (0.9 * noteTime * pow(2, noteLength) -
											(0.8 * noteTime * pow(2, noteLength) * noteStaccato));

				if (noteMultiply == 1) noteLengthTime 		*= 1.5;
				if (noteMultiply == 2) noteLengthTime 		*= 1.333;

				if (noteGate > 0 && groupPlay[0]==1 && mode1SeqCounter == 0) {
					notePulseGeneratorA.trigger(noteLengthTime);
					outputs[GATE_OUTPUT + 0].value 			= 10;
					outputs[ACCENT_OUTPUT + 0].value 		= (noteGate == 2) ? 10.0 : 0.0;
				}

				if (mode1SeqCounter == 0) {
					mode1SeqCounter = pow(2, noteLength);
					if (noteMultiply == 1) mode1SeqCounter = round (mode1SeqCounter * 1.5); 
					if (noteMultiply == 2) mode1SeqCounter = round (mode1SeqCounter * 1.333); 
				}
			}
			break;

		///////////////////////////////////////////////////////////////////////////////

			//A+B and C+D Group
			case 1:
			//A
			if (groupPlay[0] == 1) {
				notePitch 				= (params[SLIDER_NOTE_PITCH_PARAM 		+ mode2SeqA].getValue())*(10/120.0) ;
				outputs[PITCH_OUTPUT + 0].setVoltage(notePitch  + globalTranspose * (10/120.0) + uniOut);

				noteStaccato 			= params[SWITCH_NOTE_STACCATO_PARAM 	+ mode2SeqA].getValue() ;
				noteGate 				= params[SWITCH_NOTE_PAUSE_ACC_PARAM 	+ mode2SeqA].getValue() ;
				noteLength 				= params[SLIDER_NOTE_LENGTH_PARAM 		+ mode2SeqA].getValue() ;
				noteMultiply 			= params[SWITCH_NOTE_DOT_TRI_PARAM 		+ mode2SeqA].getValue() ;
				noteLengthTime 			= (0.9*noteTime*pow(2, noteLength) -
											(0.8*noteTime*pow(2, noteLength) * noteStaccato));

				if (noteMultiply == 1) noteLengthTime 		*= 1.5;
				if (noteMultiply == 2) noteLengthTime 		*= 1.333;

				if (noteGate > 0 && groupPlay[0]==1 && mode2SeqCounterA == 0) {
					notePulseGeneratorA.trigger(noteLengthTime);
					outputs[GATE_OUTPUT + 0].value 			= 10;
					outputs[ACCENT_OUTPUT + 0].value 		= (noteGate == 2) ? 10.0 : 0.0;
				}
				if (mode2SeqCounterA == 0) {
					mode2SeqCounterA = pow(2, noteLength);
					if (noteMultiply == 1) mode2SeqCounterA = round (mode2SeqCounterA * 1.5); 
					if (noteMultiply == 2) mode2SeqCounterA = round (mode2SeqCounterA * 1.333); 
				}
			}

			//C
			if (groupPlay[2] == 1) {
				notePitch 				= (params[SLIDER_NOTE_PITCH_PARAM 		+ mode2SeqC].getValue())*(10/120.0) ;
				outputs[PITCH_OUTPUT + 2].setVoltage(notePitch  + globalTranspose * (10/120.0) + uniOut);

				noteStaccato 			= params[SWITCH_NOTE_STACCATO_PARAM 	+ mode2SeqC].getValue() ;
				noteGate 				= params[SWITCH_NOTE_PAUSE_ACC_PARAM 	+ mode2SeqC].getValue() ;
				noteLength 				= params[SLIDER_NOTE_LENGTH_PARAM 		+ mode2SeqC].getValue() ;
				noteMultiply 			= params[SWITCH_NOTE_DOT_TRI_PARAM 		+ mode2SeqC].getValue() ;
				noteLengthTime 			= (0.9*noteTime*pow(2, noteLength) -
											(0.8*noteTime*pow(2, noteLength) * noteStaccato));

				if (noteMultiply == 1) noteLengthTime 		*= 1.5;
				if (noteMultiply == 2) noteLengthTime 		*= 1.333;

				if (noteGate > 0 && groupPlay[2]==1 && mode2SeqCounterC == 0) {
					notePulseGeneratorC.trigger(noteLengthTime);
					outputs[GATE_OUTPUT + 2].value 			= 10;
					outputs[ACCENT_OUTPUT + 2].value 		= (noteGate == 2) ? 10.0 : 0.0;
				}
				if (mode2SeqCounterC == 0) {
					mode2SeqCounterC = pow(2, noteLength);
					if (noteMultiply == 1) mode2SeqCounterC = round (mode2SeqCounterC * 1.5); 
					if (noteMultiply == 2) mode2SeqCounterC = round (mode2SeqCounterC * 1.333); 
				}
			}
			break;

		///////////////////////////////////////////////////////////////////////////////


			//A and B and C and D Group
			case 2:
			//A
			if (groupPlay[0] == 1) {
				notePitch 				= (params[SLIDER_NOTE_PITCH_PARAM 		+ mode3SeqA].getValue())*(10/120.0) ;
				outputs[PITCH_OUTPUT + 0].setVoltage(notePitch  + globalTranspose * (10/120.0) + uniOut);

				noteStaccato 			= params[SWITCH_NOTE_STACCATO_PARAM 	+ mode3SeqA].getValue() ;
				noteGate 				= params[SWITCH_NOTE_PAUSE_ACC_PARAM 	+ mode3SeqA].getValue() ;
				noteLength 				= params[SLIDER_NOTE_LENGTH_PARAM 		+ mode3SeqA].getValue() ;
				noteMultiply 			= params[SWITCH_NOTE_DOT_TRI_PARAM 		+ mode3SeqA].getValue() ;
				noteLengthTime 			= (0.9*noteTime*pow(2, noteLength) -
											(0.8*noteTime*pow(2, noteLength) * noteStaccato));

				if (noteMultiply == 1) noteLengthTime 		*= 1.5;
				if (noteMultiply == 2) noteLengthTime 		*= 1.333;

				if (noteGate > 0 && groupPlay[0]==1 && mode3SeqCounterA == 0) {
					notePulseGeneratorA.trigger(noteLengthTime);
					outputs[GATE_OUTPUT + 0].value 			= 10;
					outputs[ACCENT_OUTPUT + 0].value 		= (noteGate == 2) ? 10.0 : 0.0;
				}
				if (mode3SeqCounterA == 0) {
					mode3SeqCounterA = pow(2, noteLength);
					if (noteMultiply == 1) mode3SeqCounterA = round (mode3SeqCounterA * 1.5); 
					if (noteMultiply == 2) mode3SeqCounterA = round (mode3SeqCounterA * 1.333); 
				}
			}

			//B
			if (groupPlay[1] == 1) {
				notePitch 				= (params[SLIDER_NOTE_PITCH_PARAM 		+ mode3SeqB].getValue())*(10/120.0) ;
				outputs[PITCH_OUTPUT + 1].setVoltage(notePitch  + globalTranspose * (10/120.0) + uniOut);

				noteStaccato 			= params[SWITCH_NOTE_STACCATO_PARAM 	+ mode3SeqB].getValue() ;
				noteGate 				= params[SWITCH_NOTE_PAUSE_ACC_PARAM 	+ mode3SeqB].getValue() ;
				noteLength 				= params[SLIDER_NOTE_LENGTH_PARAM 		+ mode3SeqB].getValue() ;
				noteMultiply 			= params[SWITCH_NOTE_DOT_TRI_PARAM 		+ mode3SeqB].getValue() ;
				noteLengthTime 			= (0.9*noteTime*pow(2, noteLength) -
											(0.8*noteTime*pow(2, noteLength) * noteStaccato));

				if (noteMultiply == 1) noteLengthTime 		*= 1.5;
				if (noteMultiply == 2) noteLengthTime 		*= 1.333;

				if (noteGate > 0 && groupPlay[1]==1 && mode3SeqCounterB == 0) {
					notePulseGeneratorB.trigger(noteLengthTime);
					outputs[GATE_OUTPUT + 1].value 			= 10;
					outputs[ACCENT_OUTPUT + 1].value 		= (noteGate == 2) ? 10.0 : 0.0;
				}
				if (mode3SeqCounterB == 0) {
					mode3SeqCounterB = pow(2, noteLength);
					if (noteMultiply == 1) mode3SeqCounterB = round (mode3SeqCounterB * 1.5); 
					if (noteMultiply == 2) mode3SeqCounterB = round (mode3SeqCounterB * 1.333); 
				}
			}
			

			//C
			if (groupPlay[2] == 1) {
				notePitch 				= (params[SLIDER_NOTE_PITCH_PARAM 		+ mode3SeqC].getValue())*(10/120.0) ;
				outputs[PITCH_OUTPUT + 2].setVoltage(notePitch  + globalTranspose * (10/120.0) + uniOut);

				noteStaccato 			= params[SWITCH_NOTE_STACCATO_PARAM 	+ mode3SeqC].getValue() ;
				noteGate 				= params[SWITCH_NOTE_PAUSE_ACC_PARAM 	+ mode3SeqC].getValue() ;
				noteLength 				= params[SLIDER_NOTE_LENGTH_PARAM 		+ mode3SeqC].getValue() ;
				noteMultiply 			= params[SWITCH_NOTE_DOT_TRI_PARAM 		+ mode3SeqC].getValue() ;
				noteLengthTime 			= (0.9*noteTime*pow(2, noteLength) -
											(0.8*noteTime*pow(2, noteLength) * noteStaccato));

				if (noteMultiply == 1) noteLengthTime 		*= 1.5;
				if (noteMultiply == 2) noteLengthTime 		*= 1.333;

				if (noteGate > 0 && groupPlay[2]==1 && mode3SeqCounterC == 0) {
					notePulseGeneratorC.trigger(noteLengthTime);
					outputs[GATE_OUTPUT + 2].value 			= 10;
					outputs[ACCENT_OUTPUT + 2].value 		= (noteGate == 2) ? 10.0 : 0.0;
				}
				if (mode3SeqCounterC == 0) {
					mode3SeqCounterC = pow(2, noteLength);
					if (noteMultiply == 1) mode3SeqCounterC = round (mode3SeqCounterC * 1.5); 
					if (noteMultiply == 2) mode3SeqCounterC = round (mode3SeqCounterC * 1.333); 
				}
			}
			

			//D
			if (groupPlay[3] == 1) {
				notePitch 				= (params[SLIDER_NOTE_PITCH_PARAM 		+ mode3SeqD].getValue())*(10/120.0) ;
				outputs[PITCH_OUTPUT + 3].setVoltage(notePitch  + globalTranspose * (10/120.0) + uniOut);

				noteStaccato 			= params[SWITCH_NOTE_STACCATO_PARAM 	+ mode3SeqD].getValue() ;
				noteGate 				= params[SWITCH_NOTE_PAUSE_ACC_PARAM 	+ mode3SeqD].getValue() ;
				noteLength 				= params[SLIDER_NOTE_LENGTH_PARAM 		+ mode3SeqD].getValue() ;
				noteMultiply 			= params[SWITCH_NOTE_DOT_TRI_PARAM 		+ mode3SeqD].getValue() ;
				noteLengthTime 			= (0.9*noteTime*pow(2, noteLength) -
											(0.8*noteTime*pow(2, noteLength) * noteStaccato));

				if (noteMultiply == 1) noteLengthTime 		*= 1.5;
				if (noteMultiply == 2) noteLengthTime 		*= 1.333;

				if (noteGate > 0 && groupPlay[3]==1 && mode3SeqCounterD == 0) {
					notePulseGeneratorD.trigger(noteLengthTime);
					outputs[GATE_OUTPUT + 3].value 			= 10;
					outputs[ACCENT_OUTPUT + 3].value 		= (noteGate == 2) ? 10.0 : 0.0;
				}
				if (mode3SeqCounterD == 0) {
					mode3SeqCounterD = pow(2, noteLength);
					if (noteMultiply == 1) mode3SeqCounterD = round (mode3SeqCounterD * 1.5); 
					if (noteMultiply == 2) mode3SeqCounterD = round (mode3SeqCounterD * 1.333); 
				}
			}
			break;
		}
	} // clockGateOn end

	///////////////////////////////////////////////////////////////////////////////

	//Group leds
	for (int i = 0; i < 4; i++) {
		lights[GROUP_LIGHT + (2* i )   ].value = (groupActive[i] + groupPlay[i] == 2) ? 1 : 0;
		lights[GROUP_LIGHT + (2* i ) +1].value = groupActive[i] - groupPlay[i];
	}

	switch(mode) {
		case 0:
		for (int i = 0; i <32; i++) {
			lights[SELECTED_STEP_LIGHT + i].value = (i == mode1Seq) ? 1.0 : 0;
		}	
		break;
		
		case 1:
		for (int i = 0; i <16; i++) {
			lights[SELECTED_STEP_LIGHT + i].value 		= (i == mode2SeqA) ? 1.0 : 0;
			lights[SELECTED_STEP_LIGHT + i + 16].value 	= (i == mode2SeqC - 16) ? 1.0 : 0;
		}	
		
		break;
		
		case 2:
		for (int i = 0; i <8; i++) {
			lights[SELECTED_STEP_LIGHT + i].value 		= (i == mode3SeqA) ? 1.0 : 0;
			lights[SELECTED_STEP_LIGHT + i + 8].value 	= (i == mode3SeqB - 8) ? 1.0 : 0;
			lights[SELECTED_STEP_LIGHT + i + 16].value 	= (i == mode3SeqC - 16) ? 1.0 : 0;
			lights[SELECTED_STEP_LIGHT + i + 24].value 	= (i == mode3SeqD - 24) ? 1.0 : 0;
		}	
		break;
	}

	//note gates clocks off
	if (!notePulseGeneratorA.process(deltaTime)) outputs[GATE_OUTPUT + 0].setVoltage(0.0f);
	if (!notePulseGeneratorB.process(deltaTime)) outputs[GATE_OUTPUT + 1].setVoltage(0.0f);
	if (!notePulseGeneratorC.process(deltaTime)) outputs[GATE_OUTPUT + 2].setVoltage(0.0f);
	if (!notePulseGeneratorD.process(deltaTime)) outputs[GATE_OUTPUT + 3].setVoltage(0.0f);

	//output clocks off
	bool pulseClock = clockPulseGenerator.process(deltaTime);
	outputs[CLK16_OUTPUT].setVoltage(pulseClock ? 10.0f : 0.0f);

	bool pulseBPM = bpmPulseGenerator.process(deltaTime);
	outputs[CLK4_OUTPUT].setVoltage(pulseBPM ? 10.0f : 0.0f);


	bool pulseFirst = lastPulseGeneratorA.process(deltaTime);
	outputs[LAST_OUTPUT].setVoltage(pulseFirst ? 10.0f : 0.0f);

	pulseFirst = lastPulseGeneratorB.process(deltaTime);
	outputs[LAST_OUTPUT + 1].setVoltage(pulseFirst ? 10.0f : 0.0f);

	pulseFirst = lastPulseGeneratorC.process(deltaTime);
	outputs[LAST_OUTPUT + 2].setVoltage(pulseFirst ? 10.0f : 0.0f);

	pulseFirst = lastPulseGeneratorD.process(deltaTime);
	outputs[LAST_OUTPUT + 3].setVoltage(pulseFirst ? 10.0f : 0.0f);

	///////////////////////////////////////////////////////////////////////////////

	//length touch leds
	if ((touchSelected >=0 ) || (oldMode != mode)) {
		switch(mode) {
			case 0:
			for (int i = 0; i <32; i++) {
				lights[GROUP_LENGTH_LIGHT + i 		].value = (i < mode1SeqEnd ) ? 1.0 : 0;
			}	
			break;
			
			case 1:
			for (int i = 0; i <16; i++) {
				lights[GROUP_LENGTH_LIGHT + i 		].value = (i < mode2SeqEndA ) ? 1.0 : 0;
				lights[GROUP_LENGTH_LIGHT + i + 16	].value = (i < mode2SeqEndC ) ? 1.0 : 0;
			}	
			break;
			
			case 2:
			for (int i = 0; i <8; i++) {
				lights[GROUP_LENGTH_LIGHT + i 		].value = (i < mode3SeqEndA ) ? 1.0 : 0;
				lights[GROUP_LENGTH_LIGHT + i + 8	].value = (i < mode3SeqEndB ) ? 1.0 : 0;
				lights[GROUP_LENGTH_LIGHT + i + 16	].value = (i < mode3SeqEndC ) ? 1.0 : 0;
				lights[GROUP_LENGTH_LIGHT + i + 24	].value = (i < mode3SeqEndD ) ? 1.0 : 0;
			}	
			break;
		}
		touchSelected = -1;
	}


	oldMode = mode;


	//display edited note
	int sliderTouch = -1000;
	for (int i = 0; i <32; i++) {
		sliderValue [i] = round(params[SLIDER_NOTE_PITCH_PARAM + i].getValue());
		if (sliderValue [i] != sliderValueOld [i]) sliderTouch = i;
		//store slider value for the nest step
		sliderValueOld [i] = sliderValue [i];
	}

	if (sliderTouch > -1000) {
		stepNumber = char(65 + floor(sliderTouch / 8)) + std::to_string(1 + sliderTouch % 8);

		int note =  round(params[SLIDER_NOTE_PITCH_PARAM + sliderTouch].getValue()) + 24;

		notePitchDisplay = noteNames[note % 12];

		//Audible sound
		if (params[SWITCH_MONITOR_PARAM].getValue() == 1){
		
			note -= 24;
			float gateMonitorTime = 0.2;

			switch(mode) {
				case 0:
				notePulseGeneratorA.trigger(gateMonitorTime); 
				outputs[GATE_OUTPUT + 0].value 			= 10;
				outputs[PITCH_OUTPUT + 0].value 		= note * (10/120.0) + globalTranspose * (10/120.0) + uniOut;
				
				break;
				
				case 1:
				if (sliderTouch<16) {
					notePulseGeneratorA.trigger(gateMonitorTime); 
					outputs[GATE_OUTPUT + 0].value 		= 10;
					outputs[PITCH_OUTPUT +0].value 		= note * (10/120.0) + globalTranspose * (10/120.0) + uniOut;
				} else {
					notePulseGeneratorC.trigger(gateMonitorTime); 
					outputs[GATE_OUTPUT + 2].value 		= 10;
					outputs[PITCH_OUTPUT +2].value 		= note * (10/120.0) + globalTranspose * (10/120.0) + uniOut;
				}

				break;
				
				case 2:
				if (sliderTouch<8) {
					notePulseGeneratorA.trigger(gateMonitorTime); 
					outputs[GATE_OUTPUT + 0].value 		= 10;
					outputs[PITCH_OUTPUT +0].value 		= note * (10/120.0) + globalTranspose * (10/120.0) + uniOut;
				}
				if (sliderTouch >=8 && sliderTouch <24) {
					if (sliderTouch<16) {
						notePulseGeneratorB.trigger(gateMonitorTime); 
						outputs[GATE_OUTPUT +1].value 	= 10;
						outputs[PITCH_OUTPUT +1].value 	= note * (10/120.0) + globalTranspose * (10/120.0) + uniOut;
					} else {
						notePulseGeneratorC.trigger(gateMonitorTime); 
						outputs[GATE_OUTPUT +2].value 	= 10;
						outputs[PITCH_OUTPUT +2].value 	= note * (10/120.0) + globalTranspose * (10/120.0) + uniOut;
					}
				}

				if (sliderTouch>=24) {
					notePulseGeneratorD.trigger(gateMonitorTime); 
					outputs[GATE_OUTPUT + 3].value 		= 10;
					outputs[PITCH_OUTPUT +3].value 		= note * (10/120.0) + globalTranspose * (10/120.0) + uniOut;
				}
				break;
			} // End of Switch
		}
	}

	//End of first step
	initStep = false;
}


///////////////////////////////////////////////////////////////////////////////
// Store variables
///////////////////////////////////////////////////////////////////////////////

//json_t *Scorovnik::dataToJson() {
json_t *dataToJson() override {
	json_t *rootJ = json_object();


	json_object_set_new(rootJ, "groupPlayA", json_integer(groupPlay[0]));
	json_object_set_new(rootJ, "groupPlayB", json_integer(groupPlay[1]));
	json_object_set_new(rootJ, "groupPlayC", json_integer(groupPlay[2]));
	json_object_set_new(rootJ, "groupPlayD", json_integer(groupPlay[3]));

	json_object_set_new(rootJ, "mode1SeqEnd", json_integer(mode1SeqEnd));
	json_object_set_new(rootJ, "mode2SeqEndA", json_integer(mode2SeqEndA));
	json_object_set_new(rootJ, "mode2SeqEndC", json_integer(mode2SeqEndC));
	json_object_set_new(rootJ, "mode3SeqEndA", json_integer(mode3SeqEndA));
	json_object_set_new(rootJ, "mode3SeqEndB", json_integer(mode3SeqEndB));
	json_object_set_new(rootJ, "mode3SeqEndC", json_integer(mode3SeqEndC));
	json_object_set_new(rootJ, "mode3SeqEndD", json_integer(mode3SeqEndD));
	return rootJ;
}

void dataFromJson(json_t *rootJ) override {



	json_t *j_groupPlayA = json_object_get(rootJ, "groupPlayA");
	groupPlay[0] = json_integer_value(j_groupPlayA);

	json_t *j_groupPlayB = json_object_get(rootJ, "groupPlayB");
	groupPlay[1] = json_integer_value(j_groupPlayB);

	json_t *j_groupPlayC = json_object_get(rootJ, "groupPlayC");
	groupPlay[2] = json_integer_value(j_groupPlayC);

	json_t *j_groupPlayD = json_object_get(rootJ, "groupPlayD");
	groupPlay[3] = json_integer_value(j_groupPlayD);






	json_t *j_mode1SeqEnd = json_object_get(rootJ, "mode1SeqEnd");
	mode1SeqEnd = json_integer_value(j_mode1SeqEnd);

	json_t *j_mode2SeqEndA = json_object_get(rootJ, "mode2SeqEndA");
	mode2SeqEndA = json_integer_value(j_mode2SeqEndA);

	json_t *j_mode2SeqEndC = json_object_get(rootJ, "mode2SeqEndC");
	mode2SeqEndC = json_integer_value(j_mode2SeqEndC);

	json_t *j_mode3SeqEndA = json_object_get(rootJ, "mode3SeqEndA");
	mode3SeqEndA = json_integer_value(j_mode3SeqEndA);

	json_t *j_mode3SeqEndB = json_object_get(rootJ, "mode3SeqEndB");
	mode3SeqEndB = json_integer_value(j_mode3SeqEndB);

	json_t *j_mode3SeqEndC = json_object_get(rootJ, "mode3SeqEndC");
	mode3SeqEndC = json_integer_value(j_mode3SeqEndC);

	json_t *j_mode3SeqEndD = json_object_get(rootJ, "mode3SeqEndD");
	mode3SeqEndD = json_integer_value(j_mode3SeqEndD);


}

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// GUI ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct ScorovnikWidget : ModuleWidget {

    ScorovnikWidget(Scorovnik *module) {
        setModule(module);

		box.size = Vec(68 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Scorovnik-Dark.svg")));





		///////////////////////////////////////////////////////////////////////////////
		// Displays
		///////////////////////////////////////////////////////////////////////////////
		if (module) {
			//Tempo
			Dot3DisplayWidget *displayTempo = new Dot3DisplayWidget();
			displayTempo->box.pos = Vec(50,20);
			displayTempo->box.size = Vec(45, 20);
			displayTempo->value = &module->tempo;
			displayTempo->colorDisplay = &module->colorDisplay;
			addChild(displayTempo);

			//Transpose
			Dot3DisplayWidget *displayTranspose = new Dot3DisplayWidget();
			displayTranspose->box.pos = Vec(103,20);
			displayTranspose->box.size = Vec(45, 20);
			displayTranspose->value = &module->transpose;
			displayTranspose->colorDisplay = &module->colorDisplay;
			addChild(displayTranspose);

			//Loop numbers
			Dot3DisplayWidget *displayLoop = new Dot3DisplayWidget();
			displayLoop->box.pos = Vec(155,20);
			displayLoop->box.size = Vec(45, 20);
			displayLoop->value = &module->loopNumber;
			displayLoop->colorDisplay = &module->colorDisplay;
			addChild(displayLoop);



			//Step numbers (helper)
			Dot2DisplayWidget *displayStep = new Dot2DisplayWidget();
			displayStep->box.pos = Vec(908,69);
			displayStep->box.size = Vec(45, 20);
			displayStep->value = &module->stepNumber;
			displayStep->colorDisplay = &module->colorDisplay;
			addChild(displayStep);

			//Note pitch (helper)
			Dot2DisplayWidget *displayNotePitch = new Dot2DisplayWidget();
			displayNotePitch->box.pos = Vec(938,69);
			displayNotePitch->box.size = Vec(45, 20);
			displayNotePitch->value = &module->notePitchDisplay;
			displayNotePitch->colorDisplay = &module->colorDisplay;
			addChild(displayNotePitch);
		}


		///////////////////////////////////////////////////////////////////////////////
		// Steps elements
		/////////////////////////////////////////////////////////////////////////////// 

		float leftPos = 200;
		float topPos = 0;
		float stepY = 0;
		float stepX = 0;

		for (int i = 0; i < 32; i++) {
			if(i  % 8 == 0)  stepX += 10;
			//Sliders
			addParam(createParam<Koralfx_PitchSlider>					(Vec(leftPos + 18 + stepX + i*20, 63),
				module, Scorovnik::SLIDER_NOTE_PITCH_PARAM + i));

			addParam(createParam<Koralfx_LengthSlider>					(Vec(leftPos + 18 + stepX + i*20, 218),
				module, Scorovnik::SLIDER_NOTE_LENGTH_PARAM + i));
			//switches
			addParam(createParam<Koralfx_Switch_Green_Red>				(Vec(leftPos + 21 + stepX + i*20, 303),
				module, Scorovnik::SWITCH_NOTE_DOT_TRI_PARAM + i));

			addParam(createParam<Koralfx_Switch_Green_Red>				(Vec(leftPos + 21 + stepX + i*20, 323),
				module, Scorovnik::SWITCH_NOTE_PAUSE_ACC_PARAM + i));

			addParam(createParam<Koralfx_Switch_Blue>					(Vec(leftPos + 21 + stepX + i*20, 343),
				module, Scorovnik::SWITCH_NOTE_STACCATO_PARAM + i));

			addParam(createParam<Koralfx_LEDButton>						(Vec(leftPos + 20.6 + stepX + i*20, 202),
				module, Scorovnik::LED_BUTTON_PARAM + i));

			//lights
			addChild(createLight<SmallLight<BlueLight>>			(Vec(leftPos + 24.5 + stepX + i*20, 51),
				module, Scorovnik::SELECTED_STEP_LIGHT + i));

			addChild(createLight<SmallLight<BlueLight>>			(Vec(leftPos + 24.5 + stepX + i*20, 206),
				module, Scorovnik::GROUP_LENGTH_LIGHT + i));
		}

		for (int i = 0; i < 4; i++) {
			addChild(createLight<MediumLight<GreenRedLight>>(Vec(leftPos + 13 + stepX + 8*i*20 + 10 * i, 20),
				module, Scorovnik::GROUP_LIGHT + i*2));
		}

		///////////////////////////////////////////////////////////////////////////////
		// IN/OUT elements
		///////////////////////////////////////////////////////////////////////////////

		leftPos = 50;
		topPos = 90;
		stepY = 27;
		stepX = 37;
		for (int i = 0; i < 4; i++) {
			//Inputs
			addInput(createInput<PJ301MPort>			(Vec(leftPos + i*stepX , 0 * stepY + topPos),
				module, Scorovnik::START_INPUT + i));

			addInput(createInput<PJ301MPort>			(Vec(leftPos + i*stepX , 1 * stepY + topPos),
				module, Scorovnik::STOP_INPUT + i));

			addInput(createInput<PJ301MPort>			(Vec(leftPos + i*stepX , 2 * stepY + topPos),
				module, Scorovnik::RESET_INPUT + i));

			//Outputs
			addOutput(createOutput<PJ301MPort>			(Vec(leftPos + i*stepX , 3 * stepY + topPos),
				module, Scorovnik::GATE_OUTPUT + i));

			addOutput(createOutput<PJ301MPort>			(Vec(leftPos + i*stepX , 4 * stepY + topPos),
				module, Scorovnik::PITCH_OUTPUT + i));

			addOutput(createOutput<PJ301MPort>			(Vec(leftPos + i*stepX , 5 * stepY + topPos),
				module, Scorovnik::ACCENT_OUTPUT + i));

			addOutput(createOutput<PJ301MPort>			(Vec(leftPos + i*stepX , 6 * stepY + topPos),
				module, Scorovnik::LAST_OUTPUT + i));

		}


		//Outputs
		addOutput(createOutput<PJ301MPort>(Vec(leftPos + 0*stepX , 330),
			module, Scorovnik::CLK4_OUTPUT));

		addOutput(createOutput<PJ301MPort>(Vec(leftPos + 1*stepX , 330),
			module, Scorovnik::CLK16_OUTPUT));

		//Inputs
		addInput(createInput<PJ301MPort>(Vec(leftPos + 3*stepX , 330),
			module, Scorovnik::CV_TRANSPOSE_INPUT));

		addInput(createInput<PJ301MPort>(Vec(leftPos + 2*stepX , 330),
			module, Scorovnik::EXT_CLK_INPUT));


		///////////////////////////////////////////////////////////////////////////////
		// Control elements
		///////////////////////////////////////////////////////////////////////////////

		//switches
		addParam(createParam<Koralfx_Switch_Green_Red>(Vec(21, 315), 
			module, Scorovnik::SWITCH_MODE_PARAM));

		addParam(createParam<Koralfx_CKD6_Blue>(Vec(leftPos - 5 + 4*stepX , 0 * stepY + topPos),
			module, Scorovnik::SWITCH_START_PARAM));

		addParam(createParam<Koralfx_CKD6_Blue>(Vec(leftPos - 5 + 4*stepX , 1 * stepY + topPos),
			module, Scorovnik::SWITCH_STOP_PARAM));

		addParam(createParam<Koralfx_CKD6_Blue>(Vec(leftPos - 5 + 4*stepX , 2 * stepY + topPos),
			module, Scorovnik::SWITCH_RESET_PARAM));

		addParam(createParam<Koralfx_Switch_Blue>(Vec(930,146),  module, Scorovnik::SWITCH_MONITOR_PARAM));
		addParam(createParam<Koralfx_Switch_Blue>(Vec(200.5,201.5),  module, Scorovnik::SWITCH_UNI_PARAM));


		//Knobs
		addParam(createParam<Koralfx_StepRoundSmallBlackKnob>(Vec(59, 46),
			module, Scorovnik::TEMPO_PARAM));

		addParam(createParam<Koralfx_StepRoundSmallBlackKnob>(Vec(113, 46),
			module, Scorovnik::TRANSPOSE_PARAM));

		addParam(createParam<Koralfx_StepRoundSmallBlackKnob>(Vec(167, 46),
			module, Scorovnik::KNOB_LOOP_NUMBER_PARAM));
	}
};


///////////////////////////////////////////////////////////////////////////////

Model *modelScorovnik = createModel<Scorovnik, ScorovnikWidget>("Scorovnik");
