// Ble.h

#ifndef _BLE_h
#define _BLE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#include "WiFi.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Preferences.h>

#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "dispaly.h"


#define BAT_SENSE 35

// The remote service we wish to connect to.
static BLEUUID serviceUUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("6e400003-b5a3-f393-e0a9-e50e24dcca9e");

//peripheral

static BLEUUID SettingsServiceUUID("db3e25d0-79e9-48fa-a2c3-facc90a3d437");
static BLEUUID WifiCharacterisitcUUID("db3e25d1-79e9-48fa-a2c3-facc90a3d437");
static BLEUUID FlashCharacterisitcUUID("db3e25d2-79e9-48fa-a2c3-facc90a3d437");
static BLEUUID BrightnessCharacterisitcUUID("db3e25d3-79e9-48fa-a2c3-facc90a3d437");
static BLEUUID ActiveTimeCharacterisitcUUID("db3e25d4-79e9-48fa-a2c3-facc90a3d437");

#define CTS_SERVICE_UUID "1805"
#define CURRENT_TIME_CHARACTERISTIC "2A2B"

#define WEIGHT_SERVICE_UUID "181D"
#define WEIGHT_CHARACTERISTIC_UUID "2A98"

#define BATERY_SERVICE_UUID BLEUUID((uint16_t)0x180F) 

#define SERVICE_UUID        "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

typedef struct {
    bool isHotspot;
    String ssid;
    String pass;
} wifi_t;

#define CONNECTED    (1<<0)
#define DO_SCAN      (1<<1)
#define DO_CONNECT   (1<<2)
#define DO_ADV       (1<<3)

extern boolean wifiSetup;
static boolean doConnect = false;
extern boolean doScan;
static boolean doAdv = true;
extern boolean connected;
extern boolean centralIsConnected;
extern BLEScan* pBLEScan;
extern BLEClient* pClient;
static BLEAdvertisedDevice* myDevice;
extern BLEAdvertising* pAdvertising;
static BLECharacteristic* pBateryCharacteristic;
extern BLECharacteristic* pWiFiCharacteristic;
extern BLECharacteristic* pBrightnessCharacteristic;
extern BLECharacteristic* pActiveTimeCharacteristic;
extern BLECharacteristic* pFlashCharacteristic;
extern BLERemoteCharacteristic* pRemoteCharacteristic;
extern BLECharacteristic* pWeightCharacteristic;

extern boolean connected;

static BLEDescriptor BatteryLevelDescriptor(BLEUUID((uint16_t)0x2901));

static esp_adc_cal_characteristics_t* adc_chars;

void bleBegin();

bool connectToServer();

#endif

