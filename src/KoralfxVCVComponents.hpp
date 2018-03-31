#ifndef KORALFXVCV_COMPONENTS_HPP
#define KORALFXVCV_COMPONENTS_HPP

#include "KoralfxVCVWidgets.hpp"

#include "app.hpp"

#include <sstream>
#include <iomanip>
#include <string>


///////////////////////////////////////////////////////////////////////////////
// Sliders
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_SliderPot : SVGSlider {
	Koralfx_SliderPot() {
		Vec margin = Vec(4, 4);
		maxHandlePos = Vec(-1.5, -8).plus(margin);
		minHandlePos = Vec(-1.5, 87).plus(margin);
		background->svg = SVG::load(assetPlugin(plugin,"res/Koralfx_SliderPot.svg"));
		background->wrap();
		background->box.pos = margin;
		box.size = background->box.size.plus(margin.mult(2));
		handle->svg = SVG::load(assetPlugin(plugin,"res/Koralfx_SliderPotHandle.svg"));
		handle->wrap();
	}
};

///////////////////////////////////////

struct Koralfx_PitchSlider : SVGSlider {
	Koralfx_PitchSlider() {
		maxHandlePos = mm2px(Vec(0.738, 0.738).plus(Vec(2, 0)));
		minHandlePos = mm2px(Vec(0.738, 41.478).plus(Vec(2, 0)));
		setSVGs(SVG::load(assetPlugin(plugin, "res/Koralfx_PitchSlider.svg")),
							SVG::load(assetPlugin(plugin, "res/Koralfx_LEDSliderRedHandle.svg")));
		snap = true;
	}
};

///////////////////////////////////////

struct Koralfx_LengthSlider : SVGSlider {
	Koralfx_LengthSlider() {
		maxHandlePos = mm2px(Vec(0.738, 0.738).plus(Vec(2, 0)));
		minHandlePos = mm2px(Vec(0.738, 21.078).plus(Vec(2, 0)));
		setSVGs(SVG::load(assetPlugin(plugin, "res/Koralfx_LengthSlider.svg")),
							SVG::load(assetPlugin(plugin, "res/Koralfx_LEDSliderBlueHandle.svg")));
		snap = true;
	}
};


///////////////////////////////////////////////////////////////////////////////
// 2-state Toggle Switches
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_Switch_Red : SVGSwitch, ToggleSwitch {
	Koralfx_Switch_Red() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Koralfx_Switch_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Koralfx_Switch_1_Red.svg")));
	}
};
///////////////////////////////////////
struct Koralfx_Switch_Blue : SVGSwitch, ToggleSwitch {
	Koralfx_Switch_Blue() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Koralfx_Switch_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Koralfx_Switch_1_Blue.svg")));
	}
};
///////////////////////////////////////
struct Koralfx_Switch_Green : SVGSwitch, ToggleSwitch {
	Koralfx_Switch_Green() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Koralfx_Switch_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Koralfx_Switch_1_Green.svg")));
	}

};


///////////////////////////////////////////////////////////////////////////////
// 3-state Toggle Switches
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_Switch_Green_Red : SVGSwitch, ToggleSwitch {
	Koralfx_Switch_Green_Red() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Koralfx_Switch_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Koralfx_Switch_1_Green.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Koralfx_Switch_1_Red.svg")));
	}
};


///////////////////////////////////////////////////////////////////////////////
// Momentary Switches
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_CKD6_Blue : SVGSwitch, MomentarySwitch {
	Koralfx_CKD6_Blue() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Koralfx_CKD6_Blue_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Koralfx_CKD6_Blue_1.svg")));
	}
};


///////////////////////////////////////////////////////////////////////////////
// Step Knobs
///////////////////////////////////////////////////////////////////////////////

struct Koralfx_StepRoundSmallBlackKnob : RoundKnob {
	Koralfx_StepRoundSmallBlackKnob() {
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/RoundSmallBlackKnob.svg")));
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
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/RoundLargeBlackKnob.svg")));
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
		setSVG(SVG::load(assetGlobal("res/ComponentLibrary/RoundBlackKnob.svg")));
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

struct Koralfx_LEDButton : SVGSwitch, MomentarySwitch {
	Koralfx_LEDButton() {
		addFrame(SVG::load(assetPlugin(plugin,"res/Koralfx_LEDButton.svg")));
	}
};


///////////////////////////////////////////////////////////////////////////////
// Displays
///////////////////////////////////////////////////////////////////////////////

struct Seg3DisplayWidget : TransparentWidget {
	std::string *value;
	NVGcolor colorDisplay;
	std::shared_ptr<Font> font;

	Seg3DisplayWidget() {
		font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
	};

	void draw(NVGcontext *vg) override {

		nvgFontSize(vg, 13);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.0);

		std::stringstream to_display;
		to_display << std::setw(3) << *value;


		Vec textPos = Vec(4.0f, 17.0f); 

		NVGcolor textColor = colorDisplay;
		nvgFillColor(vg, nvgTransRGBA(textColor, 30));
		nvgText(vg, textPos.x, textPos.y, "888", NULL);

		textColor = colorDisplay;
		nvgFillColor(vg, textColor);
		nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
	}
};

///////////////////////////////////////

struct Dot3DisplayWidget : TransparentWidget {
	std::string *value;
	NVGcolor colorDisplay;
	std::shared_ptr<Font> font;

	Dot3DisplayWidget() {
		font = Font::load(assetPlugin(plugin, "res/LCD_Dot_Matrix_HD44780U.ttf"));
	};

	void draw(NVGcontext *vg) override {

		nvgFontSize(vg, 17);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.0);

		std::stringstream to_display;
		to_display << std::setw(3) << *value;

		Vec textPos = Vec(4.0f, 17.0f); 

		NVGcolor textColor = colorDisplay;
		nvgFillColor(vg, nvgTransRGBA(textColor, 76));
		nvgText(vg, textPos.x, textPos.y, "॓॓॓", NULL);

		textColor = colorDisplay;
		nvgFillColor(vg, textColor);
		nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
	}
};

///////////////////////////////////////

struct Dot2DisplayWidget : TransparentWidget {
	std::string *value;
	NVGcolor colorDisplay;
	std::shared_ptr<Font> font;

	Dot2DisplayWidget() {
		font = Font::load(assetPlugin(plugin, "res/LCD_Dot_Matrix_HD44780U.ttf"));
	};

	void draw(NVGcontext *vg) override {

		nvgFontSize(vg, 17);
		nvgFontFaceId(vg, font->handle);
		nvgTextLetterSpacing(vg, 2.0);

		std::stringstream to_display;
		to_display << std::setw(2) << *value;

		Vec textPos = Vec(4.0f, 17.0f); 

		NVGcolor textColor = colorDisplay;
		nvgFillColor(vg, nvgTransRGBA(textColor, 76));
		nvgText(vg, textPos.x, textPos.y, "॓॓", NULL);

		textColor = colorDisplay;
		nvgFillColor(vg, textColor);
		nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
	}
};

///////////////////////////////////////

#endif