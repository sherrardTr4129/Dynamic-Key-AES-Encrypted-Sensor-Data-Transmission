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
#include "DHT.h"

// defines and macros
#define BNO055_SAMPLERATE_DELAY_MS (30)
#define KEY_ARRAY_SIZE              16
#define DHTPIN                      4
#define DHTTYPE                     DHT11 
#define TX_DONE_PIN                 3

Adafruit_BNO055 bno = Adafruit_BNO055(55);
DHT dht(DHTPIN, DHTTYPE);

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

// setup buffer for recived text
char textX[128] = "";
char keyModCharArr1[128] = "";
char keyModCharArr2[128] = "";

// setup key for AES encryption
uint8_t key[] = {34, 45, 77, 20, 24, 48, 63, 46, 73, 99, 57, 81, 03, 47, 85, 11};
uint8_t keyModArr[] = {34, 45, 77, 20, 24, 48, 63, 46, 73, 99, 57, 81, 03, 47, 85, 11};

// setup control variables
String CommandStr = "";
float tempF = 0;
float humidity = 0;
float HeatIndexF = 0;

void modArray(uint8_t keyArr[])
{
  for (int i = 0; i < KEY_ARRAY_SIZE; i++)
  {
    keyArr[i] = random(0, 99);
  }
}

String ArrToString(uint8_t keyArr[])
{
  String toReturnStr = "{";
  for (int i = 0; i < KEY_ARRAY_SIZE; i++)
  {
    toReturnStr += String(keyArr[i]);
    if(i != KEY_ARRAY_SIZE - 1)
    {
      toReturnStr += ",";
    }
  }
  toReturnStr += "}";
  return toReturnStr;
}

void setup() {
  
  //setup serial
  Serial.begin(9600);

  // set status pin
  pinMode(TX_DONE_PIN, OUTPUT);

  // setup NRF24L01 radio
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  // setup DHT11 Sensor
  dht.begin();

  Serial.println("TX SIDE: ");
  Serial.println("Serial Command Menu: ");
  Serial.println("\'read\': reads a set of values from the INS");
  Serial.println("\'send\': sends an encrypted version of the sensor value triple to the RX side");
  Serial.println("\'rotate\': Generates new AES key, sends it to RX side, then updates TX key");
}

void loop()
{
  // keep status pin high
  digitalWrite(TX_DONE_PIN, HIGH);
  while (Serial.available())
  {
    delay(5);
    char c = Serial.read();
    CommandStr += c;
  }
  CommandStr.trim();

  if (CommandStr.equals("read"))
  {
    Serial.println("reading from sensor");
    tempF = dht.readTemperature(true);
    humidity = dht.readHumidity();
    HeatIndexF = dht.computeHeatIndex(tempF, humidity);
    Serial.print("Got Values of Temp: ");
    Serial.print(tempF);
    Serial.print(" F, Humidity: ");
    Serial.print(humidity);
    Serial.print(" %, Heat_Index: ");
    Serial.print(HeatIndexF);  
    Serial.println(" F");
  }

  else if (CommandStr.equals("send"))
  {
    Serial.println("sending message");
    String textStrX = "DT: " + String(tempF) + " H: " + String(humidity) + " I: " + String(HeatIndexF);
    textStrX.toCharArray(textX, sizeof(textX));

    // encrypt character arrays using AES
    aes128_enc_single(key, textX);

    // send encrypted text through radio
    digitalWrite(TX_DONE_PIN, LOW);
    radio.write(&textX, sizeof(textX));
    digitalWrite(TX_DONE_PIN, HIGH);
  }
  else if (CommandStr.equals("rotate"))
  {
    // perform key rotation
    memcpy(keyModArr, key, sizeof(key)); // make copy
    modArray(keyModArr);
    String keyString = ArrToString(keyModArr);
    Serial.print("new key generated: ");
    Serial.println(keyString);

    // split string for encyption purposes
    String sub1 = "K" + keyString.substring(0, 25);
    String sub2 = "J" + keyString.substring(25);

    sub1.toCharArray(keyModCharArr1, sizeof(keyModCharArr1));
    sub2.toCharArray(keyModCharArr2, sizeof(keyModCharArr2));

    // encrypt character arrays using AES
    aes128_enc_single(key, keyModCharArr1);
    aes128_enc_single(key, keyModCharArr2);

    // send encrypted text through radio
    digitalWrite(TX_DONE_PIN, LOW);
    bool oneDone = radio.write(&keyModCharArr1, sizeof(keyModCharArr1));
    delay(1);
    bool twoDone = radio.write(&keyModCharArr2, sizeof(keyModCharArr2));
    digitalWrite(TX_DONE_PIN, HIGH);
    delay(1);

    if(oneDone && twoDone)
    {
      // modify key if radio transmission successful
      memcpy(key, keyModArr, sizeof(keyModArr));
    }
  }
  CommandStr = "";
}
