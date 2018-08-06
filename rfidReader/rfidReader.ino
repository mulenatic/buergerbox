#include <SPI.h>
#include <MFRC522.h>
#include <Keyboard.h>

#define RST_PIN 9
#define SS_PIN 10
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
    Serial.begin(115200);
  Serial.println("Booting");

    SPI.begin();
  mfrc522.PCD_Init();

  Keyboard.begin();

  Serial.println("Ready for RFID reading");

}

void loop() {
  // put your main code here, to run repeatedly:
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

  //mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); // dump some details about the card

  long code = 0;
  for (byte i = 0; i < mfrc522.uid.size; i++ ) {
    code = ((code + mfrc522.uid.uidByte[i]) * 10 );
  }

  cardId = String(code);

  //====================



  Serial.println(F("\n**End Reading**\n"));

  isFinished = true;
  yield();

  Serial.print("ID: ");
  Serial.println(cardId);

  Keyboard.print(cardId + "\n");

  delay(5000);

}
