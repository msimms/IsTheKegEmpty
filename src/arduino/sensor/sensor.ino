#include <SPI.h>
#include <WiFiNINA.h>
#include "HX711.h"

#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// If you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "www.google.com";    // name address for Google (using DNS)

// Initialize the Ethernet client library with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

// HX711 circuit wiring
const int LOADCELL1_DOUT_PIN = 2;
const int LOADCELL1_SCK_PIN = 3;

// HX711 object.
HX711 scale;

void printWifiStatus() {
  // Print the SSID of the attached network.
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your board's IP address.
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength.
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setupWifi() {
  // Check for the WiFi module.
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // Attempt to connect to WiFi network.
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // Wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to WiFi");
  printWifiStatus();

  // Connect. If you get a connection, report back via serial.
  Serial.println("\nStarting connection to server...");
  if (client.connect(server, 80)) {
    Serial.println("connected to server");
  }
}

void setup() {
  // Initialize serial and wait for port to open.
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));

  Serial.println("\nEmpty the scale, press a key to continue");
  while(!Serial.available());
  while(Serial.available()) Serial.read();

  scale.tare();
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));


  Serial.println("\nPut 1000 gram in the scale, press a key to continue");
  while(!Serial.available());
  while(Serial.available()) Serial.read();

  scale.calibrate_scale(1000, 5);
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));

  Serial.println("\nScale is calibrated, press a key to continue");
  // Serial.println(scale.get_scale());
  // Serial.println(scale.get_offset());
  while(!Serial.available());
  while(Serial.available()) Serial.read();
}

void loop() {
  Serial.print("UNITS: ");
  Serial.println(scale.get_units(10));
  delay(250);
}
