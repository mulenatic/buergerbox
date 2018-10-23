#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const byte pin_realais_1 = D5;
const byte pin_realais_2 = D6;
const byte pin_realais_3 = D7;
const byte pin_realais_4 = D8;

//const char* ssid = "buergerbox";
//const char* password = "t-systems";
const char* ssid = "HUAWEI-4832";
const char* password = "47059578";


ESP8266WebServer server(80);


void handleLockOpenGet() {

  String backArg = server.arg("back");
  bool isNotBack = (backArg == "");

  String numberArg = server.arg("number");
  Serial.print("Opening lock number ");
  Serial.println(numberArg);
  if ( numberArg == "" ) {
    Serial.println("Number not given");
    server.send(409, "textjson", "{ \"success\": \"false\" }");
  } else {

    int pin = -1;
    if (numberArg == "1" ) {
      pin = pin_realais_1;
    } else if (numberArg == "2") {
      pin = pin_realais_2;
    } else if (numberArg == "3" ) {
      pin = pin_realais_3;
    } else if (numberArg == "4" ) {
      pin = pin_realais_4;
    }


    if ( pin == -1 ) {
      Serial.println("Only number 1-4 are valid lockids");
      isNotBack ? server.send(409, "textjson", "{ \"success\": \"false\" }") : handleRoot();
    } else {
      digitalWrite(pin, LOW);
      delay(5000);
      digitalWrite(pin, HIGH);

      Serial.println("Successfully opened lock");
      isNotBack ? server.send(200, "textjson", "{ \"success\": \"true\" }") : handleRoot();
    }
  }
}

void handleRoot() {
  char temp[2000];
  snprintf(temp, 2000, "<!doctype html>\
<html>\
  <head>\
     <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">\
    <title>Manuelles Entriegeln der Boxen</title>\
    <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\">\
    <style>.btn-custom { color: white; background-color: #E20074; border-color: #5bc2c2 }</style>\
  </head>\
  <body>\
  <div class='container'>\
    <div class='jumbotron'>\
      <h3>Klicken sie auf einen der untenstehenden Buttons um eine Box zu entriegeln</h3>\
        <div class='row justify-content-center'>\
          <div class='col-8 p-2'><a href='/lock/open?number=1&back=true' class=\"btn btn-custom btn-lg w-100\">Box #1</a></div>\
          <div class='col-8 p-2'><a href='/lock/open?number=2&back=true' class=\"btn btn-custom btn-lg w-100\">Box #2</a></div>\
          <div class='col-8 p-2'><a href='/lock/open?number=3&back=true' class=\"btn btn-custom btn-lg w-100\">Box #3</a></div>\
          <div class='col-8 p-2'><a href='/lock/open?number=4&back=true' class=\"btn btn-custom btn-lg w-100\">Box #4</a></div>\
        </div>\
      </div>\
    </div>\
  </body>\
</html>");
  server.send(200, "text/html", temp);
}


void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  pinMode(pin_realais_1, OUTPUT);
  pinMode(pin_realais_2, OUTPUT);
  pinMode(pin_realais_3, OUTPUT);
  pinMode(pin_realais_4, OUTPUT);

  digitalWrite(pin_realais_1, HIGH);
  digitalWrite(pin_realais_2, HIGH);
  digitalWrite(pin_realais_3, HIGH);
  digitalWrite(pin_realais_4, HIGH);

  // Port defaults to 8266
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.hostname("arudinoLock");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/lock/open", HTTP_GET, handleLockOpenGet);
  server.begin();
  Serial.println("ESP8266WebServer started");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  server.handleClient();

}
