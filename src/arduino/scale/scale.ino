// MIT License
//
// Copyright (c) 2025 Mike Simms
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <Wire.h>
#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include "Arduino_LED_Matrix.h" // Include the LED_Matrix library
#include "arduino_secrets.h" 

// Wifi
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)

// Initialize WiFi.
WiFiClient g_wifi_client;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 g_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ArduinoLEDMatrix g_matrix;  // Create an instance of the ArduinoLEDMatrix class

#define LOGO_HEIGHT 32
#define LOGO_WIDTH  32
#define LEFT_LOGO_MARGIN 4
#define LEFT_TEXT_MARGIN (LEFT_LOGO_MARGIN + LOGO_WIDTH + 4)
static const unsigned char PROGMEM g_logo_bmp[] =
{ 0x00, 0x00, 0x00, 0x00,
  0x00, 0x05, 0xA0, 0x00,
  0x00, 0x3F, 0xFC, 0x00,
  0x00, 0xF8, 0x1F, 0x00,
  0x01, 0xC0, 0x03, 0x80,
  0x03, 0x80, 0x00, 0xE0,
  0x06, 0x00, 0x00, 0x60,
  0x0E, 0x05, 0x00, 0x30,
  0x18, 0x81, 0x08, 0x18,
  0x19, 0x74, 0xD4, 0x18,
  0x32, 0x45, 0x94, 0x0C,
  0x31, 0x14, 0x90, 0x0C,
  0x30, 0x51, 0x48, 0x06,
  0x20, 0x14, 0x00, 0x06,
  0x60, 0x11, 0x00, 0x06,
  0x20, 0x05, 0x4D, 0x86,
  0x60, 0x14, 0x94, 0x06,
  0x20, 0x11, 0x90, 0x86,
  0x70, 0x15, 0x50, 0xC6,
  0x20, 0x10, 0x08, 0x0C,
  0x30, 0x10, 0x00, 0x04,
  0x30, 0x15, 0xD5, 0x8C,
  0x18, 0x15, 0x15, 0x18,
  0x1C, 0x11, 0x11, 0x18,
  0x0C, 0x14, 0x91, 0x30,
  0x06, 0x08, 0x44, 0x70,
  0x03, 0x80, 0x00, 0xE0,
  0x01, 0xC0, 0x03, 0x80,
  0x00, 0xF8, 0x1F, 0x00,
  0x00, 0x3F, 0xFC, 0x00,
  0x00, 0x05, 0xC0, 0x00,
  0x00, 0x00, 0x00, 0x00
};
  
// HX711 circuit wiring for three load cells.
const int LOADCELL1_DOUT_PIN = 2;
const int LOADCELL1_SCK_PIN = 3;
const int LOADCELL2_DOUT_PIN = 4;
const int LOADCELL2_SCK_PIN = 5;
const int LOADCELL3_DOUT_PIN = 6;
const int LOADCELL3_SCK_PIN = 7;
const int LOADCELL4_DOUT_PIN = 8;
const int LOADCELL4_SCK_PIN = 9;

// HX711 objects.
HX711 g_hx711_1;
HX711 g_hx711_2;
HX711 g_hx711_3;
HX711 g_hx711_4;

// Will return this as an error code when reading an HX711.
#define ERROR_NUM -1

// Configuration values.
float g_tare_value = 0.0;
float g_calibration_value = 0.0;
float g_calibration_weight = 0.0;

/// @function float_is_valid
bool float_is_valid(float num) {
  return num >= 0.001;
}

