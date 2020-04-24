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

// setup variables for joystick state (past and present)
int currentXstate = 0;
int currentYstate = 0;
int currentButtonState = 0;
int prevXstate = 0;
int prevYstate = 0;
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
  Serial.println("move joystick to control robot in Gazebo enviornment. Click joystick down to rotate AES keys");
}

void loop()
{
  // keep status pin high
  digitalWrite(TX_DONE_PIN, HIGH);

  // update joystick states
  currentXstate = analogRead(joyXaxis);
  currentYstate = analogRead(joyYaxis);
  currentButtonState = !digitalRead(joyButton);

  // debounce button
  if(currentButtonState)
  {
    delay(DEBOUNCE_BUFF);
    if(!digitalRead(joyButton))
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

  if(currentXstate != prevXstate or currentYstate != prevYstate or buttonStateDebounced != prevButtonState)
  {
    String textStr1 = "X:" + String(currentXstate) +  " Y:" + String(currentYstate) + " B:" + String(currentButtonState);
    textStr1.toCharArray(text1, sizeof(text1));

    // encrypt char array using AES
    aes128_enc_single(key, text1);

    // send encrypted text through radio
    digitalWrite(TX_DONE_PIN, LOW);
    radio.write(&text1, sizeof(text1));
    digitalWrite(TX_DONE_PIN, HIGH);
  }
  
  if (buttonStateDebounced)
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

  // update previous joystick states
  prevXstate = currentXstate;
  prevYstate = currentYstate;
  prevButtonStateDebounced = buttonStateDebounced;
  buttonStateDebounced = 0;
}
