// 
// 
// 

#include "ota.h"

#define UPD "UPDATE"
#define VERSION_CODE "V0.07"
#define VERSION 7


static Preferences prefs;

void OTAInit(WiFiClient* _pWiFiClient)
{
    pWiFiClient = _pWiFiClient;

    current_version = VERSION;
}

// Utility to extract header value from headers
String getHeaderValue(String header, String headerName) {
    return header.substring(strlen(headerName.c_str()));
}

// OTA Logic 
void execOTA() {
    Serial.println("Connecting to: " + String(host));
    // Connect to S3
    if (pWiFiClient->connect(host.c_str(), port)) {
        // Connection Succeed.
        // Fecthing the bin
        Serial.println("Fetching Bin: " + String(txt));

        // Get the contents of the bin file
        pWiFiClient->print(String("GET ") + txt + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "Cache-Control: no-cache\r\n" +
            "Connection: close\r\n\r\n");

        // Check what is being sent
        //    Serial.print(String("GET ") + bin + " HTTP/1.1\r\n" +
        //                 "Host: " + host + "\r\n" +
        //                 "Cache-Control: no-cache\r\n" +
        //                 "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (pWiFiClient->available() == 0) {
            if (millis() - timeout > 5000) {
                Serial.println("Client Timeout !");
                pWiFiClient->stop();
                return;
            }
        }
        // Once the response is available,
        // check stuff

        /*
           Response Structure
            HTTP/1.1 200 OK
            x-amz-id-2: NVKxnU1aIQMmpGKhSwpCBh8y2JPbak18QLIfE+OiUDOos+7UftZKjtCFqrwsGOZRN5Zee0jpTd0=
            x-amz-request-id: 2D56B47560B764EC
            Date: Wed, 14 Jun 2017 03:33:59 GMT
            Last-Modified: Fri, 02 Jun 2017 14:50:11 GMT
            ETag: "d2afebbaaebc38cd669ce36727152af9"
            Accept-Ranges: bytes
            Content-Type: application/octet-stream
            Content-Length: 357280
            Server: AmazonS3

            {{BIN FILE CONTENTS}}

        */
        while (pWiFiClient->available()) {
            // read line till /n
            String line = pWiFiClient->readStringUntil('\n');
            // remove space, to check if the line is end of headers
            line.trim();

            // if the the line is empty,
            // this is end of headers
            // break the while and feed the
            // remaining `wifiClient` to the
            // Update.writeStream();
            if (!line.length()) {
                //headers ended
                break; // and get the OTA started
            }

            // Check if the HTTP Response is 200
            // else break and Exit Update
            if (line.startsWith("HTTP/1.1")) {
                if (line.indexOf("200") < 0) {
                    Serial.println("Got a non 200 status code from server. Exiting OTA Update.");
                    break;
                }
            }

            // extract headers here
            // Start with content length
            if (line.startsWith("Content-Length: ")) {
                contentLength = atol((getHeaderValue(line, "Content-Length: ")).c_str());
                Serial.println("Got " + String(contentLength) + " bytes from server");
            }

            // Next, the content type
            if (line.startsWith("Content-Type: ")) {
                String contentType = getHeaderValue(line, "Content-Type: ");
                Serial.println("Got " + contentType + " payload.");
                if (contentType == "text/plain") {
                    isValidContentType = true;
                }
            }
        }
    }
    else {
        // Connect to S3 failed
        // May be try?
        // Probably a choppy network?
        Serial.println("Connection to " + String(host) + " failed. Please check your setup");
        // retry??
        // execOTA();
    }
    // Check what is the contentLength and if content type is `application/octet-stream`
    Serial.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));

    // check contentLength and content type
    if (contentLength && isValidContentType) {

        String line = pWiFiClient->readString();

        Serial.println(line);

        versionOnServer = line.toDouble();
        Serial.println("Current version is: " + String(current_version));
        Serial.println("Version on server is " + String(versionOnServer));
        

        if (!(versionOnServer > current_version)) {
            Serial.println("Current versoin are the same as in the server. No update are required");
            
            xTaskNotify(dispalyTaskHandle, (uint32_t)&VERSION_CODE, eSetValueWithoutOverwrite);
            return;
        }
        else
        {
            xTaskNotify(dispalyTaskHandle, (uint32_t)&UPD, eSetValueWithoutOverwrite);
            Serial.println("Current versoin are older than in the server. Update are required");
            prefs.end();
            prefs.begin("version");
            prefs.putInt("version", versionOnServer);
            Serial.println("Version in flash: " + String(prefs.getInt("version")));
            prefs.end();
        }


    }
    else {
        Serial.println("There was no content in the response");
        return;
    }

    isValidContentType = false;

    if (pWiFiClient->connect(host.c_str(), port)) {
        // Connection Succeed.
        // Fecthing the bin
        Serial.println("Fetching Bin: " + String("firmware_") + String(versionOnServer) + String(".bin"));

        // Get the contents of the bin file
        pWiFiClient->print(String("GET ") + String("/firmware_") + String(versionOnServer) + String(".bin") + " HTTP/1.1\r\n" +
            "Host: " + host + "\r\n" +
            "Cache-Control: no-cache\r\n" +
            "Connection: close\r\n\r\n");

        // Check what is being sent
        //    Serial.print(String("GET ") + bin + " HTTP/1.1\r\n" +
        //                 "Host: " + host + "\r\n" +
        //                 "Cache-Control: no-cache\r\n" +
        //                 "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (pWiFiClient->available() == 0) {
            if (millis() - timeout > 5000) {
                Serial.println("Client Timeout !");
                pWiFiClient->stop();
                return;
            }
        }
        // Once the response is available,
        // check stuff

        /*
           Response Structure
            HTTP/1.1 200 OK
            x-amz-id-2: NVKxnU1aIQMmpGKhSwpCBh8y2JPbak18QLIfE+OiUDOos+7UftZKjtCFqrwsGOZRN5Zee0jpTd0=
            x-amz-request-id: 2D56B47560B764EC
            Date: Wed, 14 Jun 2017 03:33:59 GMT
            Last-Modified: Fri, 02 Jun 2017 14:50:11 GMT
            ETag: "d2afebbaaebc38cd669ce36727152af9"
            Accept-Ranges: bytes
            Content-Type: application/octet-stream
            Content-Length: 357280
            Server: AmazonS3

            {{BIN FILE CONTENTS}}

        */
        while (pWiFiClient->available()) {
            // read line till /n
            String line = pWiFiClient->readStringUntil('\n');
            Serial.println(line);
            // remove space, to check if the line is end of headers
            line.trim();

            // if the the line is empty,
            // this is end of headers
            // break the while and feed the
            // remaining `wifiClient` to the
            // Update.writeStream();
            if (!line.length()) {
                //headers ended
                break; // and get the OTA started
            }

            // Check if the HTTP Response is 200
            // else break and Exit Update
            if (line.startsWith("HTTP/1.1")) {
                if (line.indexOf("200") < 0) {
                    Serial.println("Got a non 200 status code from server. Exiting OTA Update.");
                    break;
                }
            }

            // extract headers here
            // Start with content length
            if (line.startsWith("Content-Length: ")) {
                contentLength = atol((getHeaderValue(line, "Content-Length: ")).c_str());
                Serial.println("Got " + String(contentLength) + " bytes from server");
            }

            // Next, the content type
            if (line.startsWith("Content-Type: ")) {
                String contentType = getHeaderValue(line, "Content-Type: ");
                Serial.println("Got " + contentType + " payload.");
                if (contentType == "application/octet-stream") {
                    isValidContentType = true;
                }
            }
        }
    }
    else {
        // Connect to S3 failed
        // May be try?
        // Probably a choppy network?
        Serial.println("Connection to " + String(host) + " failed. Please check your setup");
        // retry??
        // execOTA();
    }

    // Check what is the contentLength and if content type is `application/octet-stream`
    Serial.println("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));

    // check contentLength and content type
    if (contentLength && isValidContentType) {
        // Check if there is enough to OTA Update
        bool canBegin = Update.begin(contentLength);

        // If yes, begin
        if (canBegin) {
            Serial.println("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
            // No activity would appear on the Serial monitor
            // So be patient. This may take 2 - 5mins to complete
            size_t written = Update.writeStream(*pWiFiClient);

            if (written == contentLength) {
                Serial.println("Written : " + String(written) + " successfully");
            }
            else {
                Serial.println("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
                // retry??
                // execOTA();
            }

            if (Update.end()) {
                Serial.println("OTA done!");
                if (Update.isFinished()) {
                    Serial.println("Update successfully completed. Rebooting.");

                    ESP.restart();
                }
                else {
                    Serial.println("Update not finished? Something went wrong!");
                }
            }
            else {
                Serial.println("Error Occurred. Error #: " + String(Update.getError()));
            }
        }
        else {
            // not enough space to begin OTA
            // Understand the partitions and
            // space availability
            Serial.println("Not enough space to begin OTA");
            pWiFiClient->flush();
        }
    }
    else {
        Serial.println("There was no content in the response");
        pWiFiClient->flush();
    }
}
