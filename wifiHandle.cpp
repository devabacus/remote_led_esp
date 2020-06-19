// 
// 
// 

#include "wifiHandle.h"

Preferences wifiPrefs;

WiFiClient wifiClient = 0;

wifi_t wifi;

const long  gmtOffset_sec = 18000;
const int   daylightOffset_sec = 18000;
const char* ntpServer = "pool.ntp.org";

int epoch_time = 0;
timezone utc = { 0, 0 };
const timezone* tz = &utc;

void WiFiEvent(WiFiEvent_t event) {
	switch (event) {
	case SYSTEM_EVENT_STA_CONNECTED:
		Serial.println("STA Connected");
		WiFi.enableIpV6();
		break;
	case SYSTEM_EVENT_AP_STA_GOT_IP6:
		Serial.print("STA IPv6: ");
		Serial.println(WiFi.localIPv6());
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		Serial.print("STA IPv4: ");
		Serial.println(WiFi.localIP());
		//use_wifi = true;//ToDo : do it in right way
		//reconnect(); //MQTT
		if (pWiFiCharacteristic != NULL)
		{
			pWiFiCharacteristic->setNotifyProperty(true);
			pWiFiCharacteristic->setValue("Successfull Connect");
			pWiFiCharacteristic->notify();
			delay(100);
			ESP.restart();//ToDo : do it in right way
		}
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		Serial.println("STA Disconnected");
		if (wifiSetup)
		{
			pWiFiCharacteristic->setNotifyProperty(true);
			pWiFiCharacteristic->setValue("Wrong pass or ssid");
			pWiFiCharacteristic->notify();
			WiFi.disconnect(true, true);

			//ToDo : do it in right way
			wifiPrefs.begin("wifi");
			wifiPrefs.remove("ssid");
			wifiPrefs.end();
		}
		else
		{
			WiFi.disconnect(true, true);
		}
		//if (wifi.ssid != NULL)//ToDo : do it in right way
		//{
		//	if (wifi.isHotspot)
		//	{
		//		wifiPrefs.begin("wifi");
		//		wifiPrefs.remove("wifi");
		//		wifiPrefs.end();
		//		ESP.restart();
		//	}
		//}
		break;
	case SYSTEM_EVENT_STA_STOP:
		Serial.println("STA Stopped");
		break;
	default:
		break;
	}
}

void wifiBegin(void) {

	WiFi.onEvent(WiFiEvent);

	wifiPrefs.begin("wifi");
	Serial.println(wifiPrefs.getBytesLength("ssid") > 0 ? "WiFi settings are present" : "WiFi is`t configured");
	if (wifiPrefs.getBytesLength("ssid") > 0) {
		char buf[50];
		memset(buf, 0, 50);
		wifiPrefs.getBytes("ssid", buf, 50);
		wifi.ssid = String(buf);
		memset(buf, 0, 50);
		wifiPrefs.getBytes("pass", buf, 50);
		wifi.pass = String(buf);
		wifi.isHotspot = wifiPrefs.getBool("isHotspot");
		wifiPrefs.end();
		

		Serial.println("USE WIFI = " + String(wifi.isHotspot == true ? "Y" : "N"));
		Serial.println("SSID = " + wifi.ssid);
		Serial.println("PASS = " + wifi.pass);


		if (wifi.isHotspot)//only for ota now
		{
			char data[1000];

			memset(data, 0, sizeof(data));

			bool found = false;

			int n = WiFi.scanNetworks();
			Serial.println("scan done");
			if (n == 0) {
				Serial.println("no networks found");
			}
			else {
				for (int i = 0; i < n; ++i) {
					sprintf(data, "%s%d:%s (%d)%c\r\n", data, i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? ' ' : '*');
					if (wifi.ssid == WiFi.SSID(i)) {
						Serial.println("found");
						found = true;
					}
				}
			}
			Serial.println(data);

			if (found) {

				WiFi.begin(wifi.ssid.c_str(), wifi.pass.c_str());

				// Wait for connection to establish
				while (WiFi.status() != WL_CONNECTED) {
				}
				Serial.println("");
				Serial.println("Connected to " + wifi.ssid);

				configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

#if USE_FLASH
				timeIsAccurate = true;
#endif // USE_FLASH



				// Execute OTA Update
				OTAInit(&wifiClient);

				execOTA();

				WiFi.disconnect(true, true);
			}
			else {
				Serial.println("Network not found");

				wifiPrefs.begin("time");
				epoch_time = wifiPrefs.getInt("time");
				wifiPrefs.end();

#if USE_FLASH
				timeIsAccurate = false;
#endif // USE_FLASH

				timeval epoch = { epoch_time, 0 };
				const timeval* tv = &epoch;
				settimeofday(tv, tz);
			}

		}

	}

}

