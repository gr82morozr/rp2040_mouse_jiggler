#include <Adafruit_NeoPixel.h>
#include <Adafruit_TinyUSB.h>
#include <FreeRTOS.h>  // Included automatically with Arduino-Pico core
#include <task.h>

#define PIN 16
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Variables to store the RGB values
int redValue = 255;
int greenValue = 0;
int blueValue = 0;

// Variable to track the direction of color changes
int redStep = -1;
int greenStep = 1;
int blueStep = 0;

// Breathing brightness envelope (fully off to full brightness)
int brightnessValue = 0;
int brightnessStep = 1;

// Mouse movement control
const int deltaMove = 1;                  // Move by 1 pixels each time
const unsigned long moveInterval = 60000; // move every 1 min

// HID report descriptor using TinyUSB's template
enum {
  RID_MOUSE = 1
};

// USB HID object
Adafruit_USBD_HID usb_hid;

// Standard HID Mouse Report Descriptor (Driverless)
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(RID_MOUSE))
};

void ledTask() {
  // Scale the current colour directly to avoid cumulative rounding in
  // Adafruit_NeoPixel::setBrightness().
  const uint8_t scaledRed = (redValue * brightnessValue) / 255;
  const uint8_t scaledGreen = (greenValue * brightnessValue) / 255;
  const uint8_t scaledBlue = (blueValue * brightnessValue) / 255;

  pixels.setPixelColor(0, pixels.Color(scaledRed, scaledGreen, scaledBlue));
  pixels.show();

  brightnessValue += brightnessStep;
  if (brightnessValue >= 255) {
    brightnessValue = 255;
    brightnessStep = -1;
  } else if (brightnessValue <= 0) {
    brightnessValue = 0;
    brightnessStep = 1;
  }

  redValue += redStep;
  greenValue += greenStep;
  blueValue += blueStep;

  redValue = constrain(redValue, 0, 255);
  greenValue = constrain(greenValue, 0, 255);
  blueValue = constrain(blueValue, 0, 255);

  if (redValue <= 0 && greenValue >= 255) {
    redStep = 0;
    greenStep = -1;
    blueStep = 1;
  } else if (greenValue <= 0 && blueValue >= 255) {
    redStep = 1;
    greenStep = 0;
    blueStep = -1;
  } else if (blueValue <= 0 && redValue >= 255) {
    redStep = -1;
    greenStep = 1;
    blueStep = 0;
  }
}

void process_hid() {
  if (usb_hid.ready()) {
    int8_t x = 0, y = 0;
    switch (rp2040.hwrand32() & 0x03) {
      case 0: y = -deltaMove; break;  // Move up
      case 1: x = deltaMove; break;   // Move right
      case 2: y = deltaMove; break;   // Move down
      case 3: x = -deltaMove; break;  // Move left
    }

    usb_hid.mouseMove(RID_MOUSE, x, y);
    
    // Wait briefly for the first report to complete before moving back.
    const TickType_t waitStarted = xTaskGetTickCount();
    while (!usb_hid.ready() &&
           (xTaskGetTickCount() - waitStarted) < pdMS_TO_TICKS(100)) {
      vTaskDelay(pdMS_TO_TICKS(2));
    }

    if (usb_hid.ready()) {
      usb_hid.mouseMove(RID_MOUSE, -x, -y);
    }
  }
}

void setup() {
  pixels.begin();

  // Set up HID
  usb_hid.setBootProtocol(HID_ITF_PROTOCOL_MOUSE);
  usb_hid.setPollInterval(1);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("Logitech USB Optical Mouse");
  usb_hid.begin();

  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }

  // Re-enumerate if USB was initialized before the HID interface was added.
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }
}

void loop() {
  ledTask();
  vTaskDelay(pdMS_TO_TICKS(5)); // LED update every 5ms
}

void loop1() {
  process_hid();
  vTaskDelay(pdMS_TO_TICKS(moveInterval));  // Mouse movement update
}
