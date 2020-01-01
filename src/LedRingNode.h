/*
 * LedRingNode.h
 *
 *  Created on: 11.03.2018
 *      Author: ian
 */

#ifndef SRC_LEDRINGNODE_H_
#define SRC_LEDRINGNODE_H_

#include <HomieNode.hpp>
#include <FastLED.h>

class LedRingNode: public HomieNode {
public:
	typedef CRGB RingCRGB[24];

private:
	RingCRGB leds;

public:
	LedRingNode();
	virtual ~LedRingNode();

	virtual void setup();
	virtual void onReadyToOperate();
	virtual bool handleInput(const String& property, const HomieRange& range, const String& value);

	void redraw();
	void showTemp(uint16_t temp);
	void showSetTemp(uint16_t temp);
	void showTime();
	RingCRGB& getLEDs() {return leds;};

	static const uint16_t minShownTemp = 185;
	static const uint16_t maxShownTemp = 250;
	static const uint16_t tempPerPixel = (maxShownTemp - minShownTemp) / 13;
	static const CHSV coldColor;
	static const CHSV warmColor;

};

#endif /* SRC_LEDRINGNODE_H_ */
