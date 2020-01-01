/*
 * LedRingNode.cpp
 *
 *  Created on: 11.03.2018
 *      Author: ian
 */

#include <LedRingNode.h>
#include <LoggerNode.h>

const CHSV LedRingNode::coldColor = CHSV(HUE_BLUE, 255, 255);
const CHSV LedRingNode::warmColor = CHSV(HUE_RED, 255, 255);


LedRingNode::LedRingNode():HomieNode("LEDRing", "LEDRing") {
	// TODO Auto-generated constructor stub

}

LedRingNode::~LedRingNode() {
	// TODO Auto-generated destructor stub
}

void LedRingNode::setup() {
	FastLED.addLeds<NEOPIXEL, 14>(leds,24);
}

void LedRingNode::onReadyToOperate() {
}

bool LedRingNode::handleInput(const String& property, const HomieRange& range,
		const String& value) {
}

void LedRingNode::redraw() {
	//TODO: Implement
}

void LedRingNode::showTemp(uint16_t temp) {
	if (temp < minShownTemp) temp = minShownTemp;
	if (temp > maxShownTemp) temp = maxShownTemp;

	uint_fast8_t pixAmount = (temp-minShownTemp) / tempPerPixel;
	uint_fast8_t hueStep = (warmColor.hue - coldColor.hue) / tempPerPixel;

	CHSV colorStep = coldColor;
	for (uint_fast8_t i = 0 ; i < pixAmount ; i ++) {
		colorStep.hue += hueStep;
		leds[i] = colorStep;
	}
//	fill_rainbow(leds, pixAmount, coldColor.hue, hueStep);

	FastLED.show();
}

void LedRingNode::showSetTemp(uint16_t temp) {
	uint_fast8_t idx = (temp-minShownTemp) / tempPerPixel;
	uint_fast8_t remain = (temp-minShownTemp) % tempPerPixel;
	if (idx >= 13) {
		LN.logf(__PRETTY_FUNCTION__, LoggerNode::ERROR, "idx [%d] out of bounds [0-12] for set-temp %d", idx, temp);
		return;
	}
	leds[idx] = (remain < 3) ? CRGB::Yellow : CRGB::Orange;

}

void LedRingNode::showTime() {
}
