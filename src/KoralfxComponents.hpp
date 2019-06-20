#ifndef KORALFX_COMPONENTS_HPP
#define KORALFX_COMPONENTS_HPP

#include "app.hpp"

#include <sstream>
#include <iomanip>
#include <string>


///////////////////////////////////////////////////////////////////////////////
// Sliders
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_SliderPot : app::SvgSlider {
	Koralfx_SliderPot() {
		Vec margin = Vec(4, 4);
		maxHandlePos = Vec(-1.5, -8).plus(margin);
		minHandlePos = Vec(-1.5, 87).plus(margin);
		background->svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_SliderPot.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
		handle->svg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_SliderPotHandle.svg"));
		handle->wrap();
	}
};

///////////////////////////////////////

struct Koralfx_PitchSlider : app::SvgSlider {
	Koralfx_PitchSlider() {
		maxHandlePos = mm2px(Vec(0.738, 0.738).plus(Vec(2, 0)));
		minHandlePos = mm2px(Vec(0.738, 41.478).plus(Vec(2, 0)));
		setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_PitchSlider.svg")));
		setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_LEDSliderRedHandle.svg")));
		snap = true;
	}
};

///////////////////////////////////////

struct Koralfx_LengthSlider : app::SvgSlider {
	Koralfx_LengthSlider() {
		maxHandlePos = mm2px(Vec(0.738, 0.738).plus(Vec(2, 0)));
		minHandlePos = mm2px(Vec(0.738, 21.078).plus(Vec(2, 0)));
		setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_LengthSlider.svg"))),
		setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_LEDSliderBlueHandle.svg")));
		snap = true;
	}
};


///////////////////////////////////////////////////////////////////////////////
// 2-state Toggle Switches
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_Switch_Red : app::SvgSwitch {
	Koralfx_Switch_Red() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_Switch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_Switch_1_Red.svg")));
	}
};
///////////////////////////////////////
struct Koralfx_Switch_Blue : app::SvgSwitch {
	Koralfx_Switch_Blue() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_Switch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_Switch_1_Blue.svg")));
	}
};
///////////////////////////////////////
struct Koralfx_Switch_Green : app::SvgSwitch {
	Koralfx_Switch_Green() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_Switch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_Switch_1_Green.svg")));
	}

};


///////////////////////////////////////////////////////////////////////////////
// 3-state Toggle Switches
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_Switch_Green_Red : app::SvgSwitch {
	Koralfx_Switch_Green_Red() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_Switch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_Switch_1_Green.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_Switch_1_Red.svg")));
	}
};


///////////////////////////////////////////////////////////////////////////////
// Momentary Switches
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_CKD6_Blue : app::SvgSwitch {
	Koralfx_CKD6_Blue() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_CKD6_Blue_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Koralfx_CKD6_Blue_1.svg")));
	}
};


///////////////////////////////////////////////////////////////////////////////
// Step Knobs
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_StepRoundSmallBlackKnob : RoundKnob {
	Koralfx_StepRoundSmallBlackKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/RoundSmallBlackKnob.svg")));
		snap = true;
		this->shadow->box.size = Vec(30, 30);
		this->shadow->blurRadius = 5;
		this->shadow->box.pos = Vec(0, 0);
		this->shadow->opacity = 0.5;
	}
};
///////////////////////////////////////
struct Koralfx_StepRoundLargeBlackKnob : RoundKnob {
	Koralfx_StepRoundLargeBlackKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/RoundLargeBlackKnob.svg")));
		snap = true;
		this->shadow->box.size = Vec(45, 45);
		this->shadow->blurRadius = 7;
		this->shadow->box.pos = Vec(0, 0);
		this->shadow->opacity = 0.5;
	}
};


///////////////////////////////////////////////////////////////////////////////
// Continuous Knobs
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_RoundBlackKnob : RoundKnob {
	Koralfx_RoundBlackKnob() {
		setSvg(APP->window->loadSvg(asset::system("res/ComponentLibrary/RoundBlackKnob.svg")));
		snap = false;
		this->shadow->box.size = Vec(34, 34);
		this->shadow->blurRadius = 4;
		this->shadow->box.pos = Vec(0, 0);
		this->shadow->opacity = 0.5;
	}
};


///////////////////////////////////////////////////////////////////////////////
// Led buttons
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_LEDButton : SvgSwitch {
	//momentary = true;
	Koralfx_LEDButton() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/Koralfx_LEDButton.svg")));
	}
};


///////////////////////////////////////////////////////////////////////////////
// Displays
///////////////////////////////////////////////////////////////////////////////

