// 
// 
// 

#include "dispaly.h"


SMARTMATRIX_ALLOCATE_BUFFERS(matrixLayer, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);


TaskHandle_t dispalyTaskHandle = 0;
static TimerHandle_t testTimerHandle;

Preferences matrixPrefs;

matrixSettings_t matrixSettings;

static void show_callback();

static SmartMatrix_GFX matrix(matrixleds, mw, mh, show_callback);

// Sadly this callback function must be copied around with this init code
static void show_callback() {
	backgroundLayer.swapBuffers(true);
	matrixleds = (CRGB*)backgroundLayer.backBuffer();
	matrix.newLedsPtr(matrixleds);
}

static void dispalyTask(void* pParam) {
	int width = 0;

	static char* pStr;

	char* _pStr;

	for (;;) {
		if (xTaskNotifyWait(0, ULONG_MAX, (uint32_t*)&_pStr, portMAX_DELAY) == pdTRUE) {
			pStr = _pStr;
			matrix.clear();
			u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso16_tf);
			width = u8g2_for_adafruit_gfx.getUTF8Width(pStr);
			u8g2_for_adafruit_gfx.setCursor(64 - width, 16);
			u8g2_for_adafruit_gfx.print(pStr);
			matrix.show();
			xTimerReset(clearTimerHandle, 0);
		}
	}

}

void clearTimerCallback(TimerHandle_t xTimer) {
	matrix.clear();
	matrix.show();
}

void testTimerCallback(TimerHandle_t xTimer) {
	matrix.clear();
	matrix.show();
}



void dispalyBegin()
{
	matrixPrefs.begin("matrixSettings");
	if (matrixPrefs.getBytesLength("matrixSettings") > 0)
	{
		matrixPrefs.getBytes("matrixSettings", &matrixSettings, matrixPrefs.getBytesLength("matrixSettings"));
		Serial.println(matrixSettings.brightness);
		Serial.println(matrixSettings.activeTime);
	}
	else
	{
		matrixSettings.brightness = 255;
		matrixSettings.activeTime = 60000;
		matrixPrefs.putBytes("matrixSettings", &matrixSettings, sizeof(matrixSettings_t));
	}	
	if (matrixSettings.brightness == 0 || matrixSettings.activeTime == 0)
	{
		matrixSettings.brightness = 255;
		matrixSettings.activeTime = 60000;
		matrixPrefs.putBytes("matrixSettings", &matrixSettings, sizeof(matrixSettings_t));
	}
	matrixPrefs.end();

	matrixLayer.addLayer(&backgroundLayer);
	matrixLayer.begin();
	matrixLayer.setBrightness(matrixSettings.brightness);
	// This sets the neomatrix and LEDMatrix pointers
	show_callback();
	matrixLayer.setRefreshRate(24);
	backgroundLayer.enableColorCorrection(true);

	backgroundLayer.fillScreen({ 0x80, 0x80, 0x80 });
	backgroundLayer.swapBuffers();

	matrix.begin();

	u8g2_for_adafruit_gfx.begin(matrix);
	u8g2_for_adafruit_gfx.setFontMode(1);
	u8g2_for_adafruit_gfx.setFontDirection(0);            // left to right (this is default)
	u8g2_for_adafruit_gfx.setForegroundColor(matrix.Color(255, 0, 0));      // apply Adafruit GFX color
	u8g2_for_adafruit_gfx.setFont(u8g2_font_logisoso16_tf);  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
	matrix.clear();
	matrix.show();

	clearTimerHandle = xTimerCreate("clearTimer", pdMS_TO_TICKS(matrixSettings.activeTime), pdTRUE, (void*)0, clearTimerCallback);
	testTimerHandle  = xTimerCreate("testTimer", pdMS_TO_TICKS(500), pdFALSE, (void*)1, testTimerCallback);

	xTimerStart(clearTimerHandle, 0);
	xTimerStart(testTimerHandle, 0);

	xTaskCreatePinnedToCore(dispalyTask, "dispalyTask", configMINIMAL_STACK_SIZE + 200, NULL, 1, &dispalyTaskHandle, 1);
}

void BrightnessCharacteristicCallbacks::onRead(BLECharacteristic* pCharacteristic)
{
	pCharacteristic->setValue(String(lroundf((100.0/255.0) * matrixSettings.brightness)).c_str());
}

void BrightnessCharacteristicCallbacks::onWrite(BLECharacteristic* pCharacteristic)
{
	matrixSettings.brightness = ((atoi(pCharacteristic->getValue().c_str()) * 255) / 100);
	matrixPrefs.begin("matrixSettings");
	matrixPrefs.putBytes("matrixSettings", &matrixSettings, sizeof(matrixSettings_t));
	matrixPrefs.end();

	matrixLayer.setBrightness(matrixSettings.brightness);
}

void ActiveTimeCharacteristicCallbacks::onRead(BLECharacteristic* pCharacteristic)
{
	pCharacteristic->setValue(String(matrixSettings.activeTime/1000).c_str());
}

void ActiveTimeCharacteristicCallbacks::onWrite(BLECharacteristic* pCharacteristic)
{
	matrixSettings.activeTime = atoi(pCharacteristic->getValue().c_str()) * 1000;
	matrixPrefs.begin("matrixSettings");
	matrixPrefs.putBytes("matrixSettings", &matrixSettings, sizeof(matrixSettings_t));
	matrixPrefs.end();

	xTimerChangePeriodFromISR(clearTimerHandle, pdMS_TO_TICKS(matrixSettings.activeTime), NULL);
}
