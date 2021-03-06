#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

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
const String hostLockBackup = "192.168.8.100";
String hostLock = "";
const String instanceURL = "https://dev51124.service-now.com";

ESP8266WebServer server(80);

const char* fingerprint = "45 8b 95 cd 4b 67 60 83 4e f7 b9 5e 65 ba d8 4f c8 27 32 cc";
const char* fingerprintLock = "DA DC FA 4C F4 1B 9B AB 0C A5 70 8B 51 3E 78 07 AD 02 3C 34";

//const char* ssid = "buergerbox";
//const char* password = "t-systems";
const char* ssid = "HUAWEI-4832";
const char* password = "47059578";


int RFID_UID_Nr = 0;
String RFID_UID = "";
int StrLen = 0;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  for(byte i=0; i<6; i++) {
    key.keyByte[i] = 0xFF;
  }


  SPI.begin();
  mfrc522.PCD_Init();

  // Port defaults to 8266
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.hostname("rfid");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  server.on("/lockIP", HTTP_GET, handleGetLockIp);
  server.on("/lockIP", HTTP_POST, handlePostLockIp);
  server.begin();
  Serial.println("ESP8266WebServer started");
  
  Serial.println("Ready for RFID reading");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  

  //client.setTimeout(20000);
  clientLock.setTimeout(10000);
}

void loop() {

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

  RFID_UID_Nr = 0;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    RFID_UID_Nr = RFID_UID_Nr * 256 + mfrc522.uid.uidByte[mfrc522.uid.size-i-1];
  }

  RFID_UID = String(RFID_UID_Nr);
  StrLen = RFID_UID.length();
  for (byte i = 0; i < 10 - StrLen; i++)
  {
     RFID_UID = "0" + RFID_UID;
  }
  Serial.print("Die ID des RFID-TAGS lautet: ");
  Serial.println(RFID_UID);

  cardId = RFID_UID;

  Serial.print("Verbesserte cardId: ");
  Serial.println(cardId);

  

  //====================


  mfrc522.PICC_HaltA();
  Serial.println(F("\n**End Reading**\n"));

  isFinished = true;
  yield();

  Serial.print("ID: ");
  Serial.println(cardId);


  String url = instanceURL + "/api/x_buergerbox/buergerboxrestapi/buergerbox/findAndRelease/kartenid/" + cardId;
  client.begin(url, fingerprint);
  client.setAuthorization("bm9kZU1DVToxMTEx");

  boolean reachedSN = false;

  while(!reachedSN) {

    int httpCode =  client.GET();
  
    Serial.print("Statuscode: ");
    Serial.println(httpCode);
  
    if (httpCode <= 0 ) {
      Serial.println("Unexpected failure communicating to ServiceNow.");
    } else {

      reachedSN = true;
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
        if(!openLock(boxid)) {
          Serial.println("Error opening lock");
          Serial.println(payload);
        } else {
          //releaseBox(boxid, cardId);
          Serial.println("Box opened");
        }
        
        delay(1000);
        client.end();
        //resetFunc();
  
      }
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

  boolean lockOpened = false;

  while(!lockOpened) {
  
    int httpLockCode = clientLock.GET();
  
    Serial.print("httpLockCode: ");
    Serial.println(httpLockCode);
    if (httpLockCode <= 0) {
      Serial.println("Unexpected failure communicatig to lock.");
      return false;
    } else {
  
      if (httpLockCode != HTTP_CODE_OK) {
        Serial.println("Could not open lock");
        
      } else {
  
        Serial.println("Successfully opened lock");
        lockOpened = true;      
  
      }
    }

  }
  clientLock.end();
  return true;
  
}


boolean releaseBox(int boxid, String cardId) {

  Serial.println();
  Serial.print("Releasing lock for box ");
  Serial.print(boxid);
  Serial.print(" and cardId " );
  Serial.println(cardId);

  String url = instanceURL + "/api/x_buergerbox/buergerboxrestapi/buergerbox/box/" + String(boxid);
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



