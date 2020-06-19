// dispaly.h

#ifndef _DISPALY_h
#define _DISPALY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <u8g2_fonts.h>
#include <SPI.h>
#include <FastLED.h>
#include <gamma.h>
#include <Framebuffer_GFX.h>
#include <Adafruit_GFX.h>
#include <SmartMatrix_GFX.h>
#include <SmartMatrix3.h>

extern TaskHandle_t dispalyTaskHandle;
static TimerHandle_t clearTimerHandle;




/// SmartMatrix Defines
#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
#define kMatrixWidth  64       // known working: 32, 64, 96, 128
#define kMatrixHeight 16*2       // known working: 16, 32, 48, 64
static const uint8_t kRefreshDepth = 24;       // known working: 24, 36, 48
static const uint8_t kDmaBufferRows = 2;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
static const uint8_t kPanelType = SMARTMATRIX_HUB12_16ROW_32COL_MOD4SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
static const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_HUB12_MODE);      // see http://docs.pixelmatix.com/SmartMatrix for options
static const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);


typedef struct {
	int brightness;
	int activeTime;
}matrixSettings_t;

static const int brightness = (1 * 255) / 100;       // dim: 15% brightness

#define mw kMatrixWidth
#define mh kMatrixHeight/2
#define NUMMATRIX (kMatrixWidth*kMatrixHeight)

static U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

static CRGB* matrixleds;

class BrightnessCharacteristicCallbacks : public BLECharacteristicCallbacks {
	void onRead(BLECharacteristic* pCharacteristic);
	void onWrite(BLECharacteristic* pCharacteristic);
};


class ActiveTimeCharacteristicCallbacks : public BLECharacteristicCallbacks {
	void onRead(BLECharacteristic* pCharacteristic);
	void onWrite(BLECharacteristic* pCharacteristic);
};


void dispalyBegin();


#endif

