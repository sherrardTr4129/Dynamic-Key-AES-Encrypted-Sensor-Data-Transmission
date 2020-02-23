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
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55);

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

// setup buffer for recived text
char text[128] = "";

// setup key for AES encryption
const uint8_t key[] = {34, 45, 77, 20, 24, 48, 63, 46, 73, 99, 57, 81, 03, 47, 85, 11};

void setup() {
  //setup serial
  Serial.begin(9600);
  
  // initialize BNO055 Sensor
  if(!bno.begin())
  {
    // sensor could not be found, loop forever
    Serial.print("No BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  delay(1000);
  bno.setExtCrystalUse(true);

  // setup NRF24L01 radio
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();      
}
void loop()
{  
  // get new sensor event instance
  sensors_event_t event; 
  bno.getEvent(&event);

  // get calibration information
  uint8_t sys, gyro, accel, mag = 0;
  bno.getCalibration(&sys, &gyro, &accel, &mag);

  // grab sensor orientations
  float Xorientation = (float)event.orientation.x;
  float Yorientation = (float)event.orientation.y;
  float Zorientation = (float)event.orientation.z;

  // construct message string and convert it to a character array
  String textStr = "X:" + String(Xorientation) + "," + "Y:" + String(Yorientation)+ "," + "Z:" + String(Zorientation);
  textStr.toCharArray(text, sizeof(text));

  // encrypt character array using AES
  aes128_enc_single(key, text);

  // send encrypted text through radio
  radio.write(&text, sizeof(text));
  delay(100);
}