/// @function draw_logo
void draw_logo(void) {
  g_display.clearDisplay();
  g_display.drawBitmap(
    LEFT_LOGO_MARGIN,
    ((g_display.height() - LOGO_HEIGHT) / 2) + 2,
    g_logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  g_display.display();
}

/// @function update_builtin_display
void update_builtin_display(char* msg) {
}

/// @function update_display
void update_display(char* msg) {

  // Logo.
  draw_logo();

  // Message.
  g_display.setTextSize(1); // X pixel scale
  g_display.setTextColor(SSD1306_WHITE); // Draw white text
  g_display.setCursor(LEFT_TEXT_MARGIN, 14); // Start at top-left corner, after the logo
  g_display.println(msg);
  g_display.display();
}

/// @function update_display_with_weight
void update_display_with_weight(char* msg) {

  // Logo.
  draw_logo();

  // Message.
  g_display.setTextSize(2); // X pixel scale
  g_display.setTextColor(SSD1306_WHITE); // Draw white text
  g_display.setCursor(LEFT_TEXT_MARGIN, 14); // Start at top-left corner, after the logo
  g_display.println(msg);
  g_display.display();
}

/// @function setup_display
void setup_display(void) {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (g_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    draw_logo();
  }
  else {
    Serial.println("Error: SSD1306 allocation failed!");
  }
}

/// @function setup_wifi
void setup_wifi() {
  Serial.println("Setting up Wifi...");

  // Attempt to connect to Wi-Fi network:
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  // You're connected now, so print out the data.
  Serial.println("Wifi connected!");
}

/// @function post_status
void post_status(String post_data) {
  if (WiFi.status() == WL_CONNECTED) {
    HttpClient client = HttpClient(g_wifi_client, STATUS_URL, STATUS_PORT);

    Serial.println("Sending status...");

    // Set headers
    client.beginRequest();
    client.post(STATUS_ENDPOINT);
    client.sendHeader("Content-Type", "application/json");
    client.sendHeader("Content-Length", post_data.length());
    client.beginBody();
    client.print(post_data);
    client.endRequest();

    // Get response
    int status_code = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Status Code: ");
    Serial.println(status_code);
    Serial.print("Response: ");
    Serial.println(response);
  }
  else {
    Serial.println("Not connected to Wifi!");
  }
}

/// @function setup_scale
/// Called once to initializae all of the HX711s.
void setup_scale(void) {
  g_hx711_1.begin(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN);
  delay(10);
  g_hx711_2.begin(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN);
  delay(10);
  g_hx711_3.begin(LOADCELL3_DOUT_PIN, LOADCELL3_SCK_PIN);
  delay(10);
  g_hx711_4.begin(LOADCELL4_DOUT_PIN, LOADCELL4_SCK_PIN);
  delay(10);

  g_hx711_1.power_up();
  delay(10);
  g_hx711_2.power_up();
  delay(10);
  g_hx711_3.power_up();
  delay(10);
  g_hx711_4.power_up();
  delay(10);
}

/// @function readHx711
/// Called to read a value from a single HX711.
long readHx711(HX711 hx) {
  uint8_t loop_count = 0;
  while (!hx.is_ready() && loop_count < 100) {
    delay(10);
    ++loop_count;
  }
  if (hx.is_ready()) {
    return hx.read();
  }
  return ERROR_NUM;
}

/// @function read_scale_value
// Called to read a value from the scale.
long read_scale_value(void) {
  long sum = 0;

  // Load Cell 1
  long value1 = readHx711(g_hx711_1);
  if (value1 == ERROR_NUM) {
    Serial.println("Error reading HX711 #1");
  }
  else {
    Serial.println(value1);
    sum = sum + value1;
  }

  // Load Cell 2
  long value2 = readHx711(g_hx711_2);
  if (value2 == ERROR_NUM) {
    Serial.println("Error reading HX711 #2");
  }
  else {
    Serial.println(value2);
    sum = sum + value2;
  }

  // Load Cell 3
  long value3 = readHx711(g_hx711_3);
  if (value3 == ERROR_NUM) {
    Serial.println("Error reading HX711 #3");
  }
  else {
    Serial.println(value3);
    sum = sum + value3;
  }

  // Load Cell 4
  long value4 = readHx711(g_hx711_4);
  if (value4 == ERROR_NUM) {
    Serial.println("Error reading HX711 #4");
  }
  else {
    Serial.println(value4);
    sum = sum + value4;
  }
  return sum;
}

/// @function read_avg_scale_value
float read_avg_scale_value(void) {

  // Average three values.
  float value = 0.0;
  for (uint8_t t = 0; t < 3; t++) {
    long raw_value = read_scale_value();
    if (raw_value != ERROR_NUM) {
      value = value + (float)raw_value;
    }
  }
  value = value / 3.0;
  return value;
}

/// @function compute_weight
// Returns the weight, in grams, or -1 upon error.
float compute_weight(float measured_value) {

  // Make sure we have tare and calibration values and a calibration weight.
  if (!(float_is_valid(g_tare_value) && float_is_valid(g_calibration_value))) {
    Serial.println("Error: No tare or calibration value!");
    return (float)ERROR_NUM;
  }
  if (!float_is_valid(g_tare_value)) {
    Serial.println("Error: No tare value!");
    return (float)ERROR_NUM;
  }
  if (!float_is_valid(g_calibration_value)) {
    Serial.println("Error: No calibration value!");
    return (float)ERROR_NUM;
  }
  if (!float_is_valid(g_calibration_weight)) {
    Serial.println("Error: No calibration weight!");
    return (float)ERROR_NUM;
  }

  float m = g_calibration_weight / (g_calibration_value - g_tare_value);
  float weight = (m * measured_value);
  return weight;
}

/// @function read_config_values
void read_config_values() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    g_tare_value = Serial.parseFloat();
    c = Serial.read();
    g_calibration_value = Serial.parseFloat();
    c = Serial.read();
    g_calibration_weight = Serial.parseFloat();
  }
}

