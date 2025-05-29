// Created by Michael Simms

#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h" 

// Wifi
char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 g_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
const int LOADCELL1_DOUT_PIN = 1;
const int LOADCELL1_SCK_PIN = 2;
const int LOADCELL2_DOUT_PIN = 3;
const int LOADCELL2_SCK_PIN = 4;
const int LOADCELL3_DOUT_PIN = 5;
const int LOADCELL3_SCK_PIN = 6;
const int LOADCELL4_DOUT_PIN = 7;
const int LOADCELL4_SCK_PIN = 8;

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

/// @function updateDisplay
void updateDisplay(char* msg) {
  draw_logo();
  g_display.setTextSize(1); // X pixel scale
  g_display.setTextColor(SSD1306_WHITE); // Draw white text
  g_display.setCursor(LEFT_TEXT_MARGIN, 14); // Start at top-left corner, after the logo
  g_display.println(msg);
  g_display.display();
}

/// @function updateDisplayWithWeight
void updateDisplayWithWeight(char* msg) {
  draw_logo();
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
  int wifi_status = WL_IDLE_STATUS;
  while (wifi_status != WL_CONNECTED) {
    Serial.print("Attempting to connect to the network: ");
    Serial.println(ssid);
    wifi_status = WiFi.begin(ssid, pass);

    // Wait a few seconds for connection.
    delay(5000);
  }

  // You're connected now, so print out the data.
  Serial.println("Wifi connected!");
}

/// @function print_wifi_status
void print_wifi_status() {
  // Print the SSID of the attached network.
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print the board's IP address.
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength.
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/// @function post_status
void post_status(String str) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;

    Serial.println("Sending status...");
    if (client.connect(STATUS_URL, STATUS_PORT)) {
      client.println(str);
      Serial.println("Status sent!");
    } else {
      Serial.println("Error connecting to the server!");
      print_wifi_status();
    }
    client.stop();
  }
  else {
    Serial.println("Not connected to Wifi!");
    print_wifi_status();
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
    updateDisplayWithWeight(buff);
  }
  else if (!float_is_valid(g_tare_value)) {
    updateDisplay("Tare needed!");
  }
  else if (!float_is_valid(g_calibration_value)) {
    updateDisplay("Cal. needed!");
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
      updateDisplay("Tareing...");
      Serial.println("Tareing...");

      g_tare_value = raw_value;
      Serial.print("Tare Value:");
      Serial.println(g_tare_value, 1);
    }

    // Compute a new weight value.
    else if (received_char == 'W') {
      updateDisplay("Calibrating...");
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
