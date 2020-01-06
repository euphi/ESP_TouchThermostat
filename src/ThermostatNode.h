/*
 * ThermostatNode.h
 *
 *  Created on: 25.11.2017
 *      Author: ian
 */

#ifndef SRC_THERMOSTATNODE_H_
#define SRC_THERMOSTATNODE_H_

#include <HomieNode.hpp>
#include <functional>

class ThermostatNode: public HomieNode {
public:
	enum EThermostatMode {Manual_ON = 0, Manual_OFF, Auto_ON, Auto_OFF, LAST};
private:
	// Data format is fixed point (1 bit = 1/10)
	int16_t setTemp;
	int16_t	actTemp;
	void updateSetTemp();
	void updateActTemp();

	const String mode_id[LAST] = { "Manual_ON", "Manual_OFF", "Auto_ON", "Auto_OFF" };
	EThermostatMode mode;
	String modeFormat;

	void updateMode();

	std::function<void(int16_t temp)> onSetTempChangedFct;
	std::function<void(int16_t temp)> onActTempChangedFct;
	std::function<void(EThermostatMode mode)> onModeChangedFct;

public:
	ThermostatNode();
	int16_t getSetTemp() const {return setTemp;}
	int16_t getActTemp() const {return actTemp;}
	EThermostatMode getMode() const {return mode;}
	void increase() {
		setTemp += 1;
		updateSetTemp();
	}
	void decrease() {
		setTemp -= 1;
		updateSetTemp();
	}

	void setOnModeChangedFct(std::function<void(EThermostatMode mode)> onModeChangedFct);
	void setOnSetTempChangedFct(std::function<void(int16_t temp)> onSetTempChangedFct);
	void setOnActTempChangedFct(std::function<void(int16_t temp)> onActTempChangedFct);


protected:
//	  virtual void setup() {}
//	  virtual void loop() {}
//	  virtual void onReadyToOperate() {}
	virtual bool handleInput(const HomieRange& range, const String& property, const String& value) override;

};

#endif /* SRC_THERMOSTATNODE_H_ */
