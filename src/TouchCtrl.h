/*
 * TouchCtrl.h
 *
 *  Created on: 19.02.2017
 *      Author: ian
 */

#ifndef SRC_TOUCHCTRL_H_
#define SRC_TOUCHCTRL_H_

#include "Homie.hpp"

#define MPR121_I2C_ADDRESS 0x5A // 0x5A - 0x5D

class TouchCtrl {

public:
	TouchCtrl();
	void setup();
	void loop();

	void setSerialPrintData(bool on) {
		serialPrintData = on;
	}

	static HomieSetting<bool> printSerial;
private:
	void readRawInputs();
	void LogTouchDataToSerial();
	void LogTouchData();

	bool serialPrintData;
};

#endif /* SRC_TOUCHCTRL_H_ */
