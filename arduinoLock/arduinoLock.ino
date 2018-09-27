#include <ESP8266WiFi.h>
#include <ESP8266WebServerSecure.h>

const byte pin_realais_1 = D5;
const byte pin_realais_2 = D6;
const byte pin_realais_3 = D7;
const byte pin_realais_4 = D8;

const char* ssid = "buergerbox";
const char* password = "t-systems";

BearSSL::ESP8266WebServerSecure server(443);


static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDXTCCAkWgAwIBAgIJAMchveZAAyuOMA0GCSqGSIb3DQEBCwUAMEUxCzAJBgNV
BAYTAkRFMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBX
aWRnaXRzIFB0eSBMdGQwHhcNMTgwODA2MTIyNTI1WhcNMjkxMDIzMTIyNTI1WjBF
MQswCQYDVQQGEwJERTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50
ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB
CgKCAQEAtojQwytSsXu/iOo1bTInCcK6y1avnBakKv2tgAsbluFpb8h1HTRN3YTx
/2KFsabzzWzOfyRkJptjuWyMn8Bn4k5KSMkcvpXh+QaMQNVz6+N5kzILmxrIkXPy
t+fm4oye1bMa6W7GG9ZjCmIjI9X/88FcoajLKGqS3Zs/dzuXRWkYzmhCz9EHW45X
BpRscCM70knpt4iLa5LqVT2HNZI8l4pVhQMNx2RbhPFQTlecp93ZDjlLX4Vgudqx
I1eiTHjsUmVIXNaYTck+1Fj4TAevolqPVB3lPWYndYvuhFdEzCoiCew+zOOgxcES
WJSRlgebCWGWpAfkDYAORv6wiwBpTwIDAQABo1AwTjAdBgNVHQ4EFgQU2zEt/hET
kUPSp4kjiur4VmhAfKcwHwYDVR0jBBgwFoAU2zEt/hETkUPSp4kjiur4VmhAfKcw
DAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAqA445b4yVJ2a4Z5Wv+IA
0dNiCYCtOQrpcgrVwgYbNf+zdsSiaiDOO51GYKzLlxsPp3q2Rsqv8GTDp+PSFmkK
Rn53FRPhJfe1iTIoyy+pBd4oztmidYOSRrC2ARhOdQPPIHHGLffz3R0FOa/gF2eC
MDU8E1NdfJriGDlJfJaRopZN69yZJi5XVRDrJj4MFhDSkuujH4UrATYb+kzdyV1m
Hgnrvg3L9/7pxA09WgfW9KHlYPMLQoTETCxiJ3CQE1ilCojo83b3sLKt/7paBfce
++t+4yxXpj2IKziTu6wLPo/YYGteNfoh/vns2J5bdfRbkyICX59BXPo5X2+LnjEs
tg==
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM = R"EOF(
-----BEGIN PRIVATE KEY-----
MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQC2iNDDK1Kxe7+I
6jVtMicJwrrLVq+cFqQq/a2ACxuW4WlvyHUdNE3dhPH/YoWxpvPNbM5/JGQmm2O5
bIyfwGfiTkpIyRy+leH5BoxA1XPr43mTMgubGsiRc/K35+bijJ7VsxrpbsYb1mMK
YiMj1f/zwVyhqMsoapLdmz93O5dFaRjOaELP0QdbjlcGlGxwIzvSSem3iItrkupV
PYc1kjyXilWFAw3HZFuE8VBOV5yn3dkOOUtfhWC52rEjV6JMeOxSZUhc1phNyT7U
WPhMB6+iWo9UHeU9Zid1i+6EV0TMKiIJ7D7M46DFwRJYlJGWB5sJYZakB+QNgA5G
/rCLAGlPAgMBAAECggEAOLCxNYxE+H5YdCtZQHqtCSkDXpejnrShdACpW/UXMnr7
hg4Z4WIXNM9PYWO15gDWA1zhnTcuK5djF3mRCyR0nyJ1i7l6Re7C6daQxKoGgKJ7
Za3LUPa2iAYcweUyHfZSdkhT+V/AA3FH/TeRdlXGUmwhWsvTnM/MoSrQQTWuu41r
i8sramPZjmneqLiRaHQWsdqj86W20EfffIgURbokQSHXU5SG5ku+fM9USr+Kd0iT
yEvOJCj4NSptEWYFO0vphl4L81iz4Jxy8Hk0wSHFXvsgBzAoliWD0Y8bXMixhdfY
vVYtglEf03oonh3iIp8sfXtbA/QWZzSVl55WKNEmgQKBgQDui/SzksSYSj3Hfs0x
v0GeYRnyjJu/gLzJzuMXYmbwFKMUmqb1Y31xnacPzjwLb+5T2aSmzjpTdSiuXK7H
0eBGK/zOuDNWq+dKJz/5X5Hmwl0Vau9fVPXfChwKdgnKlRgZcM5RnGSRyziNFxL9
vSkY/kVzyqCxpSUuG32tj7a3YQKBgQDD47wXNJsWUX3gRQ1oz9poA58gd9o5/p/l
dNNrzXQsJXPDJiEZ8IJPWH0gTSVwFE2pRC6Ivy/HiaL3Ia5USW3zeOVdU5Szl5Cp
RpcUx2aI2L1EwUI/S3JLs5G80NlPEWWD0P/OPiwr+XobDIyRJjQe7OamnlncQu/7
qVqnRynOrwKBgQCSK5RQDNkGcEeW3K4jk4xD3kAmDd7J47rK9kjU5WkqJ5seXv17
U/FkEzRxTPAiGf4xxyVLSQcYaKv627QFoScKGJ4alWhv7XoWSCK9TcB7ZBHhs9rl
0YuvleREzLuHNQDFZggmnexKIfqPCdqLPMPYGsa4Ayugbw9vR00lcvCFQQKBgQCv
GSDnCUHbqEpBLtg2qRzMPvh9WES79EtPGqdlREgksfGRLcMG4cJJVIq8VdkyHyPw
Op7x4xu9W91M1FF0zBI5QQac7noMMl2/wfaIP1j61+oOzEUAONtYqZGK2pB0qdM5
CZeu7kojoKOyC/yWPjT2s8HUi9U9GqRTKuV6gxTuxwKBgQDHLVa4q/QQv3nNPxwK
XRrwPqGugKhMtC+MNHzK2sqzlKvAUNEVAjtIip1rKOR9pxLmj4u1KEqOEyO3yfAf
BcJgUkaQ/SCzkb0TTOo0IvJuBrPvkOE3cr/YyExFOsQaYDMWHkZIZSTw3O8vbiOB
tW3MCH719OOzgSnGa41atT7v5Q==
-----END PRIVATE KEY-----
)EOF";



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

  server.setRSACert(new BearSSLX509List(serverCert), new BearSSLPrivateKey(serverKey));

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
