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

char text[255] = "";

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

  String textStr = "X:" + String(Xorientation) + "," + "Y:" + String(Yorientation)+ "," + "Z:" + String(Zorientation);
  textStr.toCharArray(text, sizeof(text));
  radio.write(&text, sizeof(text));
  delay(100);
}
