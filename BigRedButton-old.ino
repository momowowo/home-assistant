#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>

const char* ssid = "*****";
const char* password = "*****";

const String host = "test.local";
const int port = 1234;
IPAddress ip(10,10,10,10);

// constants won't change. They're used here to set pin numbers:
const int buttonPin = D3;     // the number of the pushbutton pin

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

//Init WifiClient
WiFiClient client;


void setup() {
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  // make wifi connection
  Serial.begin(115200);
  Serial.println();
  Serial.println("connecting to " + String(ssid));
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
  }
  // status bar ...
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

/*
- Included but commented out because it seemed to be working but then broke due to the IP address field being blank, even when the service was found, and throwing an exception.
- May have broke due to the host having a statically assigned IP

  // mDNS stuff
  // hostString will be used to identify this device,
  // but not relevant as we're not providing mDNS services
  char hostString[16] = {0};
  if (!MDNS.begin(hostString)) {
    Serial.println("Error setting up MDNS responder!");
  }

  // Get IP address from hostname
  // We now query our network for the 'ssh' service, since we know we have it enabled
  // over tcp, and get the number of available devices
  int n = MDNS.queryService("ssh", "tcp");
  if (n == 0) {
    Serial.println("no services found - could not do mDNS resolution");

  }
  else {
    for (int i = 0; i < n; ++i) {
      // Going through every available service,
      // we're searching for the one whose hostname
      // matches what we want, and then get its IP
      Serial.print("mDNS found hostname: " + MDNS.hostname(i));
      Serial.print(" ip: " + MDNS.IP(i));
      Serial.println(" port: " + MDNS.port(i));
      if (MDNS.hostname(i) == host) {
        ip = MDNS.IP(i);
        Serial.println("Found IP: " + ip.toString());
      }
    }
  }
*/

  //Check that the API is up
  checkAPIStatus();
}

void loop() {

  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }


  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {
    Serial.println("Read button press");
      int state = getLightState();
      if (state == 0) {
        setLightState("on");
        Serial.println("lights turned on");
      } else {
        setLightState("off");
        Serial.println("lights turned off");
      }
      delay(3000);
   }

}

int getLightState() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(ip, port)) {
    Serial.println("getLightState: connecting to " + host + "/api");
    // send the HTTP request:
    client.println("GET /api/states/group.all_lights HTTP/1.1");
    client.println("Host: " + host);
    client.println("User-Agent: ESP8266");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    //print out debug info
    Serial.println(String("GET /api/states/group.all_lights HTTP/1.1\r\n");

    //Read in response, line by line
    //Once the blank line is reach (\r\n), the header block should be done
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
      if (line == "\r") {
        Serial.println("headers received for get light state");
        break;
      }
    }

    //Now do the same for the body
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
      if (line.indexOf("state\": \"off") != -1) {
        Serial.print("found off ");
        Serial.println(line.indexOf(String("state: off")));
        client.flush();
        return 0;
      }
      else if (line.indexOf("state\": \"on") != -1) {
        Serial.println("found on");
        client.flush();
        return 1;
      }
      else {
        return -1;
      }
    }

    Serial.println("reached end of get light state");
    client.stop();
  }

}

void setLightState(String lightState) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(ip, port)) {
    Serial.println("setLightState connecting to " + host + "/api");
    // send the HTTP request:
    client.println("POST /api/services/switch/turn_" + lightState + " HTTP/1.1");
    client.println("Host: " + host + ":" + String(port));
    client.println("User-Agent: ESP8266");
    client.println("Accept: */*");
    client.println("Content-Type: application/json");
    //client.println("Content-Length: " + String(lightState.length() + 13));
    client.println("Content-Length: 33");
    client.println();
    client.println("{\"entity_id\": \"group.all_lights\"}");
    client.println();

    //print out debug info
    Serial.println(String("POST /api/states/group.all_lights HTTP/1.1\r\n");
    Serial.println("{\"state\": \"" + lightState + "\"}");


    //Read in response, line by line
    //Once the blank line is reach (\r\n), the header block should be done
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
      if (line == "\r") {
        Serial.println("headers received for set light state");
        break;
      }
    }

    //Now do the same for the body
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
      client.flush();
      break;
    }

    //Serial.println("reached end of set light state");
    client.stop();
  }

}

void checkAPIStatus() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(ip, port)) {
    Serial.println("checkAPIStatus connecting to " + host + "/api");
    // send the HTTP request:
    client.println("GET /api/ HTTP/1.1");
    client.println("Host: " + host);
    client.println("User-Agent: ESP8266");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    //print out debug info
    Serial.println(String("GET /api/ HTTP/1.1\r\n");

    //Read in response, line by line
    //Once the blank line is reach (\r\n), the header block should be done
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
      if (line == "\r") {
        Serial.println("headers received for api check");
        break;
      }
    }

    //Now do the same for the body
    while (client.connected()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
      if (line.indexOf("API running")) {
        Serial.println("API is up");
        client.flush();
        break;
      }
    }

    Serial.println("reached end of while loop");
    client.stop();

  }


}
