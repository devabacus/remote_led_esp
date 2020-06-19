#include "dispaly.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <Preferences.h>
#include "WiFi.h"
#include <Update.h>
#include "ota.h"
#include "wifiHandle.h"
#include "Ble.h"

#include <U8g2_for_Adafruit_GFX.h>
#include <u8g2_fonts.h>
#include <SPI.h>
#include <FastLED.h>
#include <gamma.h>
#include <Framebuffer_GFX.h>
#include <Adafruit_GFX.h>
#include <SmartMatrix_GFX.h>
#include <SmartMatrix3.h>


Preferences prefs;



void setup() {

	Serial.begin(115200);

	dispalyBegin();

	wifiBegin();

	bleBegin();


	
}

void loop() {
	delay(1000);
}
