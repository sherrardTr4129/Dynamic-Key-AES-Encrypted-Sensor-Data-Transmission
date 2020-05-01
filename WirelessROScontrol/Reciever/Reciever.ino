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

// setup key for AES encryption
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

/*
 * This function generates a String representation of a given AES
 * key array.
 * 
 * Params: 
 *    keyArr: The pointer to a given key array
 * 
 * Returns:
 *    toReturnStr: The string representation of the AES key
 */
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

/*
 * This function takes a String representation of a given key 
 * and modifies an existing key array to contain the same data within the
 * String representation. Note that this function assumes the key's String representation
 * is in the format {XX, YY, ... ,ZZ}.
 * 
 * params:
 *    keyString: The String representation of a given key array
 *    keyArr: The pointer to an existing key array to be modified.
 */
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

  // setup serial communication
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

    // attempt to decrypt the message using the current AES key
    aes128_dec_single(key, text);
    String MessageString = String(text);

    // if the message starts with the character 'X', it is a user input
    // data packet
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

      // convert extracted data from String representation to integer representation.
      rxXaxis = Xsub.toInt();
      rxYaxis = Ysub.toInt();
      rxButtonState = Bsub.toInt();

      // shift joystickReadings down by 512
      rxXaxisFloat = rxXaxis - 512;
      rxYaxisFloat = rxYaxis - 512;

      // if the shifted values are not already zero, scale them to be within -1 and 1
      if (rxXaxisFloat != 0 and rxYaxisFloat != 0)
      {
        rxXaxisFloat = rxXaxisFloat / 512;
        rxYaxisFloat = rxYaxisFloat / 512;
      }

      // print the extracted data over the serial interface
      Serial.print("X: ");
      Serial.print(rxXaxisFloat);
      Serial.print(" ");
      Serial.print("Y: ");
      Serial.println(rxYaxisFloat);
    }

    // if the first element of the retrieved message is K, it is 
    // the first portion of a newly generated key.
    else if (MessageString.charAt(0) == 'K')
    {
      // get the first portion of the new key, starting
      // after the control character.
      KeyRotateVal1 = MessageString.substring(1);
      seenK = true;
    }

    // if the first element of the retrieved message is J, it is 
    // the first portion of a newly generated key.
    else if (MessageString.charAt(0) == 'J')
    {
      // get the seoond portion of the new key, starting
      // after the control character.
      KeyRotateVal2 = MessageString.substring(1);
      seenJ = true;
    }

    // once both key portions have been recieved...
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
  // loop refresh delay
  delay(BNO055_SAMPLERATE_DELAY_MS);
}
