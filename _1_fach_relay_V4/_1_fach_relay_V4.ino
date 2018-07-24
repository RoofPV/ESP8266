/*

  It connects to an MQTT server then:
  - on 0 switches off relays
  - on 1 switches on relays

  - sends 0 on off relays
  - sends 1 on on relays

Board: ESP-01S (Generic ESP-8266 Moduke)
Settings:Generic 8266 Module
Flashmode: DIO
1M (64k SPIFFS)
80 MHz
v2 lower memory
reset method: nodemcu
Upload speed: 115200

Relay:
1 Relay Module

Wiring:

GPIO(0): for connecting to relay on relay board, also connected to green housing LED via 1,5h resistor. LED is on when relay is off
GPIO(2): to ground via button (internal pull-up resistor)


VU: to VCC of relais board
G: to GND of relais board
470µF between GND and Vin
47µf between GND an 3V 

Outputs on HIGH means LED's and relays off

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


const char* outrelay1 = "switch/garden/pump/state";
const char* inrelay1 = "switch/garden/pump/set";


//Use of GPIO(0) for relais output1
int relay_pin1 = 0;


//Use of GPIO(2) for button
int button_pin = 2;

//Use of D4 =GPIO(2) for internal LED
//int led_pin = 2;
bool relayState = HIGH;

// Instantiate a Bounce object :
Bounce debouncer = Bounce();

const char* host = "RelaySwitch1";


void setup() {
  pinMode(relay_pin1, OUTPUT);     // Initialize the relay pin1 as an output


  pinMode(button_pin, INPUT_PULLUP);     // Initialize the button pin as input


  // start with relay state HIGH (LED's and relay off)
  digitalWrite(relay_pin1, relayState);

  debouncer.attach(button_pin);   // Use the bounce2 library to debounce the built in button
  debouncer.interval(50);         // Input must be low for 50 ms

  Serial.begin(115200);

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

  if (strcmp(topic, inrelay1) == 0) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    // Switch off the Relay1 if "0" was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(relay_pin1, HIGH);
      client.publish(outrelay1, "0");


      // Switch on the Relay1 if "1" was received as first character
    } else if ((char)payload[0] == '1') {
      digitalWrite(relay_pin1, LOW);
      client.publish(outrelay1, "1");

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
      client.subscribe(inrelay1);
    }   else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
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
    digitalWrite(relay_pin1, relayState);


    if (relayState == 0) {
      Serial.println("relay_pins -> HIGH");
      client.publish(outrelay1, "1");
    }
    else if (relayState == 1) {
      Serial.println("relay_pins -> LOW");
      client.publish(outrelay1, "0");
    }
  }
}





