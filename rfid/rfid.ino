#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>

#include <ArduinoJson.h>

#include <SPI.h>
#include <MFRC522.h>

#include <ESP8266HTTPClient.h>

#define RST_PIN 20
#define SS_PIN 2
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory;
MFRC522::MIFARE_Key key;

HTTPClient client;
HTTPClient clientLock;
const String hostLockBackup = "192.168.2.104";
String hostLock = "";

ESP8266WebServer server(80);

const char* fingerprint = "dc a6 d8 b8 0a ea 7e 0b 32 77 14 05 2f 27 b2 90 32 b7 7e b2";

//ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  for(byte i=0; i<6; i++) {
    key.keyByte[i] = 0xFF;
  }


  SPI.begin();
  mfrc522.PCD_Init();

  WiFiManager wifiManager;
  wifiManager.autoConnect();

  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // HostSNname default to esp8266-[ChipId]
  ArduinoOTA.setHostname("RFIDMCU");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well

  // ArduinoOTA.setPasswordHash(xxx);

  server.on("/lockIP", HTTP_GET, handleGetLockIp);
  server.on("/lockIP", HTTP_POST, handlePostLockIp);
  server.begin();
  Serial.println("ESP8266WebServer started");


  ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
  type == "sketch";
      } else {
  type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating" + type);
    });

  ArduinoOTA.onEnd([]() {
      Serial.println("\nEND");
    });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / ( total / 100 )));
    });

  ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
  Serial.println("Auth failed");
      } else if (error == OTA_BEGIN_ERROR) {
  Serial.println("Begin failed");
      } else if (error == OTA_CONNECT_ERROR) {
  Serial.println("Connect failed");
      } else if (error == OTA_RECEIVE_ERROR) {
  Serial.println("Receive failed");
      } else if (error == OTA_END_ERROR) {
  Serial.println("End failed");
      }
    });

  ArduinoOTA.begin();
  Serial.println("Ready for RFID reading");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());      
}

void loop() {

  ArduinoOTA.handle();
  readCardID();
  server.handleClient();

}

void readCardID() {

  String cardId = "";
  boolean isFinished = false;

  yield();

  //--------------------

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Card Detected:**"));

  //====================

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); // dump some details about the card

  long code = 0;
  for (byte i = 0; i < mfrc522.uid.size; i++ ) {
    code = ((code + mfrc522.uid.uidByte[i]) * 10 );
  }

  cardId = String(code);

  //====================


  mfrc522.PICC_HaltA();
  Serial.println(F("\n**End Reading**\n"));

  isFinished = true;
  yield();

  Serial.print("ID: ");
  Serial.println(cardId);


  String url = "https://poc9.osnow.de/api/x_buergerbox/buergerboxrestapi/buergerbox/kartenid/" + cardId;
  client.begin(url, fingerprint);
  client.setAuthorization("bm9kZU1DVToxMTEx");

  int httpCode =  client.GET();

  Serial.print("Statuscode: ");
  Serial.println(httpCode);

  if (httpCode <= 0 ) {
    Serial.println("Unexpected failure communicatig to lock.");
  } else {

    StaticJsonBuffer<200> jsonBuffer;
    String payload = client.getString();
    Serial.println(payload);

    if (httpCode != HTTP_CODE_OK) {

      Serial.println("Error from ServiceNow");
      Serial.println(payload);
      
    } else {

      JsonObject& root = jsonBuffer.parseObject(payload);
  
      const char* success = root["result"]["success"];
      int boxid = root["result"]["boxid"];

      Serial.print("Got answer from ServiceNow, boxid: ");
      Serial.println(boxid);


      //==============================
      //if(!openLock(boxid)) {
  //Serial.println("Error opening lock");
  //Serial.println(payload);
      //} else {
  releaseBox(boxid, cardId);
      //}
      
      delay(1000);

    }
  }
}
      
boolean openLock(int boxid) {

  Serial.println();
  Serial.println("Opening lock");

  String host = hostLockBackup;
  if (hostLock != "" ){
    host =  hostLock;
  }
  String urlLock = "http://" + host + "/lock/open?number=" + String(boxid);
  Serial.print("Openeing url: ");
  Serial.println(urlLock);
  clientLock.begin(urlLock);
  int httpLockCode = clientLock.GET();

  Serial.print("httpLockCode: ");
  Serial.println(httpLockCode);
  if (httpLockCode <= 0) {
    Serial.println("Unexpected failure communicatig to lock.");
    return false;
  } else {

    if (httpLockCode != HTTP_CODE_OK) {
      Serial.println("Could not open lock");
      return false;
    } else {

      Serial.println("Successfully opened lock");
      return true;

    }
  }
}


boolean releaseBox(int boxid, String cardId) {

  Serial.println();
  Serial.print("Releasing lock for box ");
  Serial.print(boxid);
  Serial.print(" and cardId " );
  Serial.println(cardId);

  String url = "https://poc9.osnow.de/api/x_buergerbox/buergerboxrestapi/buergerbox/box/" + String(boxid);
  url = url + "/kartenid/" + cardId;
  Serial.println(url);
  client.begin(url, fingerprint);
  client.setAuthorization("bm9kZU1DVToxMTEx");

  int httpCode =  client.GET();

  Serial.print("Statuscode: ");
  Serial.println(httpCode);

  if (httpCode <= 0 ) {
    Serial.println("Unexpected failure communicatig to ServiceNow.");
  } else {

    StaticJsonBuffer<200> jsonBuffer;
    String payload = client.getString();
    Serial.println(payload);


    if (httpCode != HTTP_CODE_OK) {

      Serial.println("Error from ServiceNow");
      Serial.println(payload);
      
    } else {

      JsonObject& root = jsonBuffer.parseObject(payload);
  
      const char* success = root["result"]["success"];
      //int boxid = root["result"]["boxid"];

      Serial.print("Got answer from ServiceNow, success: ");
      Serial.println(success);
      Serial.println();
    }
  }
}


void handleGetLockIp() {

  char html[2000];
  char hostChar[15];

  hostLock.toCharArray(hostChar, 15);

  sprintf(html,
    "<html>\
<link rel='stylesheet' href='//maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'><link rel='stylesheet' href='//maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'><link rel='stylesheet' href='//maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'>\
<body>\
<h1>Lock IP</h1>\
%s\
<form action='/lockIP' method='POST'>\
<div class='form-group'>\
<label for='ip'>Lock IP:</label>\
<input type='text' name='ip'></br>\
</div>\
<button class='btn btn-primary' type='submit'>Submit</button>\
</form>\
</body></html>", hostChar);

  server.send(200, "text/html", html);

}

void handlePostLockIp() {

  if (server.args() > 0) {
    hostLock = server.arg("ip");
    Serial.print("Setting hostLock to: ");
    Serial.println(hostLock);
  }

  handleGetLockIp();
}
