#include "globals.h"
#include "BluetoothSerial.h"

#include <SPI.h>
#include <FastLED.h>
// How many leds in your strip?
#define NUM_LEDS 1
#define DATA_PIN 27
// Define the array of leds
CRGB leds[NUM_LEDS];

BluetoothSerial SerialBT;

#ifdef ARDUINO_TEENSY41
  #include <SdFat-beta.h>
#else
  #include <SdFat.h>  // One SD library to rule them all - Greinman SdFat from Library Manager
#endif

// Board definitions go into the "hardware" folder
// Choose/change a file from there
#include "hardware/esp32.h"

// Delays for LED blinking
#define sDELAY 250
#define DELAY 100

#include "abstraction_arduino.h"

// Serial port speed
#define SERIALSPD 115200

// PUN: device configuration
#ifdef USE_PUN
File pun_dev;
int pun_open = FALSE;
#endif

// LST: device configuration
#ifdef USE_LST
File lst_dev;
int lst_open = FALSE;
#endif

#include "ram.h"
#include "console.h"
#include "cpu.h"
#include "disk.h"
#include "host.h"
#include "cpm.h"
#ifdef CCP_INTERNAL
#include "ccp.h"
#endif

void led_show(CRGB color) {
  leds[0] = color;
  FastLED.show();
}

bool serialBT_opened = false;
void callbackBT(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
     serialBT_opened = true;
    led_show(CRGB::Blue);
  }else if (event == ESP_SPP_CLOSE_EVT) {
    led_show(CRGB::Red);
  }
}
void setup(void) {
  // ## Clockless types ##
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  pinMode(39, INPUT);
  //Bluetoothのcallback関数設定
  SerialBT.register_callback(callbackBT);
  SerialBT.begin("Atomic CP/M"); //Bluetooth device name

#ifdef DEBUGLOG
  _sys_deletefile((uint8 *)LogName);
#endif

  while(!serialBT_opened) {
    // Turn the LED on, then pause
    led_show(CRGB::Black);
    delay(sDELAY);
    // Now turn the LED off, then pause
    led_show(CRGB::Blue);
    delay(sDELAY);   
  }
  led_show(CRGB::Blue);
  
  _clrscr();
  _puts("CP/M 2.2 Emulator v" VERSION " by Marcelo Dantas\r\n");
  _puts("Arduino read/write support by Krzysztof Klis\r\n");
  _puts("      Build " __DATE__ " - " __TIME__ "\r\n");
  _puts("--------------------------------------------\r\n");
  _puts("CCP: " CCPname "    CCP Address: 0x");
  _puthex16(CCPaddr);
  _puts("\r\nBOARD: ");
  _puts(BOARD);
  _puts("\r\n");
  
#if defined board_agcm4
  _puts("Initializing Grand Central SD card.\r\n");
  if (SD.cardBegin(SDINIT, SD_SCK_MHZ(50))) {

    if (!SD.fsBegin()) {
      _puts("\nFile System initialization failed.\n");
      return;
    }
#elif defined board_teensy40 
  _puts("Initializing Teensy 4.0 SD card.\r\n");
  if (SD.begin(SDINIT, SD_SCK_MHZ(25))) {
#elif defined board_esp32
  _puts("Initializing ESP32 SD card.\r\n");
  SPI.begin(SDINIT);
//  if (SD.begin(SS, SD_SCK_MHZ(SDMHZ))) {
  if (SD.begin(-1, SD_SCK_MHZ(SDMHZ))) {
#else
  _puts("Initializing SD card.\r\n");
  if (SD.begin(SDINIT)) {
#endif
    if (VersionCCP >= 0x10 || SD.exists(CCPname)) {
      while (true) {
        _puts(CCPHEAD);
        _PatchCPM();
	Status = 0;
#ifndef CCP_INTERNAL
        if (!_RamLoad((char *)CCPname, CCPaddr)) {
          _puts("Unable to load the CCP.\r\nCPU halted.\r\n");
          break;
        }
        Z80reset();
        SET_LOW_REGISTER(BC, _RamRead(0x0004));
        PC = CCPaddr;
        Z80run();
#else
        _ccp();
#endif
        if (Status == 1)
          break;
#ifdef USE_PUN
        if (pun_dev)
          _sys_fflush(pun_dev);
#endif
#ifdef USE_LST
        if (lst_dev)
          _sys_fflush(lst_dev);
#endif
      }
    } else {
      _puts("Unable to load CP/M CCP.\r\nCPU halted.\r\n");
    }
  } else {
    _puts("Unable to initialize SD card.\r\nCPU halted.\r\n");
  }
}

void loop(void) {
//  digitalWrite(LED, HIGH^LEDinv);
//  delay(DELAY);
//  digitalWrite(LED, LOW^LEDinv);
//  delay(DELAY);
//  digitalWrite(LED, HIGH^LEDinv);
//  delay(DELAY);
//  digitalWrite(LED, LOW^LEDinv);
  delay(DELAY * 4);
}
