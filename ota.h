// ota.h

#ifndef _OTA_h
#define _OTA_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "WiFi.h"
#include <Preferences.h>
#include "Update.h"
#include "dispaly.h"

// S3 Bucket Config
static String host = "led-scale.s3.eu-central-1.amazonaws.com"; // Host => bucket-name.s3.region.amazonaws.com
static int port = 80; // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
//String bin = "/firmware.bin"; // bin file name with a slash in front.
static String txt = "/version.txt";

// response from S3
static long contentLength = 0;
static int versionOnServer = 0;
static bool isValidContentType = false;
static int current_version = 0;

static WiFiClient* pWiFiClient;


void OTAInit(WiFiClient* _pWiFiClient);
void execOTA();

#endif

