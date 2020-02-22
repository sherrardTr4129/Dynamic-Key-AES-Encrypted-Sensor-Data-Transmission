#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "StringSplitter.h"

// define and macros
#define BNO055_SAMPLERATE_DELAY_MS (100)

// instantiate radio object
RF24 radio(9, 10); // CE, CSN

// setup radio address array
const byte address[6] = "00001";

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
    // create empty string objects for incomming data after it is parsed
    String X_ValString = "";
    String Y_ValString = "";
    String Z_ValString = "";
    String ZeroChar = "0";
    String AccelCalib = "3";

    // create buffers for text data
    char text[255] = "";

    //create buffer for our first pass parsing
    char message[6][128];

    // define size of items to be parsed from recived string
    int messageSize = 3;
    // grab data from radiobuffer
    radio.read(&text, sizeof(text));

    // create delimeter char
    char delimiterComma[] = ",";

    // split message into the three different measurements
    char* buf = strtok(text, delimiterComma);
    int count = 0;
    while (buf != NULL)
    {
      strcpy(message[count], buf);
      buf = strtok(NULL, delimiterComma);
      count++;
    }

    // split the individual substrings into their respective
    // directional values
    for (int i = 0; i < messageSize; i++)
    {
      String subString = String(message[i]);
      int indexOfColon = subString.indexOf(":");
      String currentNumericalValue = subString.substring(indexOfColon + 1);

      // set current value to appropriate message string
      if (i == 0)
      {
        X_ValString = currentNumericalValue;
      }
      else if (i == 1)
      {
        Y_ValString = currentNumericalValue;
      }
      else if (i == 2)
      {
        Z_ValString = currentNumericalValue;
      }
    }
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
  }
  delay(BNO055_SAMPLERATE_DELAY_MS);
}
