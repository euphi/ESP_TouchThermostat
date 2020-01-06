/*
 * TouchCtrl.cpp
 *
 *  Created on: 19.02.2017
 *      Author: ian
 */

#include "TouchCtrl.h"
#include "MPR121.h"
#include "LoggerNode.h"


HomieSetting<bool> TouchCtrl::printSerial ("PrintTouch", "flag, if to print raw touch data to Serial");

TouchCtrl::TouchCtrl(): serialPrintData(true) {
	Serial.println(__PRETTY_FUNCTION__);
}

void TouchCtrl::setup() {
	    if(!MPR121.begin(MPR121_I2C_ADDRESS)){
	      Serial.println("error setting up MPR121");
	      switch(MPR121.getError()){
	        case NO_ERROR:
	          Serial.println("no error");
	          break;
	        case ADDRESS_UNKNOWN:
	          Serial.println("incorrect address");
	          break;
	        case READBACK_FAIL:
	          Serial.println("readback failure");
	          break;
	        case OVERCURRENT_FLAG:
	          Serial.println("overcurrent on REXT pin");
	          break;
	        case OUT_OF_RANGE:
	          Serial.println("electrode out of range");
	          break;
	        case NOT_INITED:
	          Serial.println("not initialised");
	          break;
	        default:
	          Serial.println("unknown error");
	          break;
	      }
	    } else {
	    	Serial.println("MPR121 initialized");
	    }

	    MPR121.setInterruptPin(14);

	    // this is the touch threshold - setting it low makes it more like a proximity trigger
	    // default value is 40 for touch
	    MPR121.setTouchThreshold(40);

	    // this is the release threshold - must ALWAYS be smaller than the touch threshold
	    // default value is 20 for touch
	    MPR121.setReleaseThreshold(30);

	    MPR121.setProxMode(PROX0_3);

//	    MPR121_settings_t settings;
//	    settings.ECR = 0xCC;		//Auto-Config
//	    settings.ACCR0 = 0x3F;		//Auto-Config
//
//		//settings.ACCR1 = 0x80;
//		settings.USL = 0x80;			//Upper Limit
//		settings.TL =  0x70;			//Target Limit
//		settings.LSL = 0x40;			//Lower Limit
//		MPR121.applySettings(&settings);
//	    delay(1000);

	    // initial data update
	    MPR121.updateTouchData();
}

void TouchCtrl::loop() {
	static uint32_t next_read = 0;
	if (millis() > next_read) {
		readRawInputs();
		next_read = millis() + 125;
	}
}

void TouchCtrl::readRawInputs(){
	static uint8_t count = 0;

    if(MPR121.touchStatusChanged()) MPR121.updateTouchData();
    MPR121.updateBaselineData();
    MPR121.updateFilteredData();

	if (serialPrintData && (count % 4 == 0)) LogTouchDataToSerial();
	if ((count % 128 == 0) && LN.loglevel(LoggerNode::DEBUG)) LogTouchData();
	count++;
}

void TouchCtrl::LogTouchDataToSerial() {
	Serial.printf("Error %x: OORS1: %x\tOORS2: %x\n", MPR121.getError(),
			MPR121.getRegister(MPR121_OORS1), MPR121.getRegister(MPR121_OORS2));
	Serial.print("TOUCH: ");
	for (uint_fast8_t i = 0; i < 13; i++) {
		// 13 touch values
		Serial.print(MPR121.getTouchData(i), DEC);
		if (i < 12)
			Serial.print(" ");
	}
	Serial.println();
	Serial.print("FDAT: ");
	for (uint_fast8_t i = 0; i < 13; i++) {
		// 13 filtered values
		Serial.print(MPR121.getFilteredData(i), DEC);
		if (i < 12)
			Serial.print(" ");
	}
	Serial.println();
	Serial.print("BVAL: ");
	for (uint_fast8_t i = 0; i < 13; i++) {
		// 13 baseline values
		Serial.print(MPR121.getBaselineData(i), DEC);
		if (i < 12)
			Serial.print(" ");
	}
	Serial.println();
	Serial.print("DIFF: ");
	for (uint_fast8_t i = 0; i < 13; i++) {
		// 13 value pairs
		Serial.print(MPR121.getBaselineData(i) - MPR121.getFilteredData(i),
				DEC);
		if (i < 12)
			Serial.print(" ");
	}
	Serial.println();
}

void TouchCtrl::LogTouchData() {
	String jsstring = "{\n\t\"Error\": ";
	jsstring += MPR121.getError();
	jsstring += ",\n\t\"FDAT:\": {";
	for (uint_fast8_t i = 0; i < 13; i++) {
		// 13 touch values
		jsstring += MPR121.getFilteredData(i);
		;
		if (i < 12)
			jsstring += ", ";
	}
	jsstring += "},\n\t\"BVAL:\": {";
	for (uint_fast8_t i = 0; i < 13; i++) {
		// 13 touch values
		jsstring += MPR121.getBaselineData(i);
		;
		if (i < 12)
			jsstring += ", ";
	}
	jsstring += "}\n}\n";
	LN.log("Touchdata", LoggerNode::DEBUG, jsstring);
}


