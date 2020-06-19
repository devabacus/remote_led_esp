#include "Ble.h"


TaskHandle_t BLETaskHandle;
TaskHandle_t BateryTaskHandle;

volatile SemaphoreHandle_t BaterySemaphore;

TimerHandle_t BateryTimerHandle;

BLERemoteCharacteristic* pRemoteCharacteristic = NULL;
BLECharacteristic* pWeightCharacteristic = NULL;
BLECharacteristic* pWiFiCharacteristic = NULL;
BLECharacteristic* pBrightnessCharacteristic = NULL;
BLECharacteristic* pActiveTimeCharacteristic = NULL;
BLECharacteristic* pFlashCharacteristic = NULL;
BLEAdvertising* pAdvertising = NULL;
BLEScan* pBLEScan = NULL;

BLEServer* pServer = NULL;

BLEClient* pClient = NULL;

bool connectionStarted = false;
bool scanStarted = false;

boolean centralIsConnected = false;
boolean doScan = false;

boolean connected = false;

boolean wifiSetup = false;

Preferences blePrefs;

char bleWeight[20];

static std::string value;

class MyCallbacks : public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic* pCharacteristic) {
		value = pCharacteristic->getValue();

		if (value.length() > 0) {
			strcpy(bleWeight, pCharacteristic->getValue().c_str());
			xTaskNotify(dispalyTaskHandle, uint32_t(bleWeight), eSetValueWithoutOverwrite);
		}
	}
};

class AsPeripheralCallback : public BLEServerCallbacks {

	void onConnect(BLEServer* pServer) {
		Serial.println("onConnect as perifperal");
		xTaskNotify(dispalyTaskHandle, (uint32_t) & "CONN", eSetValueWithoutOverwrite);
	}

	void onDisconnect(BLEServer* pServer) {
		Serial.println("onDisconnect as perifperal");
		xTaskNotify(dispalyTaskHandle, (uint32_t) & "DISCON", eSetValueWithoutOverwrite);
	}

};



String getValue(String data, char separator, int index) {
	int found = 0;
	int strIndex[] = { 0, -1 };
	int maxIndex = data.length() - 1;

	for (int i = 0; i <= maxIndex && found <= index; i++) {
		if (data.charAt(i) == separator || i == maxIndex) {
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

class WiFiCharacteristicCallbacks : public BLECharacteristicCallbacks {
	void onRead(BLECharacteristic* pCharacteristic) {
		char data[1000];

		memset(data, 0, sizeof(data));

		int n = WiFi.scanNetworks();
		Serial.println("scan done");
		if (n == 0) {
			Serial.println("no networks found");
		}
		else {
			for (int i = 0; i < n; ++i) {
				sprintf(data, "%s%d:%s (%d)%c\r\n", data, i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? ' ' : '*');
			}
		}
		Serial.println(data);

		pCharacteristic->setValue(data);
	}
	void onWrite(BLECharacteristic* pCharacteristic) {
		std::string value = pCharacteristic->getValue();

		if (value.length() > 0) {
			Serial.println("*********");
			Serial.print("New value: ");

			char str[100];

			for (int i = 0; i < value.length(); i++) {
				Serial.print(value[i]);
				str[i] = value[i];
				str[i + 1] = '\0';
			}

			Serial.println();
			Serial.println("*********");


			wifi_t wifi;
			String ssids = getValue(String(str), ' ', 0);
			String passs = getValue(String(str), ' ', 1);
			String useWifi = getValue(String(str), ' ', 2);


			wifi.isHotspot = String("Y") == useWifi ? true : false;

			Serial.println(ssids);
			Serial.println(passs);
			Serial.println(wifi.isHotspot == true ? "Y" : "N");


			blePrefs.end();
			blePrefs.begin("wifi");
			blePrefs.remove("wifi");
			blePrefs.putBytes("ssid", ssids.c_str(), strlen(ssids.c_str()));
			blePrefs.putBytes("pass", passs.c_str(), strlen(passs.c_str()));
			blePrefs.putBool("isHotspot", wifi.isHotspot);
			blePrefs.end();

			wifiSetup = true;

			WiFi.begin(ssids.c_str(), passs.c_str());

			//ToDo: get rid of Strings 
		}
	}
};



void bleBegin()
{
	BLEDevice::init("WDMini");
	BLEServer* pServer = BLEDevice::createServer();

	pServer->setCallbacks(new AsPeripheralCallback);

	///////////////////////////////////////////////////////////////////////////

	BLEService* pService = pServer->createService(SERVICE_UUID);

	BLECharacteristic* pCharacteristic = pService->createCharacteristic(
		CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ |
		BLECharacteristic::PROPERTY_WRITE
	);

	pCharacteristic->setCallbacks(new MyCallbacks());
	pService->start();

	///////////////////////////////////////////////////////////////////////////

	BLEService* pSettingsService = pServer->createService(SettingsServiceUUID);

	pWiFiCharacteristic = pSettingsService->createCharacteristic(
		WifiCharacterisitcUUID,
		BLECharacteristic::PROPERTY_NOTIFY |
		BLECharacteristic::PROPERTY_WRITE |
		BLECharacteristic::PROPERTY_READ
	);

	pWiFiCharacteristic->setCallbacks(new WiFiCharacteristicCallbacks());
	pWiFiCharacteristic->addDescriptor(new BLE2902());

	pBrightnessCharacteristic = pSettingsService->createCharacteristic(
		BrightnessCharacterisitcUUID,
		BLECharacteristic::PROPERTY_WRITE |
		BLECharacteristic::PROPERTY_READ
	);

	pBrightnessCharacteristic->setCallbacks(new BrightnessCharacteristicCallbacks());

	pActiveTimeCharacteristic = pSettingsService->createCharacteristic(
		ActiveTimeCharacterisitcUUID,
		BLECharacteristic::PROPERTY_WRITE |
		BLECharacteristic::PROPERTY_READ
	);

	pActiveTimeCharacteristic->setCallbacks(new ActiveTimeCharacteristicCallbacks());


	pSettingsService->start();

	///////////////////////////////////////////////////////////////////////////

	BLEAdvertising* pAdvertising = pServer->getAdvertising();
	pAdvertising->addServiceUUID(SERVICE_UUID);
	pAdvertising->start();
}