struct Seg3DisplayWidget : TransparentWidget {
	std::string *value;
	NVGcolor *colorDisplay;
	std::shared_ptr<Font> font;

	Seg3DisplayWidget() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Segment7Standard.ttf"));
	};

	void draw(const DrawArgs &args) override {

		nvgFontSize(args.vg, 13);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 2.0);

		std::stringstream to_display;

		to_display << std::setw(3) << *value;
		//to_display << std::setw(3) << "10";

		Vec textPos = Vec(4.0f, 17.0f); 

		NVGcolor textColor = *colorDisplay;
		//NVGcolor textColor = nvgRGB(0xff, 0xcc, 0x00);

		nvgFillColor(args.vg, nvgTransRGBA(textColor, 30));
		nvgText(args.vg, textPos.x, textPos.y, "888", NULL);
		
		textColor = *colorDisplay;
		//textColor = nvgRGB(0xff, 0xcc, 0x00);;

		nvgFillColor(args.vg, textColor);
		nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
	}
};

///////////////////////////////////////

struct Dot3DisplayWidget : TransparentWidget {
	std::string *value;
	NVGcolor *colorDisplay;
	std::shared_ptr<Font> font;

	Dot3DisplayWidget() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/LCD_Dot_Matrix_HD44780U.ttf"));
	};

	void draw(const DrawArgs &args) override {

		nvgFontSize(args.vg, 17);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 2.0);

		std::stringstream to_display;
		to_display << std::setw(3) << *value;

		Vec textPos = Vec(4.0f, 17.0f); 

		NVGcolor textColor = *colorDisplay;
		nvgFillColor(args.vg, nvgTransRGBA(textColor, 76));
		nvgText(args.vg, textPos.x, textPos.y, "॓॓॓", NULL);

		textColor = *colorDisplay;
		nvgFillColor(args.vg, textColor);
		nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
	}
};

///////////////////////////////////////

struct Dot2DisplayWidget : TransparentWidget {
	std::string *value;
	NVGcolor *colorDisplay;
	std::shared_ptr<Font> font;

	Dot2DisplayWidget() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/LCD_Dot_Matrix_HD44780U.ttf"));
	};

	void draw(const DrawArgs &args) override {

		nvgFontSize(args.vg, 17);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 2.0);

		std::stringstream to_display;
		to_display << std::setw(2) << *value;

		Vec textPos = Vec(4.0f, 17.0f); 

		NVGcolor textColor = *colorDisplay;
		nvgFillColor(args.vg, nvgTransRGBA(textColor, 76));
		nvgText(args.vg, textPos.x, textPos.y, "॓॓", NULL);

		textColor = *colorDisplay;
		nvgFillColor(args.vg, textColor);
		nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
	}
};
 
///////////////////////////////////////////////////////////////////////////////
// Others
///////////////////////////////////////////////////////////////////////////////


struct Koralfx_knobRing : TransparentWidget{
	float *pointerKnob;
	NVGcolor *colorPointer;
	Koralfx_knobRing() {}
	
	void draw(const DrawArgs &args) override {
float d = 22.0;
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, 0,0, d);
		//nvgFillColor(args.vg, nvgRGBA(0x55, 0xaa, 0xff, 0x33)); 
		nvgFillColor(args.vg, nvgTransRGBA(*colorPointer, 0x33)); 
		nvgFill(args.vg);

		
		for (int i = 210; i <= 510 ; i += 150) {
		float gradius = i ;
		float xx =  d * sin( gradius *0.0174);
		float yy =  -d * cos( gradius *0.0174);
			nvgFillColor(args.vg, nvgRGBA(0x28, 0x2c, 0x33, 0xff));
			//nvgStrokeColor(args.vg, nvgRGBA(0x55, 0xaa, 0xff, 0xff));
			{
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, 0,0);
				nvgLineTo(args.vg, xx,yy);
				nvgClosePath(args.vg);
			}
			nvgStroke(args.vg);
		}


		float gradius = 210 + *pointerKnob * 360 * 0.8333;
		float xx =  d * sin( gradius *0.0174);
		float yy =  -d * cos( gradius *0.0174);

				nvgStrokeWidth(args.vg, 2.0);
				nvgLineCap(args.vg, NVG_ROUND);
				nvgMiterLimit(args.vg, 2.0);

			nvgStrokeColor(args.vg, nvgTransRGBA(*colorPointer, 0xff));
			{
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, 0,0);
				nvgLineTo(args.vg, xx,yy);
				nvgClosePath(args.vg);
			}
			nvgStroke(args.vg);
	}
};


///////////////////////////////////////


#endif