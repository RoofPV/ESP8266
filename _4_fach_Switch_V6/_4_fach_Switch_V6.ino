/*Based on: http://www.esp8266.com/viewtopic.php?f=29&t=8746
  Controll two 12 Volt outputs and one 14 Volt input for 2 garden lights and power supply input
  Measures Battery voltage

  It connects to an MQTT server then:
  - on 0 switches off relays
  - on 1 switches on relays

  - sends 0 on off relays
  - sends 1 on on relays

Board: NodeMCU Lolin
Settings:NodeMCU 1.0 (ESP-12E Module
4M (3M SPIFFS)
80 MHz
v2 lower memory
Upload speed: 115200

Relay:
4 Relay Module

Wiring:
D3 (GPIO0): To output toggle button, pressing button will connect D3 to ground

D5 (GPIO(14):to input 1 of relay board, connected to 3V via 3.3k pull-up resistor
D6 (GPIO(12):to input 2 of relay board, connected to 3V via 3.3k pull-up resistor
D7 (GPIO(13):to input 3 of relay board, connected to 3V via 3.3k pull-up resistor
D1 (GPIO(5): to input 4 of relay board, connected to 3V via 3.3k pull-up resistor

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

const char* outrelay4 = "switch/garden/fountain/state";
const char* inrelay4 = "switch/garden/fountain/set";
const char* outrelay3 = "switch/garden/floodlight/state";
const char* inrelay3 = "switch/garden/floodlight/set";
const char* outrelay2 = "switch/garden/center_wall/state";
const char* inrelay2 = "switch/garden/center_wall/set";
const char* outrelay1 = "switch/garden/garage_slope/state";
const char* inrelay1 = "switch/garden/garage_slope/set";

//Use of D1 =GPIO(5) for relais output1
int relay_pin4 = 5;
//Use of D5 =GPIO(14) for relais output2
int relay_pin1 = 14;
//Use of D6 =GPIO(12) for relais output3
int relay_pin2 = 12;
//Use of D7 =GPIO(13) for relais output4
int relay_pin3 = 13;



//Use of D3 = GPIO(0) for button
int button_pin = 0;

//Use of D4 =GPIO(2) for internal LED
int led_pin = 2;
bool relayState = HIGH;

// Instantiate a Bounce object :
Bounce debouncer = Bounce();

const char* host = "4xSwitch";


void setup() {
  pinMode(relay_pin1, OUTPUT);     // Initialize the relay pin1 as an output
  pinMode(relay_pin2, OUTPUT);     // Initialize the relay pin2 as an output
  pinMode(relay_pin3, OUTPUT);     // Initialize the relay pin3 as an output
  pinMode(relay_pin4, OUTPUT);     // Initialize the relay pin4 as an output



  pinMode(button_pin, INPUT_PULLUP);     // Initialize the button pin as input
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, HIGH);    // start with on-board LED off

  // start with relay state HIGH (LED's and relay off)
  digitalWrite(relay_pin1, relayState);
  digitalWrite(relay_pin2, relayState);
  digitalWrite(relay_pin3, relayState);
  digitalWrite(relay_pin4, relayState);


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


  digitalWrite(led_pin, LOW);          // Blink to indicate setup completed
  delay(2000);
  digitalWrite(led_pin, HIGH);

  Serial.println("Setup done");
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

  if (strcmp(topic, inrelay2) == 0) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    // Switch off the Relay2 if "0" was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(relay_pin2, HIGH);
      client.publish(outrelay2, "0");


      // Switch on the Relay2 if "1" was received as first character
    } else if ((char)payload[0] == '1') {
      digitalWrite(relay_pin2, LOW);
      client.publish(outrelay2, "1");

    }
  }

  if (strcmp(topic, inrelay3) == 0) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    // Switch off the Relay3 if "0" was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(relay_pin3, HIGH);
      client.publish(outrelay3, "0");


      // Switch on the Relay3 if "1" was received as first character
    } else if ((char)payload[0] == '1') {
      digitalWrite(relay_pin3, LOW);
      client.publish(outrelay3, "1");

    }
  }

  if (strcmp(topic, inrelay4) == 0) {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    // Switch off the Relay4 if "0" was received as first character
    if ((char)payload[0] == '0') {
      digitalWrite(relay_pin4, HIGH);
      client.publish(outrelay4, "0");


      // Switch on the Relay4 if "1" was received as first character
    } else if ((char)payload[0] == '1') {
      digitalWrite(relay_pin4, LOW);
      client.publish(outrelay4, "1");

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
      client.subscribe(inrelay2);
      client.subscribe(inrelay3);
      client.subscribe(inrelay4);
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
    digitalWrite(relay_pin2, relayState);
    digitalWrite(relay_pin3, relayState);
    digitalWrite(relay_pin4, relayState);


    if (relayState == 0) {
      Serial.println("relay_pins -> HIGH");
      client.publish(outrelay1, "1");
      client.publish(outrelay2, "1");
      client.publish(outrelay3, "1");
      client.publish(outrelay4, "1");
    }
    else if (relayState == 1) {
      Serial.println("relay_pins -> LOW");
      client.publish(outrelay1, "0");
      client.publish(outrelay2, "0");
      client.publish(outrelay3, "0");
      client.publish(outrelay4, "0");
    }
  }
}





