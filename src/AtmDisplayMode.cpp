/*
 * AtmDisplayMode.cpp
 *
 *  Created on: 25.11.2017
 *      Author: ian
 */

#include <AtmDisplayMode.h>
#include "MPR121.h"

/* Add optional parameters for the state machine to begin()
 * Add extra initialization code
 */
Atm_DisplayMode::Atm_DisplayMode(const SensorNode & sens, const ThermostatNode& therm, LedRingNode& _ring):
		Machine(),
		ledring(_ring),
		sensor(sens),
		thermNode(therm),
		cur_time(9999)
{
	//timer_redraw.set(5000);
	timer_state_timeout.set(10000);
}

Atm_DisplayMode& Atm_DisplayMode::begin() {
  // clang-format off
  const static state_t state_table[] PROGMEM = {
    /*                   ON_ENTER  ON_LOOP  ON_EXIT  EVT_REDRAW  EVT_SHOW_SETTEMP  EVT_TIMEOUT  EVT_BUT_DOWN  EVT_BUT_UP  EVT_BUT_RIGHT  EVT_BUT_LEFT  ELSE */
    /* SHOW_TIME */ ENT_SHOW_TIME,      -1,      -1,  SHOW_TIME,         SET_TEMP,          -1,     SET_TEMP,   SET_TEMP,      SET_TEMP,    SHOW_TEMP,   -1,
    /* SHOW_TEMP */ ENT_SHOW_TEMP,      -1,      -1,  SHOW_TEMP,         SET_TEMP,          -1,     SET_TEMP,   SET_TEMP,     SHOW_TIME,     SET_TEMP,   -1,
    /*  SET_TEMP */  ENT_SET_TEMP,      -1,      -1,   SET_TEMP,         SET_TEMP,   SHOW_TEMP,     SET_TEMP,   SET_TEMP,     SHOW_TEMP,    SHOW_TIME,   -1,
  };
  // clang-format on
  Machine::begin( state_table, ELSE );
  return *this;
}

/* Add C++ code for each internally handled event (input)
 * The code must return 1 to trigger the event
 */

int Atm_DisplayMode::event( int id ) {
	switch (id) {
	case EVT_TIMEOUT:
		return timer_state_timeout.expired(this);
	}
	return 0;
}

/* Add C++ code for each action
 * This generates the 'output' for the state machine
 *
 * Available connectors:
 *   push( connectors, ON_DEC, 0, <v>, <up> );
 *   push( connectors, ON_INC, 0, <v>, <up> );
 */

void Atm_DisplayMode::action( int id ) {
  switch ( id ) {
    case ENT_SHOW_TIME:
    	showTime();
        return;
    case ENT_SHOW_TEMP:
    	showTemp();
        return;
    case ENT_SET_TEMP:
    	showSetTemp();
        return;
  }
}

void Atm_DisplayMode::setCurTime(int16_t curTime) {
	cur_time = curTime;
	redraw();
}

void Atm_DisplayMode::drawModePixel() {
	uint16_t c = 0;
	switch (thermNode.getMode()) {
	case ThermostatNode::Auto_ON:
		c = CRGB::Red;
		break;
	case ThermostatNode::Auto_OFF:
		c = CRGB::Green;
		break;
	case ThermostatNode::Manual_ON:
		c = CRGB::Yellow;  // --> yellow
		break;
	case ThermostatNode::Manual_OFF:
		c = CRGB::Blue;
		break;
	}
	LedRingNode::RingCRGB& ring = ledring.getLEDs();
	//TOODO:matrix.getMatrix().drawPixel(0,7,c);
}

void Atm_DisplayMode::showTime() {
	//TODO:matrix.setColorUp(CRGB::Green);
	//TODO:matrix.setColorDn(CRGB::Blue);
	//TODO: show4DigitNumber(cur_time);
	drawModePixel();
	ledring.redraw();
	//TODO: matrix.getMatrix().show();
}

void Atm_DisplayMode::showSetTemp() {
//TODO:	matrix.setColorUp(CRGB::Red);
//	matrix.setColorDn(CRGB::Blue);
	int16_t temp_dC = thermNode.getSetTemp();
	//TODO: show4DigitNumber(temp_dC, true);
	drawModePixel();
	ledring.redraw();
}

void Atm_DisplayMode::showTemp() {
	//matrix.setColorUp(CRGB::Green);
	//matrix.setColorDn(CRGB::Red);
	int16_t temp_dC = rint(sensor.getTemperatur() * 10);
	//TODO: show4DigitNumber(temp_dC, true);
	drawModePixel();
	//TODO:matrix.getMatrix().show();
}

/* Override the default trigger() method
 * to push Events on state transition
 */

Atm_DisplayMode& Atm_DisplayMode::trigger( int event ) {
  Machine::trigger( event );
  switch(event) {
  case EVT_BUT_DOWN:
	  if (state() == SET_TEMP) push( connectors, ON_DEC, 0, 1, 1 );
	  break;
  case EVT_BUT_UP:
	  if (state() == SET_TEMP) push( connectors, ON_INC, 0, 1, 1 );
  }
  return *this;
}


/* Nothing customizable below this line
 ************************************************************************************************
*/

/* Public event methods
 *
 */

Atm_DisplayMode& Atm_DisplayMode::redraw() {
  trigger( EVT_REDRAW );
  return *this;
}

Atm_DisplayMode& Atm_DisplayMode::show_settemp() {
  trigger( EVT_SHOW_SETTEMP );
  return *this;
}

/*
 * onDec() push connector variants ( slots 1, autostore 0, broadcast 0 )
 */

Atm_DisplayMode& Atm_DisplayMode::onDec( Machine& machine, int event ) {
  onPush( connectors, ON_DEC, 0, 1, 1, machine, event );
  return *this;
}

Atm_DisplayMode& Atm_DisplayMode::onDec( atm_cb_push_t callback, int idx ) {
  onPush( connectors, ON_DEC, 0, 1, 1, callback, idx );
  return *this;
}

/*
 * onInc() push connector variants ( slots 1, autostore 0, broadcast 0 )
 */

Atm_DisplayMode& Atm_DisplayMode::onInc( Machine& machine, int event ) {
  onPush( connectors, ON_INC, 0, 1, 1, machine, event );
  return *this;
}

Atm_DisplayMode& Atm_DisplayMode::onInc( atm_cb_push_t callback, int idx ) {
  onPush( connectors, ON_INC, 0, 1, 1, callback, idx );
  return *this;
}

/* State trace method
 * Sets the symbol table and the default logging method for serial monitoring
 */

Atm_DisplayMode& Atm_DisplayMode::trace( Stream & stream ) {
  Machine::setTrace( &stream, atm_serial_debug::trace,
    "DISPLAYMODE\0EVT_REDRAW\0EVT_SHOW_SETTEMP\0EVT_TIMEOUT\0EVT_BUT_DOWN\0EVT_BUT_UP\0EVT_BUT_RIGHT\0EVT_BUT_LEFT\0ELSE\0SHOW_TIME\0SHOW_TEMP\0SET_TEMP" );
  return *this;
}

