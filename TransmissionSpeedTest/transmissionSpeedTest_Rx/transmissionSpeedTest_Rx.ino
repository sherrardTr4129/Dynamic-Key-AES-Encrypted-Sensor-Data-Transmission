#include <aes.h>
#include <aes128_dec.h>
#include <aes128_enc.h>
#include <aes192_dec.h>
#include <aes192_enc.h>
#include <aes256_dec.h>
#include <aes256_enc.h>
#include <AESLib.h>
#include <aes_dec.h>
#include <aes_enc.h>
#include <aes_invsbox.h>
#include <aes_keyschedule.h>
#include <aes_sbox.h>
#include <aes_types.h>
#include <bcal-basic.h>
#include <bcal-cbc.h>
#include <bcal-cmac.h>
#include <bcal-ofb.h>
#include <bcal_aes128.h>
#include <bcal_aes192.h>
#include <bcal_aes256.h>
#include <blockcipher_descriptor.h>
#include <gf256mul.h>
#include <keysize_descriptor.h>
#include <memxor.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "StringSplitter.h"

// define and macros
#define BNO055_SAMPLERATE_DELAY_MS (30)

// instantiate radio object
RF24 radio(9, 10); // CE, CSN

char ackText[32] = "";

void setup() {

  Serial.begin(115200);

  // setup NRF24L01 as reciever
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setRetries(2, 8);
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength(RF24_CRC_16);

  byte addresses[][6] = {"1Node", "2Node"};
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);
  radio.startListening();

  radio.writeAckPayload(1, &ackText, 32);
}

void loop(void)
{
  byte pipeNo = 0;
  // see if data is available within radio buffer
  if (radio.available(&pipeNo))
  {
    // create buffers for text data
    char text[32] = "";
    // read data
    radio.read(&text, sizeof(text));

    // write data back
    radio.writeAckPayload(pipeNo, &text, sizeof(text));
    Serial.println("got data");
    radio.flush_rx();
  }
}
