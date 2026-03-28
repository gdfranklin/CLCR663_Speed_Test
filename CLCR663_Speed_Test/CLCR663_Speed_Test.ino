/*
  Simple CLCR663 Reader Motion Testing
  Used on ESP32 WROOM 38pin
  Geoff Franklin
  March 27, 2026

  Use library: https://github.com/tueddy/CLRC663-Library
*/

#include <CLRC663.h>

// Define CLCR NFC Reader 
#define RST 25
#define IRQ_PIN 14
#define CHIP_SELECT 27

CLRC663 reader(&SPI, CHIP_SELECT, IRQ_PIN);

void setup() {
  // hold the SD (Shutdown) pin low 
  pinMode(RST, OUTPUT);
  digitalWrite(RST, LOW);

  // Serial interface for debugging only
  Serial.begin(115200);
  while (!Serial);        // Wait for serial
  delay(2000);            // Wait more for serial
  
  Serial.println("\nCLCR663 Speed Test " );
  Serial.println("Geoff Franklin" );
  Serial.println(__FILE__);
  Serial.println(__DATE__);    
  Serial.println();

  reader.begin();
  
  // get version
  Serial.print("CLRC663 version: ");
  Serial.println(reader.getVersion());

  // set up reader for ISO15693
  reader.softReset();  
  reader.AN1102_recommended_registers(MFRC630_PROTO_ISO15693_1_OF_4_SSC); 

  Serial.println("Ready");
}

void loop(){
  static int readCount = 0;           // number of consecutive valid reads
  static int sequence = 1;            // tag seen count
  static unsigned long firstSeenTime; // start time of a series of good reads   
  uint8_t uid[10]={0};                // variable for 10byte UID
  uint8_t uid_len;                    // length of returned UID
  static int noTagCount = 0;          // number of no tag reads since the last good read
  static int totalNoTagCount = 0;     // number of no tags ingnored since the first good read

  unsigned long commandStartTime = millis();
  uid_len = reader.read_iso18693_uid(uid); // tag read function
  unsigned long ElapsedTime = (millis() - commandStartTime);  

  if (uid_len != 0) {                 // Successful read?
    noTagCount = 0;                   // reset the no tag count
    readCount++;                      
    if (readCount==1) {               // first successful read?
      firstSeenTime = millis();
      totalNoTagCount = 0;            // reset total no tag count 
      static char UID[50];    // display the tag info      
      sprintf(UID, "%02X %02X %02X %02X %02X %02X %02X %02X", uid[7],uid[6],uid[5],uid[4],uid[3],uid[2],uid[1],uid[0]);
      Serial.printf("Sequence: %d, UID: %s, Read Time: %dms\n", sequence++, UID, ElapsedTime);
    }
   } else {                           // No tag in range
    noTagCount++;
    totalNoTagCount++;
    if (readCount>0) {                // have we seen a tag
      if (noTagCount > 4) {           // wait for a few few noTag returns
        unsigned long timeSeen = (millis() - firstSeenTime);    
        Serial.printf("Total Time Seen: %d mSec. reads: %d, No Tag Count: %d\n\n", timeSeen, readCount, totalNoTagCount);                 
        readCount = 0;                  // reset
      }
    }
  } 
  
  delay(10);
}
