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
char server[] = "mikesimms.net"; // name address for Google (using DNS)
const int port = 80;

// Initialize the Ethernet client library with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient wifi;
HttpClient client = HttpClient(wifi, server, port);

// HX711 circuit wiring
const int LOADCELL1_DOUT_PIN = 2;
const int LOADCELL1_SCK_PIN = 3;

// HX711 object.
Adafruit_HX711 hx711(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN);

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

void setupScale() {
  // Initialize the HX711
  hx711.begin();

  // read and toss 3 values each
  Serial.println("Tareing....");
  for (uint8_t t=0; t<3; t++) {
    hx711.tareA(hx711.readChannelRaw(CHAN_A_GAIN_128));
    hx711.tareA(hx711.readChannelRaw(CHAN_A_GAIN_128));
    hx711.tareB(hx711.readChannelRaw(CHAN_B_GAIN_32));
    hx711.tareB(hx711.readChannelRaw(CHAN_B_GAIN_32));
  }
}

void readScale() {
  int32_t weightA128 = hx711.readChannelBlocking(CHAN_A_GAIN_128);
  Serial.print("Channel A (Gain 128): ");
  Serial.println(weightA128);

  // Read from Channel A with Gain 128, can also try CHAN_A_GAIN_64 or CHAN_B_GAIN_32
  int32_t weightB32 = hx711.readChannelBlocking(CHAN_B_GAIN_32);
  Serial.print("Channel B (Gain 32): ");
  Serial.println(weightB32);
}

void post() {
  Serial.println("Posting the result");
  String contentType = "application/x-www-form-urlencoded";
  String postData = "client_id=12345&client_secret=someuuid&weight=1234";
  client.post("/api/1.0/", contentType, postData );

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
}

void setup() {
  setupWifi();
  setupScale();
}

void loop() {
  readScale();
}
