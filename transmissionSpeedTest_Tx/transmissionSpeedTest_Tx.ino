#include <SPI.h>
#include "RF24.h"

RF24 radio(9, 10);

void setup()
{
  Serial.begin(115200);
  Serial.println(F("TRANSMITTER SPEED TEST"));

  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(2, 8);
  radio.setAutoAck(true);
  radio.enableDynamicPayloads();
  radio.setCRCLength(RF24_CRC_16);

  byte addresses[][6] = {"1Node", "2Node"};

}

void loop(void)
{

}
  
