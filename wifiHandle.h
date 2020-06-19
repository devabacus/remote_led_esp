// wifiHandle.h

#ifndef _WIFIHANDLE_h
#define _WIFIHANDLE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "WiFi.h"
#include "Ble.h"
#include "ota.h"

extern WiFiClient wifiClient;

void WiFiEvent(WiFiEvent_t event);
void wifiBegin(void);

#endif

