/**
 *
 * DESCRIPTION
 * MySensors ESP328266 Gateway plus locally attached relays
 * Written for a WEMOS D1 Mini (ESP8266 chipset + dev board)
 *
 */

// Enable debug prints to serial monitor
#define MY_DEBUG

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 9600

// Enables and select radio type (if attached)
//#define MY_RADIO_NRF24

#define MY_GATEWAY_ESP8266

#define MY_ESP8266_SSID "*****"
#define MY_ESP8266_PASSWORD "*****"

// Enable UDP communication
//#define MY_USE_UDP  // If using UDP you need to set MY_CONTROLLER_IP_ADDRESS below

// Set the hostname for the WiFi Client. This is the hostname
// it will pass to the DHCP server if not static.
//#define MY_ESP8266_HOSTNAME "sensor-gateway"

// Enable MY_IP_ADDRESS here if you want a static ip address (no DHCP)
#define MY_IP_ADDRESS 1,2,3,4

// If using static ip you can define Gateway and Subnet address as well
//#define MY_IP_GATEWAY_ADDRESS 192,168,178,1
//#define MY_IP_SUBNET_ADDRESS 255,255,255,0

// The port to keep open on node server mode
#define MY_PORT 5003

// How many clients should be able to connect to this gateway (default 1)
#define MY_GATEWAY_MAX_CLIENTS 2

// Controller ip address. Enables client mode (default is "server" mode).
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere.
//#define MY_CONTROLLER_ 192, 168, 178, 68

#if defined(MY_USE_UDP)
#include <WiFiUdp.h>
#endif

#include <ESP8266WiFi.h>
#include <MySensors.h>

//Adding in locally attached sensors
//Note on pin numbering:
//Numeric pin number (e.g. "4") refers to the ESP-8266 GPIO pin number (e.g. GPIO4), and they don't necessarily line up in order on the board
//So you need to double-check with a pinout diagram for Wemos D1 mini when adding more relays
//Or use the "D4" style numbering to correspond to the actual pins on the Wemos board
//RELAY_PIN 4 = Relay is attached to D2 on the Wemos D1 mini.
#define RELAY_PIN 4  // Arduino Digital I/O pin number for first relay (second on pin+1 etc).
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

void before()
{
  Serial.println("Entered before()");
  for (int sensor=1, pin=RELAY_PIN; sensor<=NUMBER_OF_RELAYS; sensor++, pin++) {
    // Then set relay pins in output mode
    pinMode(pin, OUTPUT);
    // Set relay to last known state (using eeprom storage)
    //digitalWrite(pin, !loadState(sensor)?RELAY_ON:RELAY_OFF); --> Might need to invert if using the blue relays
    digitalWrite(pin, loadState(sensor)?RELAY_ON:RELAY_OFF);
  }
}

void setup()
{
  Serial.println("Entered setup()");
  // Setup locally attached sensors
}

void presentation()
{
  Serial.println("Entered presentation()");
	// Present locally attached sensors here

  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("GWRelay", "1.0");

  for (int sensor=1, pin=RELAY_PIN; sensor<=NUMBER_OF_RELAYS; sensor++, pin++) {
    // Register all sensors to gw (they will be created as child devices)
    present(sensor, S_BINARY);

    //Send initial message back to GW (HA required)
    MyMessage msg(sensor, V_STATUS);
    send(msg.set(loadState(sensor)?RELAY_ON:RELAY_OFF));
  }
}

void loop()
{
	// Send locally attached sensors data here

}

void receive(const MyMessage &message)
{
    Serial.println("Entered receive()");
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_STATUS) {
    // Change relay state
    //digitalWrite(message.sensor-1+RELAY_PIN, !message.getBool()?RELAY_ON:RELAY_OFF); --> Might need to invert if using the blue relays
    digitalWrite(message.sensor-1+RELAY_PIN, message.getBool()?RELAY_ON:RELAY_OFF);

    //Send message back to GW because HA optimistic=false
    MyMessage msg(message.sensor, V_STATUS);
    send(msg.set(message.getBool()?RELAY_ON:RELAY_OFF));

    // Store state in eeprom
    saveState(message.sensor, message.getBool());
    // Write some debug info
    Serial.print("Incoming change for sensor:");
    Serial.print(message.sensor);
    Serial.print(", New status: ");
    Serial.println(message.getBool());
  }
}
