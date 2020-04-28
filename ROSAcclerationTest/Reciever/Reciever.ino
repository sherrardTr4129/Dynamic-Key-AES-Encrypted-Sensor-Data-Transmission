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

// define and macros
#define BNO055_SAMPLERATE_DELAY_MS (30)
#define KEY_ARRAY_SIZE              16
#define RX_DONE_PIN                  3
#define MESSAGE_LEN                  3

// instantiate radio object
RF24 radio(9, 10); // CE, CSN

// setup radio address array
const byte address[6] = "00001";

// setup key for aes encryption
uint8_t key[] = {34, 45, 77, 20, 24, 48, 63, 46, 73, 99, 57, 81, 03, 47, 85, 11};
uint8_t keyModArr[] = {34, 45, 77, 20, 24, 48, 63, 46, 73, 99, 57, 81, 03, 47, 85, 11};

// control variables
bool seenK = false;
bool seenJ = false;

// create empty string objects for incomming data after it is parsed
String KeyRotateVal1 = "";
String KeyRotateVal2 = "";
String newKeyVal = "";

// create joystick state variables
int rxXaxis = 0;
int rxYaxis = 0;
int rxButtonState = 0;
float rxXaxisFloat = 0.0;
float rxYaxisFloat = 0.0;

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

void toUint8Arr(String keyString, uint8_t keyArr[])
{
  String AccumNums;
  int count = 0;
  for (int i = 0; i < keyString.length(); i++)
  {
    char currentChar = keyString.charAt(i);
    if (currentChar != ',' && currentChar != '{' && currentChar != '}')
    {
      AccumNums += currentChar;
    }
    else if (currentChar == ',' || count == KEY_ARRAY_SIZE - 1)
    {
      uint8_t currentNum = (AccumNums.toInt());
      AccumNums = "";
      keyArr[count] = currentNum;
      count++;
    }
  }
}

void setup() {

  Serial.begin(9600);
  
  // setup status pin
  pinMode(RX_DONE_PIN, OUTPUT);

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
    aes128_dec_single(key, text);
    String MessageString = String(text);

    if (MessageString.charAt(0) == 'X')
    {
      // toggle done pulse
      digitalWrite(RX_DONE_PIN, HIGH);
      delay(1);
      digitalWrite(RX_DONE_PIN, LOW);

      // extract data
      int Xindex = MessageString.indexOf('X');
      int Yindex = MessageString.indexOf('Y');
      int Bindex = MessageString.indexOf('B');
      String Xsub = MessageString.substring(Xindex + 2, Yindex);
      String Ysub = MessageString.substring(Yindex + 2, Bindex);
      String Bsub = MessageString.substring(Bindex + 2);
      Xsub.trim();
      Ysub.trim();
      Bsub.trim();

      rxXaxis = Xsub.toInt();
      rxYaxis = Ysub.toInt();
      rxButtonState = Bsub.toInt();

      // shift and scale joystickReadings
      rxXaxisFloat = rxXaxis - 512;
      rxYaxisFloat = rxYaxis - 512;
      if (rxXaxisFloat != 0 and rxYaxisFloat != 0)
      {
        rxXaxisFloat = rxXaxisFloat / 512;
        rxYaxisFloat = rxYaxisFloat / 512;
      }
      Serial.print("X: ");
      Serial.print(rxXaxisFloat);
      Serial.print(" ");
      Serial.print("Y: ");
      Serial.println(rxYaxisFloat);
    }
    
    else if (MessageString.charAt(0) == 'K')
    {
      KeyRotateVal1 = MessageString.substring(1);
      seenK = true;
    }

    else if (MessageString.charAt(0) == 'J')
    {
      KeyRotateVal2 = MessageString.substring(1);
      seenJ = true;
    }

    if (seenK && seenJ)
    {
      // toggle done pulse
      digitalWrite(RX_DONE_PIN, HIGH);
      delay(1);
      digitalWrite(RX_DONE_PIN, LOW);

      // concatinate key components together
      newKeyVal = KeyRotateVal1 + KeyRotateVal2;

      // convert string to uint8_t array
      toUint8Arr(newKeyVal, keyModArr);
      ArrToString(keyModArr);
      memcpy(key, keyModArr, sizeof(keyModArr));

      seenK = false;
      seenJ = false;
    }
  }

  delay(BNO055_SAMPLERATE_DELAY_MS);
}
