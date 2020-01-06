/*
 * ThermostatNode.cpp
 *
 *  Created on: 25.11.2017
 *      Author: ian
 */

#include <ThermostatNode.h>
#include <LoggerNode.h>

ThermostatNode::ThermostatNode():
	HomieNode("thermostat", "Thermostat", "thermostat"),
	setTemp(225),
	actTemp(999),
	mode(Auto_OFF)
{
	Serial.begin(74880);
	Serial.println(__PRETTY_FUNCTION__);
	Serial.flush();

	for (uint_fast8_t id=ThermostatNode::Manual_ON; id < ThermostatNode::LAST; id++) {
		modeFormat += mode_id[id];
		modeFormat += ':';
	}
	modeFormat.remove(modeFormat.length()-1); // remove trailing ':'
	advertise("SetTemp").settable().setName("Set Temperature").setDatatype("float").setUnit("°C");
	advertise("ActTemp").settable().setName("Current Temperature").setDatatype("float").setUnit("°C");
	advertise("Mode").settable().setName("Heating Mode").setDatatype("enum").setFormat(modeFormat.c_str());
}

bool ThermostatNode::handleInput(const HomieRange& range, const String& property, const String& value) {
	if (property.equals("SetTemp"))
	{
		float newSetTemp = value.toFloat();
		if (newSetTemp < 15 || newSetTemp > 30) {
			LN.logf(__PRETTY_FUNCTION__, LoggerNode::ERROR, "Received invalid value for property SetTemp: %s", value.c_str());
			return false;
		}
		setTemp = static_cast<int16_t> (newSetTemp * 10);
		LN.logf(__PRETTY_FUNCTION__, LoggerNode::INFO, "Updated SetTemp to %d°dC.", setTemp);
		updateSetTemp();
		return true;
	} else if (property.equals("ActTemp")) {
		float newActTemp = value.toFloat();
		if (newActTemp < 0 || newActTemp > 100) {
			LN.logf(__PRETTY_FUNCTION__, LoggerNode::ERROR, "Received invalid value for property ActTemp: %s", value.c_str());
			return false;
		}
		actTemp = static_cast<int16_t> (newActTemp * 10);
		LN.logf(__PRETTY_FUNCTION__, LoggerNode::INFO, "Updated ActTemp to %d°dC.", setTemp);
		updateActTemp();
		return true;
	} else if (property.equals("Mode")){
		LN.logf(__PRETTY_FUNCTION__, LoggerNode::INFO, "New Mode %s.", value.c_str());
		for (uint_fast8_t id=ThermostatNode::Manual_ON; id < ThermostatNode::LAST; id++) {
			if (value.equalsIgnoreCase(mode_id[id])) {
				mode = static_cast<EThermostatMode>(id);
				updateMode();
				return true;
			}
		}
		LN.logf(__PRETTY_FUNCTION__, LoggerNode::WARNING, "mode '%s' invalid", value.c_str());
		return false;
	}
	else
	{
		LN.logf(__PRETTY_FUNCTION__, LoggerNode::ERROR, "Received invalid property %s with value %s.", property.c_str(), value.c_str());
		return false;
	}
}

void ThermostatNode::updateSetTemp() {
	if(setTemp < 150) setTemp = 150;
	if(setTemp > 300) setTemp = 300;
	const String setTempStr(static_cast<float>(setTemp)/10);
	setProperty("SetTemp").setRetained(true).send(setTempStr);
	if (onSetTempChangedFct) onSetTempChangedFct(setTemp);
}

void ThermostatNode::updateActTemp() {
	if(actTemp < 0) actTemp = 0;
	if(actTemp > 1000) actTemp = 999;
	const String actTempStr(static_cast<float>(actTemp)/10);
	setProperty("ActTemp").setRetained(true).send(actTempStr);
	if (onActTempChangedFct) onActTempChangedFct(setTemp);
}


void ThermostatNode::updateMode() {
	setProperty("Mode").setRetained(true).send(mode_id[mode]);
	if (onModeChangedFct) onModeChangedFct(mode);
}

void ThermostatNode::setOnModeChangedFct(std::function<void(EThermostatMode mode)> onModeChangedFct) {
	this->onModeChangedFct = onModeChangedFct;
}

void ThermostatNode::setOnSetTempChangedFct(std::function<void(int16_t temp)> onTempChangedFct) {
	this->onSetTempChangedFct = onTempChangedFct;
}

void ThermostatNode::setOnActTempChangedFct(std::function<void(int16_t temp)> onTempChangedFct) {
	this->onActTempChangedFct = onTempChangedFct;
}

