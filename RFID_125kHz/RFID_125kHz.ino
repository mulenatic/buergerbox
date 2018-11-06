#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <ArduinoJson.h>

#include "rdm630.h"

#include <ESP8266HTTPClient.h>


#define CO2_TX D2
#define CO2_RX D3

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
//const char* ssid = "HUAWEI-4832";
//const char* password = "47059578";
const char* ssid = "WLAN-6LJK5P";
const char* password = "7330401080787168";

rdm630 rfid(CO2_TX, CO2_RX);  //TX-pin of RDM630 connected to Arduino pin 6

unsigned long lastCardId = 0L;
unsigned long lastReadTime;

void setup()
{
    Serial.begin(9600);  // start serial to PC
    rfid.begin();

    lastReadTime = millis();

    Serial.println('RFID-Reader initialized');

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

void loop()
{
    byte data[6];
    byte length;

    if(rfid.available()){
        rfid.getData(data,length);

        // Die RFID besteht aus den Bytes 1 bis 4 im Daten Array von 0 bis 5
        unsigned long result = 
          ((unsigned long int)data[1]<<24) + 
          ((unsigned long int)data[2]<<16) + 
          ((unsigned long int)data[3]<<8) + 
          data[4];              
        Serial.print("Karten-ID: ");
        Serial.println(result);

        
        unsigned long currentTime = millis();
        //Serial.print(lastReadTime);
        //Serial.print(" ");
        //Serial.println(currentTime);

        if (result != lastCardId || currentTime - lastReadTime > 5000 ) {

          lastCardId = result;    
          lastReadTime = currentTime;

          String RFID_UID = "0000000000" + String(result);
          RFID_UID = RFID_UID.substring(RFID_UID.length()-10);


          communicateWithSNandOpenLock(RFID_UID);
        
        }
    }

    server.handleClient();    
    
}

void communicateWithSNandOpenLock(String cardId) {

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



