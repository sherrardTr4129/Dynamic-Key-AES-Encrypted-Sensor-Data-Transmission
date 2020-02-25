#include <SPI.h>
#include "RF24.h"

RF24 radio(9, 10);
uint32_t counter = 0;

void setup() {

  Serial.begin(115200);
  Serial.println(F("RECEIVER SPEED TEST"));

  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(2, 8);
  radio.setAutoAck(true);
  radio.enableDynamicPayloads();
  radio.setCRCLength(RF24_CRC_16);

  radio.openWritingPipe();
  radio.openReadingPipe();
  radio.startListening();
}

void loop(void)
{

}
  
