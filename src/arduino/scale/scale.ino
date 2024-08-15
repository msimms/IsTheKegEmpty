#include <SPI.h>
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include "Adafruit_HX711.h"
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key index number (needed only for WEP)

// If you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
char server[] = "status.mikesimms.net"; // name address for Google (using DNS)
const int port = 80;

// Initialize the Ethernet client library with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient wifi;
HttpClient client = HttpClient(wifi, server, port);

// HX711 circuit wiring for three load cells
const int LOADCELL1_DOUT_PIN = 2;
const int LOADCELL1_SCK_PIN = 3;
const int LOADCELL2_DOUT_PIN = 4;
const int LOADCELL2_SCK_PIN = 5;
const int LOADCELL3_DOUT_PIN = 6;
const int LOADCELL3_SCK_PIN = 7;

// HX711 objects.
Adafruit_HX711 hx711_1(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN);
Adafruit_HX711 hx711_2(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN);
Adafruit_HX711 hx711_3(LOADCELL3_DOUT_PIN, LOADCELL3_SCK_PIN);

// Called once to do WIFI initialization.
void setupWifi() {
  int status = WL_IDLE_STATUS;

  // Wait until we're connected.
  Serial.begin(9600);
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
  }

  // Print the SSID of the network you're attached to.
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your WiFi shield's IP address.
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

// Called once to initializae a single HX711.
void setupHx711(Adafruit_HX711 hx) {
  // Initialize the HX711.
  hx.begin();

  // Read and toss 3 values each.
  Serial.println("Tareing....");
  for (uint8_t t=0; t<3; t++) {
    hx.tareA(hx.readChannelRaw(CHAN_A_GAIN_128));
    hx.tareA(hx.readChannelRaw(CHAN_A_GAIN_128));
    hx.tareB(hx.readChannelRaw(CHAN_B_GAIN_32));
    hx.tareB(hx.readChannelRaw(CHAN_B_GAIN_32));
  }
}

// Called once to initializae all of the HX711s.
void setupScale() {
  setupHx711(hx711_1);
  setupHx711(hx711_2);
  setupHx711(hx711_3);
}

// Called to read a value from a single HX711.
float readHx711(Adafruit_HX711 hx) {
  int32_t weightA128 = hx.readChannelBlocking(CHAN_A_GAIN_128);
  Serial.print("Channel A (Gain 128): ");
  Serial.println(weightA128);

  // Read from Channel A with Gain 128, can also try CHAN_A_GAIN_64 or CHAN_B_GAIN_32
  int32_t weightB32 = hx.readChannelBlocking(CHAN_B_GAIN_32);
  Serial.print("Channel B (Gain 32): ");
  Serial.println(weightB32);

  return weightA128 + weightB32;
}

// Called to read a value from the scale.
float readScale() {
  float weight1 = readHx711(hx711_1);
  float weight2 = readHx711(hx711_2);
  float weight3 = readHx711(hx711_3);
  return weight1 + weight2 + weight3;
}

void post() {
  Serial.println("Posting the result");
  String contentType = "application/x-www-form-urlencoded";
  String postData = "client_id=12345&client_secret=someuuid&weight=1234";
  client.post("/api/1.0/", contentType, postData );

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
}

/// The setup function runs once when you press reset or power the board.
void setup() {
  setupWifi();
  setupScale();
}

/// The loop function runs continuously.
void loop() {
  float weight = readScale();

  // Rate limit.
  delay(1000);
}
