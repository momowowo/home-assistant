/**
 *
 * DESCRIPTION
 * MySensors ESP328266 Gateway plus locally attached button/switch sensors
 * Reads the state of a button switch. If the button is pressed, it will send a message to the GW with a state of 1
 * Written for a WEMOS D1 Mini (ESP8266 chipset + dev board) with three buttons wired to the GPIO pins:
 * D3 - Office lights on
 * D4 - All on
 * D5 - All off
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
const int buttonPin[3] = {D3, D4, D5}; // Array of Wemos Digital I/O pin numbers
#define NUMBER_OF_SWITCHES 3 // Total number of attached switches
#define SWITCH_ON 1  // Value to pass back when the switch is pressed
#define SWITCH_OFF 0 // Value to pass back to indicate the switch is not currently being pressed.
                     // We expect the switch state to be set to SWITCH_OFF in HA as well, when it is not being pressed

uint32_t SLEEP_TIME = 3000; // Sleep time between each button press for same button (in milliseconds)

void setup()
{
  Serial.println("entered setup");

  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
}

void presentation()
{
  // Present locally attached sensors here

  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("GWBigRedButton", "1.0");

  for (int sensor=1; sensor<=NUMBER_OF_SWITCHES; sensor++) {
    // Register all sensors to gw (they will be created as child devices)
    present(sensor, S_BINARY);

    //Send initial message back to GW (HA required)
    MyMessage msg(sensor, V_STATUS);
    send(msg.set(SWITCH_OFF));
  }

  Serial.println("Presentation complete");

}

void loop()
{
  // Send locally attached sensors data here

  // read the state of the pushbutton value:
  for (int i=0; i<NUMBER_OF_SWITCHES; i++) {
    int buttonState = digitalRead(buttonPin[i]); //Array of D3, D4, D5

    // check if the pushbutton is pressed. If it is, the buttonState is LOW:
    if (buttonState == LOW) {
      Serial.println("Read button press");
      //Send message back to GW / HA
      MyMessage msg(i+1, V_STATUS); // Sensor numbering with the GW starts at 1, not 0
      send(msg.set(SWITCH_ON));
      delay(1000);
      send(msg.set(SWITCH_OFF)); // Config in HA is set to ignore state being changed to SWITCH_OFF
      delay(2000);
     }
  } //end for loop

}