/// @function read_given_weight_value
void read_given_weight_value(void) {
  if (Serial.available() > 0) {
    char c = Serial.read();
    g_calibration_weight = Serial.parseFloat();
  }
}

/// @function setup
/// Called once, at program start
void setup() {
  Serial.begin(9600);
  Wire.begin();
  setup_wifi();
  setup_display();
  setup_scale();
}

/// @function loop
/// Called repeatedly
void loop() {

  // Raw value from the scale.
  float raw_value = read_avg_scale_value();

  // Convert to weight.
  float weight = compute_weight(raw_value);

  // Update the display.
  if (float_is_valid(weight)) {
    Serial.print("Weight:");
    Serial.println(weight, 1);

    char buff[64];
    snprintf(buff, sizeof(buff) - 1, "%0.1f g", weight);
    update_display_with_weight(buff);
  }
  else if (!float_is_valid(g_tare_value)) {
    update_display("Tare needed!");
  }
  else if (!float_is_valid(g_calibration_value)) {
    update_display("Cal. needed!");
  }

  // Print the raw value.
  Serial.print("Raw Value:");
  Serial.println(raw_value, 1);

  // Are we being sent a command?
  if (Serial.available() > 0) {

    // Read the command from the serial port.
    char received_char = Serial.read();

    // Read from the scale.
    if (received_char == 'R') {
      Serial.print("Weight:");
      Serial.println(weight, 1);
    }

    // Compute a new tare value.
    else if (received_char == 'T') {
      update_display("Tareing...");
      Serial.println("Tareing...");

      g_tare_value = raw_value;
      Serial.print("Tare Value:");
      Serial.println(g_tare_value, 1);
    }

    // Compute a new weight value.
    else if (received_char == 'W') {
      update_display("Calibrating...");
      Serial.println("Calibrating...");

      read_given_weight_value();
      g_calibration_value = raw_value;

      Serial.print("Given Value:");
      Serial.println(g_calibration_value, 1);
      Serial.print("Given Weight (g):");
      Serial.println(g_calibration_weight, 1);
    }

    // Receive configuration values from the command and control computer.
    else if (received_char == 'C') {
      read_config_values();
      Serial.print("Tare Value:");
      Serial.println(g_tare_value, 1);
      Serial.print("Config Value:");
      Serial.println(g_calibration_value, 1);
      Serial.print("Config Weight (g):");
      Serial.println(g_calibration_weight, 1);
    }
  }

  // Rate limit.
  delay(1000);
}
