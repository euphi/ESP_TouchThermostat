
// used Libraries
#include <Homie.hpp>		// Homie-ESP8266: Homie MQTT Convention for ESP8266
#include <LoggerNode.h>		// Log to MQTT (Homie)
#include <SensorNode.h>		// HTU21D Sensor
#include <Automaton.h>		// Automaton State Machine

// own extensions
#include "ThermostatNode.h" // MQTT (Homie) connection for Thermostat
#include "LedMatrixNode.h"	// MQTT (Homie) control for 8x8 Matrix (brightness, color)
#include "TouchCtrl.h"		// MPR121 Touch Interface
#include "AtmDisplayMode.h"	// Display State Machine (Automaton)
#include "AtmTouchButton.h" // Automaton Button for Touch Input


#define FW_NAME "ESP_TouchThermostat"
#define FW_VERSION "2.99.6"


// ****** HomieNodes ******
SensorNode sensor;
LedMatrixNode matrixNode;
ThermostatNode thermo;
LoggerNode LN;

// ****** own controller objects ******
TouchCtrl touch;  // TODO: Make this class a singleton and reference it in Atm_TouchButton only
Atm_DisplayMode atm_disp(sensor, thermo, matrixNode);  // State machine for LED matrix

// ****** own input elements ******
Atm_TouchButton button_up;
Atm_TouchButton button_down;
Atm_TouchButton button_left;
Atm_TouchButton button_right;

enum  ETouchButton {BUT_LEFT = 0, BUT_UP, BUT_RIGHT, BUT_DOWN, BUT_ENTER}; // PINout of Touchcontroller


void setup() {
	// Initialize Serial
	Serial.begin(74880);
	Serial.println(FW_NAME " " FW_VERSION);
	Serial.flush();

	// Initialize I2C
	Wire.begin(SDA, SCL);
	Wire.setClockStretchLimit(2000);
	delay(100);  // settle I2C

	// Initialize Homie
	Homie_setFirmware(FW_NAME, FW_VERSION);
	Homie.disableLedFeedback();
	Homie.disableResetTrigger();
	Homie.setBroadcastHandler([](const String& level, const String& value) {
		LN.logf(__PRETTY_FUNCTION__,LoggerNode::DEBUG, "Broadcast: %s: %s", level.c_str(), value.c_str());
		if (level.equalsIgnoreCase("time")) {
			atm_disp.setCurTime(value.toInt());
			return true;
		}
		return false;
	});
	Homie.setup();

	TouchCtrl::printSerial.setDefaultValue(false);
	touch.setSerialPrintData(TouchCtrl::printSerial.get());
	Serial.print("PrintTouch: ");
	Serial.println(TouchCtrl::printSerial.get() ? "true":"false");
	touch.setSerialPrintData(true);
	// Initialize State Machine
	atm_disp.begin();				// Initialize main display state machine
	atm_disp.trace(Serial);			// Trace State transitions to Serial

	// ---> Callbacks Inc and Dec
	atm_disp.onInc([]( int idx, int v, int up ) {thermo.increase();},0);
	atm_disp.onDec([]( int idx, int v, int up ) {thermo.decrease();},0);

	thermo.setOnSetTempChangedFct([](int16_t newTemp) {atm_disp.show_settemp();});
	thermo.setOnActTempChangedFct([](int16_t newTemp) {atm_disp.redraw();});
	thermo.setOnModeChangedFct([](ThermostatNode::EThermostatMode mode) {atm_disp.redraw();});

	touch.setup();

	/* --> Connect buttons as inputs for state machine
	 *     - no debounce needed for touch (touch controller already has hysteresis)
	 *     - UP and DOWN button can repeat
	 *     - Connect Button to corresponding events of atm_disp state machine
	 *     - trace state transition to Serial
	 */
	button_up.begin(BUT_UP).debounce(0).repeat(500, 333).onPress(atm_disp, Atm_DisplayMode::EVT_BUT_UP).trace(Serial);
	button_down.begin(BUT_DOWN).debounce(0).repeat(500, 333).onPress(atm_disp, Atm_DisplayMode::EVT_BUT_DOWN).trace(Serial);
	button_left.begin(BUT_LEFT).debounce(0).onPress(atm_disp, Atm_DisplayMode::EVT_BUT_LEFT).trace(Serial);
	button_right.begin(BUT_RIGHT).debounce(0).onPress(atm_disp, Atm_DisplayMode::EVT_BUT_RIGHT).trace(Serial);
}
void loop() {
	Homie.loop();
	touch.loop();
	automaton.run();
}
