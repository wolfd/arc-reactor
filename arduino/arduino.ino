#include <Adafruit_BLE_UART.h>
#include <Adafruit_NeoPixel.h>

#include <SPI.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define DEBUG 0

// Set up Bluetooth unit
#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2
#define ADAFRUITBLE_RST 9

Adafruit_BLE_UART uart = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

// Set up NeoPixel ring
#define RING_PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, RING_PIN, NEO_GRB + NEO_KHZ800);

uint8_t mode = 0; // 0 is solid color

/**************************************************************************/
/*!
    This function is called whenever select ACI events happen
*/
/**************************************************************************/
void aciCallback(aci_evt_opcode_t event)
{
  switch(event)
  {
    case ACI_EVT_DEVICE_STARTED:
      #if DEBUG
      Serial.println(F("Advertising started"));
      #endif
      break;
    case ACI_EVT_CONNECTED:
      #if DEBUG
      Serial.println(F("Connected!"));
      #endif
      break;
    case ACI_EVT_DISCONNECTED:
      #if DEBUG
      Serial.println(F("Disconnected or advertising timed out"));
      #endif
      break;
    default:
      break;
  }
}

/**************************************************************************/
/*!
    This function is called whenever data arrives on the RX channel
*/
/**************************************************************************/
void rxCallback(uint8_t *buffer, uint8_t len)
{
//  Serial.print(F("Received "));
//  Serial.print(len);
//  Serial.print(F(" bytes: "));
//  
//  for(int i=0; i<len; i++)
//   Serial.print((char)buffer[i]);
//
//  Serial.print(F(" ["));
//
//  for(int i=0; i<len; i++)
//  {
//    Serial.print(" 0x"); Serial.print((char)buffer[i], HEX); 
//  }
//  Serial.println(F(" ]"));

  /*
   * Data structure:
   * mode [data]
   * 
   * if continuation
   * mode = 255
   */
  #if DEBUG
  Serial.print(F("*"));
  #endif
  if (len == 0) return;
  mode = buffer[0];

  #if DEBUG
  Serial.print(F("m"));
  #endif

  uint8_t r, g, b;
  
  switch (mode) {
    case 0:
      #if DEBUG
      Serial.print(F("c"));
      #endif
      // Solid color mode
      r = buffer[1];
      g = buffer[2];
      b = buffer[3];
      
      for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, r, g, b);
        #if DEBUG
        Serial.print(F("*"));
        #endif
      }
      strip.show();
      #if DEBUG
      Serial.print(F("!"));
      #endif
      break;
    case 1:
      // set single LED mode
      // Solid color mode
      n = buffer[1];
      r = buffer[2];
      g = buffer[3];
      b = buffer[4];
      
      if(n < strip.numPixels()) {
        strip.setPixelColor(n, r, g, b);
      }
      strip.show();
    case 255:
      // Continuation mode
      break;
    default:
    break;
  }

  /* Echo the same data back! */
  #if DEBUG
  uart.write(buffer, len);
  Serial.print(F("w"));
  #endif
}

/**************************************************************************/
/*!
    Configure the Arduino and start advertising with the radio
*/
/**************************************************************************/
void setup(void)
{ 
  Serial.begin(9600);
//  while(!Serial); // Leonardo/Micro should wait for serial init
  Serial.println(F("I'mma Arc Reactor"));

  // Setup bluetooth
  
  uart.setRXcallback(rxCallback);
  uart.setACIcallback(aciCallback);
  uart.setDeviceName("ArcRctr"); /* 7 characters max! */
  uart.begin();

  // Setup ring
  strip.begin();
  strip.show();
}

/**************************************************************************/
/*!
    Constantly checks for new events on the nRF8001
*/
/**************************************************************************/
void loop()
{
  uart.pollACI();
}
