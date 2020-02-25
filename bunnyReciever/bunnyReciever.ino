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

// setup radio address array
const byte address[6] = "00001";

// setup key for aes encryption
const uint8_t key[] = {34, 45, 77, 20, 24, 48, 63, 46, 73, 99, 57, 81, 03, 47, 85, 11};

bool seenX = false;
bool seenY = false;
bool seenZ = false;

// create empty string objects for incomming data after it is parsed
String X_ValString = "";
String Y_ValString = "";
String Z_ValString = "";
String ZeroChar = "0";
String AccelCalib = "3";

void setup() {
  // start serial communication
  Serial.begin(115200);

  // setup NRF24L01 as reciever
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}
void loop()
{
  // see if data is available within radio buffer
  if (radio.available())
  {

    // create buffers for text data
    char text[128] = "";

    // grab data from radiobuffer
    radio.read(&text, sizeof(text));

    // print encrypted message
    Serial.print(F("Encrypted Message: "));
    Serial.println(text);

    // decrypt message
    aes128_dec_single(key, text);

    Serial.print(F("De-Encrypted Message: "));
    Serial.println(text);

    // create string object from de-encrypted text
    String stringObj = String(text);

    if (text[0] == 'X')
    {
      seenX = true;
      X_ValString = stringObj.substring(2);
    }

    else if (text[0] == 'Y')
    {
      seenY = true;
      Y_ValString = stringObj.substring(2);
    }

    else if (text[0] == 'Z')
    {
      seenZ = true;
      Z_ValString = stringObj.substring(2);
    }

    if (seenX && seenY && seenZ)
    {
      // print data
      Serial.print(F("Orientation: "));
      Serial.print(X_ValString);
      Serial.print(F(" "));
      Serial.print(Y_ValString);
      Serial.print(F(" "));
      Serial.print(Z_ValString);
      Serial.println(F(""));

      Serial.print(F("Calibration: "));
      Serial.print(ZeroChar);
      Serial.print(F(" "));
      Serial.print(ZeroChar);
      Serial.print(F(" "));
      Serial.print(AccelCalib);
      Serial.print(F(" "));
      Serial.println(ZeroChar);

      // reset flags
      seenX = false;
      seenY = false;
      seenZ = false;

      // reset string values to empty
      X_ValString = "";
      Y_ValString = "";
      Z_ValString = "";
    }
  }
  delay(BNO055_SAMPLERATE_DELAY_MS);
}
