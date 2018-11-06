#include "rdm630.h"
#include "Keyboard.h"

#define CO2_TX 8
#define CO2_RX 6

rdm630 rfid(CO2_TX, CO2_RX);  //TX-pin of RDM630 connected to Arduino pin 6

unsigned long lastCardId = 0L;
unsigned long lastReadTime;

void setup()
{
    Serial.begin(9600);
    Keyboard.begin();
    rfid.begin();

    lastReadTime = millis();

    //while(!Serial);
    //Serial.println("RFID-Leser");
    
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

        unsigned long currentTime = millis();

        if (result != lastCardId || currentTime - lastReadTime > 10000 ) {

          lastCardId = result;    
          lastReadTime = currentTime;

          String RFID_UID = "0000000000" + String(result);
          RFID_UID = RFID_UID.substring(RFID_UID.length()-10);
          Keyboard.println(RFID_UID);

        }

    }

}


