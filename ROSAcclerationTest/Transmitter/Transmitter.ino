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

// defines and macros
#define BNO055_SAMPLERATE_DELAY_MS (30)
#define KEY_ARRAY_SIZE              16
#define TX_DONE_PIN                 3
#define joyYaxis                    A0
#define joyXaxis                    A1
#define joyButton                   4
#define DEBOUNCE_BUFF               100

Adafruit_BNO055 bno = Adafruit_BNO055(55);

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

// setup buffer for recived text
char text1[128] = "";
char keyModCharArr1[128] = "";
char keyModCharArr2[128] = "";

// setup key for AES encryption
uint8_t key[] = {34, 45, 77, 20, 24, 48, 63, 46, 73, 99, 57, 81, 03, 47, 85, 11};
uint8_t keyModArr[] = {34, 45, 77, 20, 24, 48, 63, 46, 73, 99, 57, 81, 03, 47, 85, 11};

// setup control variables
String CommandStr = "";
int totalFailed = 0;
int totalPass = 0;

// setup variables for joystick state (past and present)
int currentButtonState = 0;
int prevButtonState = 0;
int prevButtonStateDebounced = 0;
int buttonStateDebounced = 0;

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
    if (i != KEY_ARRAY_SIZE - 1)
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

  // setup status pin
  pinMode(TX_DONE_PIN, OUTPUT);

  // setup joyStick Input pins
  pinMode(joyXaxis, INPUT);
  pinMode(joyYaxis, INPUT);
  pinMode(joyButton, INPUT_PULLUP);

  // setup NRF24L01 radio
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  Serial.println("TX SIDE: ");
  Serial.println("Instructions: ");
  Serial.println("click joystick button for total successful and failed transmission attempts");
}

void loop()
{
  // keep status pin high
  digitalWrite(TX_DONE_PIN, HIGH);

  // update buttonState
  currentButtonState = !digitalRead(joyButton);

  // debounce button
  if (currentButtonState)
  {
    delay(DEBOUNCE_BUFF);
    if (!digitalRead(joyButton))
    {
      buttonStateDebounced = 1;
    }
  }

  while (Serial.available())
  {
    delay(5);
    char c = Serial.read();
    CommandStr += c;
  }
  CommandStr.trim();

  // setup constant message
  String textStr1 = "X:" + String(1) +  " Y:" + String(0) + " B:" + String(currentButtonState);
  textStr1.toCharArray(text1, sizeof(text1));

  // encrypt char array using AES
  aes128_enc_single(key, text1);

  // send encrypted text through radio
  digitalWrite(TX_DONE_PIN, LOW);
  bool result = radio.write(&text1, sizeof(text1));
  digitalWrite(TX_DONE_PIN, HIGH);

  if(result)
  {
    totalPass++;
  }
  else
  {
    totalFailed++;
  }
  
  if (buttonStateDebounced)
  {
    Serial.print("Total Successful Tx: ");
    Serial.println(totalPass);
    Serial.print(" ");
    Serial.print("Total Failed Tx: ");
    Serial.println(totalFailed);
  }
  CommandStr = "";

  // update previous joystick state
  prevButtonStateDebounced = buttonStateDebounced;
  buttonStateDebounced = 0;
}
