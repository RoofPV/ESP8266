/*

  It connects to an MQTT server then:
  - on 0 switches off relays
  - on 1 switches on relays

  - sends 0 on off relays
  - sends 1 on on relays


Sonoff S20 Belegung:
GPIO0 Taster
GPIO12 Relais (high = on) + LED (blau); LED nur bei S20
GPIO13 LED grün (low = on)

Hardware:
ESP8266
PN25F08 Flash (1MByte/8MBit)
10A Relais
Taster
LED grün – grün/blau S20

Programmierpins (Steckdose links/Relaisseite):
von links: GND - Tx - Rx - Vcc

LED's: gruen liegt ueber Taster, blau liegt unter Taster



Board: ESP-01S (Generic ESP-8266 Moduke)

Settings:Generic 8266 Module
Flashmode: DOUT
1M (128k SPIFFS)
Crystal frequency: 26 MHz
Flash frequency: 40 MHz
CPU frequency: 80 MHz
v2 lower memory
reset method: ck
Buidin LED: "2"
Upload speed: 115200



Outputs on HIGH means blue LED and relay is on

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Bounce2.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>

//MQTT
const char* mqtt_server = "watchdog.fritz.box";
WiFiClient espClient;
PubSubClient client(espClient);
// WebUpdater
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;


const char* outrelay = "switch/sonoff_S20_1/state";
const char* inrelay = "switch/sonoff_S20_1/set";


//Use of GPIO(12) for relais output and blue LED
int relay_pin = 12;
bool relayState = LOW;

//Use of GPIO(0) for button
int button_pin = 0;

//Use of GPIO(13) for green LED, on=WLAN connected
int led_pin_green = 13;




// Instantiate a Bounce object :
Bounce debouncer = Bounce();

const char* host = "Sonoff_S20_1";


void setup() {
  pinMode(relay_pin, OUTPUT);     // Initialize the relay pin blue as an output (for relais and blue LED)
  pinMode(led_pin_green, OUTPUT);     // Initialize the relay pin green as an output
  digitalWrite(led_pin_green, HIGH);


  pinMode(button_pin, INPUT_PULLUP);     // Initialize the button pin as input


  // start with relay state LOW (blue LED and relay off)
  digitalWrite(relay_pin, relayState);

  debouncer.attach(button_pin);   // Use the bounce2 library to debounce the built in button
  debouncer.interval(50);         // Input must be low for 50 ms

  Serial.begin(9600);
  Serial.println("abcd");

  WiFiManager wifiManager;
  wifiManager.autoConnect("MQTT");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //WebUpdater
  MDNS.begin(host);

  httpUpdater.setup(&httpServer);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);

}



void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  extButton();
  //WebUpdater
  httpServer.handleClient();
  unsigned long uptime = millis();
  static unsigned long last_loop;
  unsigned long diff = uptime - last_loop;

  if (diff < 5000) return;
  last_loop = uptime;
}



void callback(char* topic, byte* payload, unsigned int length) {

  if (strcmp(topic, inrelay) == 0) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    // Switch off the Relay if "0" was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(relay_pin, LOW);
      client.publish(outrelay, "0");


      // Switch on the Relay if "1" was received as first character
    } else if ((char)payload[0] == '1') {
      digitalWrite(relay_pin, HIGH);
      client.publish(outrelay, "1");

    }
  }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(host)) {
      Serial.println("connected");
      digitalWrite(led_pin_green, LOW);
      client.subscribe(inrelay);
    }   else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      digitalWrite(led_pin_green, HIGH);
      delay(5000);
    }
  }
}


void extButton() {
  debouncer.update();

  // Call code if Bounce fell (transition from HIGH to LOW) :
  if ( debouncer.fell() ) {
    Serial.println("Debouncer fell");
    // Toggle relay state :
    relayState = !relayState;
    digitalWrite(relay_pin, relayState);


    if (relayState == 0) {
      Serial.println("relay_pin -> LOW");
      client.publish(outrelay, "0");
    }
    else if (relayState == 1) {
      Serial.println("relay_pin -> HIGH");
      client.publish(outrelay, "1");
    }
  }
}





