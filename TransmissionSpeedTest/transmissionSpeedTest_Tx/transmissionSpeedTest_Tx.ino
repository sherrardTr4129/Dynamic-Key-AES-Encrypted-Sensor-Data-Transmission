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

#define BNO055_SAMPLERATE_DELAY_MS (50)

Adafruit_BNO055 bno = Adafruit_BNO055(55);

RF24 radio(9, 10); // CE, CSN

char cmdChar;
char ackChar[32] = "";

// setup key for AES encryption
const uint8_t key[] = {34, 45, 77, 20, 24, 48, 63, 46, 73, 99, 57, 81, 03, 47, 85, 11};

void printBinChars(String x, String y, String z)
{
  for (int i = 0; i < x.length(); i++)
  {
    Serial.print((int)x.charAt(i), BIN);
  }
  Serial.print("!");
  for (int i = 0; i < y.length(); i++)
  {
    Serial.print((int)y.charAt(i), BIN);
  }
  Serial.print("!");
  for (int i = 0; i < z.length(); i++)
  {
    Serial.print((int)z.charAt(i), BIN);
  }
}

void setup()
{
  //setup serial
  Serial.begin(115200);

  // initialize BNO055 Sensor
  if (!bno.begin())
  {
    // sensor could not be found, loop forever
    Serial.print("No BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  delay(1000);
  bno.setExtCrystalUse(true);

  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.setRetries(3, 8);
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength(RF24_CRC_16);

  byte addresses[][6] = {"1Node", "2Node"};
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
  radio.startListening();

  radio.writeAckPayload(1, &ackChar, 32);
  radio.stopListening();
}

void loop(void)
{
  // init long values to hold millisecond timer data
  unsigned long xTxStart, xTxEnd, xTxTotal = 0;
  unsigned long yTxStart, yTxEnd, yTxTotal = 0;
  unsigned long zTxStart, zTxEnd, zTxTotal = 0;
  unsigned long encyptStart, encyptEnd, encryptTotal = 0;
  unsigned long dencyptStart, dencyptEnd, dencryptTotal = 0;
  unsigned long pollStart, pollEnd, pollTotal = 0;
  unsigned long roundTripX, roundTripY, roundTripZ = 0;
  bool fail = false;

  // setup buffer for transmitted text
  char textX_tx[32] = "";
  char textY_tx[32] = "";
  char textZ_tx[32] = "";

  // setup buffer for recived text
  char textX_rx[32] = "";
  char textY_rx[32] = "";
  char textZ_rx[32] = "";

  cmdChar = Serial.read();
  if (cmdChar == 's')
  {
    // get new sensor event instance
    sensors_event_t event;
    bno.getEvent(&event);

    // get calibration information
    uint8_t sys, gyro, accel, mag = 0;
    bno.getCalibration(&sys, &gyro, &accel, &mag);

    // grab sensor orientations
    pollStart = micros();
    float Xorientation = (float)event.orientation.x;
    float Yorientation = (float)event.orientation.y;
    float Zorientation = (float)event.orientation.z;
    pollEnd = micros();
    pollTotal = pollEnd - pollStart;

    // construct message strings and convert them to a character arrays
    String textStrX = "X:" + String(Xorientation);
    String textStrY = "Y:" + String(Yorientation);
    String textStrZ = "Z:" + String(Zorientation);
    textStrX.toCharArray(textX_tx, sizeof(textX_tx));
    textStrY.toCharArray(textY_tx, sizeof(textY_tx));
    textStrZ.toCharArray(textZ_tx, sizeof(textZ_tx));

    // encrypt character arrays using AES
    encyptStart = micros();
    aes128_enc_single(key, textX_tx);
    aes128_enc_single(key, textY_tx);
    aes128_enc_single(key, textZ_tx);
    encyptEnd = micros();
    encryptTotal = encyptEnd - encyptStart;

    // send encrypted text through radio
    xTxStart = micros();
    if (radio.write(&textX_tx, sizeof(textX_tx)))
    {
      xTxEnd = micros();
      xTxTotal = xTxEnd - xTxStart;
      if (!radio.available())
      {
        fail = true;
        radio.flush_rx();
        radio.flush_tx();
      }
      else
      {
        while (radio.available())
        {
          radio.read(&textX_rx, sizeof(textX_rx));
          roundTripX = micros() - xTxStart;
          radio.flush_rx();
          radio.flush_tx();
        }
      }
    }
    else
    {
      fail = true;
      radio.flush_rx();
      radio.flush_tx();
    }

    delay(BNO055_SAMPLERATE_DELAY_MS);

    // send encrypted text through radio
    yTxStart = micros();
    if (radio.write(&textY_tx, sizeof(textY_tx)))
    {
      yTxEnd = micros();
      yTxTotal = yTxEnd - yTxStart;
      if (!radio.available())
      {
        fail = true;
        radio.flush_rx();
        radio.flush_tx();
      }
      else
      {
        while (radio.available())
        {
          radio.read(&textY_rx, sizeof(textY_rx));
          roundTripY = micros() - yTxStart;
          radio.flush_rx();
          radio.flush_tx();
        }
      }
    }
    else
    {
      fail = true;
      radio.flush_rx();
      radio.flush_tx();
    }

    delay(BNO055_SAMPLERATE_DELAY_MS);

    // send encrypted text through radio
    zTxStart = micros();
    if (radio.write(&textZ_tx, sizeof(textZ_tx)))
    {
      zTxEnd = micros();
      zTxTotal = zTxEnd - zTxStart;
      if (!radio.available())
      {
        fail = true;
        radio.flush_rx();
        radio.flush_tx();
      }
      else
      {
        while (radio.available())
        {
          radio.read(&textZ_rx, sizeof(textZ_rx));
          roundTripZ = micros() - zTxStart;
          radio.flush_rx();
          radio.flush_tx();
        }
      }
    }
    else
    {
      fail = true;
      radio.flush_rx();
      radio.flush_tx();
    }

    if (!fail)
    {
      String x_tx = String(textX_tx);
      String y_tx = String(textY_tx);
      String z_tx = String(textZ_tx);
      String x_rx = String(textX_rx);
      String y_rx = String(textY_rx);
      String z_rx = String(textZ_rx);

      Serial.print(textStrX);
      Serial.print(",");
      Serial.print(textStrY);
      Serial.print(",");
      Serial.print(textStrZ);
      Serial.print("!");
      printBinChars(x_tx, y_tx, z_tx);
      Serial.print("!");
      printBinChars(x_rx, y_rx, z_rx);
      Serial.print("!");

      // de-encrypt character arrays using AES
      dencyptStart = micros();
      aes128_dec_single(key, textX_rx);
      aes128_dec_single(key, textY_rx);
      aes128_dec_single(key, textZ_rx);
      dencyptEnd = micros();
      dencryptTotal = dencyptEnd - dencyptStart;

      Serial.print(textX_rx);
      Serial.print(",");
      Serial.print(textY_rx);
      Serial.print(",");
      Serial.print(textZ_rx);
      Serial.print("!");
      Serial.print(xTxTotal);
      Serial.print(",");
      Serial.print(yTxTotal);
      Serial.print(",");
      Serial.print(zTxTotal);
      Serial.print("!");
      Serial.print(roundTripX);
      Serial.print(",");
      Serial.print(roundTripY);
      Serial.print(",");
      Serial.print(roundTripZ);
      Serial.print("!");
      Serial.print(encryptTotal);
      Serial.print("!");
      Serial.print(pollTotal);
      Serial.print("!");
      Serial.println(dencryptTotal);
    }
  }
}
