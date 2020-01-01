
// used Libraries
#include <Homie.hpp>		// Homie-ESP8266: Homie MQTT Convention for ESP8266
#include <LoggerNode.h>		// Log to MQTT (Homie)
#include <SensorNode.h>		// HTU21D Sensor
#include <Automaton.h>		// Automaton State Machine

// own extensions
#include "ThermostatNode.h" // MQTT (Homie) connection for Thermostat
#include "LedRingNode.h"	// MQTT (Homie) control for 8x8 Matrix (brightness, color)
#include "TouchCtrl.h"		// MPR121 Touch Interface
#include "AtmDisplayMode.h"	// Display State Machine (Automaton)
#include "AtmTouchButton.h" // Automaton Button for Touch Input


/* Magic sequence for Autodetectable Binary Upload */
#define FW_NAME "ESP_TouchThermostat"
#define FW_VERSION "0.9.5"

const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */


// ****** HomieNodes ******
SensorNode sensor;
LedRingNode ringNode;
ThermostatNode thermo;


// ****** own controller objects ******
TouchCtrl touch;  // TODO: Make this class a singleton and reference it in Atm_TouchButton only
Atm_DisplayMode atm_disp(sensor, thermo, ringNode);  // State machine for LED matrix

// ****** own input elements ******
Atm_TouchButton button_up;
Atm_TouchButton button_down;
Atm_TouchButton button_left;
Atm_TouchButton button_right;

enum  ETouchButton {BUT_LEFT = 0, BUT_UP, BUT_RIGHT, BUT_DOWN, BUT_ENTER}; // PINout of Touchcontroller


void setup() {
	// Initialize Serial
	Serial.begin(74880);
	Serial.println("Starting..");
	Serial.flush();

	// Initialize I2C
	Wire.begin(SDA, SCL);
	Wire.setClockStretchLimit(2000);
	delay(100);  // settle I2C

	TouchCtrl::printSerial.setDefaultValue(false);
	touch.setSerialPrintData(TouchCtrl::printSerial.get());
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

	// Initialize State Machine
	atm_disp.begin();				// Initialize main display state machine
	atm_disp.trace(Serial);			// Trace State transitions to Serial

	// ---> Callbacks Inc and Dec
	atm_disp.onInc([]( int idx, int v, int up ) {thermo.increase();},0);
	atm_disp.onDec([]( int idx, int v, int up ) {thermo.decrease();},0);

	thermo.setOnTempChangedFct([](int16_t newTemp) {atm_disp.show_settemp();});
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

	Serial.begin(74880);
	Serial.println("End setup..");
	Serial.flush();

}
void loop() {
	Homie.loop();
	touch.loop();
	automaton.run();
}
