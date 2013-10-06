/////////////////////////////////////////////////////////////////////////////
//
// Arduino Sketch for the Ardomino Project.
//
// NOTE: This is an experimental "new version" that communicates
//       plain text lines over a TCP socket.
//       To be merged with the original sketch in alfcrisci/ardomino.
//
// Biometeorological ArdOmino Skecth to monitor Air Temperature
// and Relative Humidity.
//
// Application Case https://github.com/alfcrisci/Ardomino presented
// OfficinaIbimet IBIMET CNR http://www.fi.ibimet.cnr.it/
//
// Author: Alessandor Matese - Alfonso Crisci - OfficinaIbimet
//
// General scheme and function are done by Mirko Mancini in his work
// thesis DESIGN AND IMPLEMENTATION OF AN AMBIENT INTELLIGENCE SYSTEM BASED
// ON ARDUINO AND ANDROID Universita di Parma FAC. DI INGEGNERIA CORSO DI
// LAUREA IN INGEGNERIA INFORMATICA
//
// Mirko Mancin <mirkomancin90@gmail.com>
//   website:  www.mancio90.it
//   forum http://forum.arduino.cc/index.php?topic=157524.0;wap2
//
// Samuele Santi <redshadow@hackzine.org>
//   Author of this experimental version, trying to solve some problems
//   (mostly with wifi connectivity).
//   website: http://hackzine.org - http://samuelesanti.com
//   github: https://github.com/rshk
//
// Library reference
//   https://github.com/adafruit/DHT-sensor-library
//   https://github.com/harlequin-tech/WiFlyHQ
//
/////////////////////////////////////////////////////////////////////////////


#include "settings.h"  // See the README file for more information

const int MAX_CONNECTION_TIME = 30000; // In milliseconds


// Communication with the DHT
//------------------------------------------------------------

#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
float sval_humidity, sval_temperature;


/**
 * Setup the communication with DHT sensor
 */
void setup_dht() {
  Serial.println("Setting up DHT module");
  dht.begin();
}


/**
 * Read sensor data from the DHT
 */
void loop_dht() {
  sval_humidity = dht.readHumidity();
  sval_temperature = dht.readTemperature();
}



// Serial-port communication
//------------------------------------------------------------

void setup_serial() {
  Serial.begin(115200);
  Serial.println("--- Ardomino Serial Console ---");
}


/**
 * Send read data through the serial port
 */
void loop_serial() {
  if (isnan(sval_humidity) || isnan(sval_temperature)) {
    Serial.println("ERROR: Failed reading values from DHT");
  }
  else {
    Serial.print("DATA:");
    Serial.print(" humidity=");
    Serial.print(sval_humidity);
    Serial.print(" temperature=");
    Serial.print(sval_temperature);
    Serial.println();
  }
}


// Wifi communication (via WiFly module)
//------------------------------------------------------------

#include <WiFlyHQ.h>
#include <SoftwareSerial.h>

SoftwareSerial wifiSerial(8,9);
WiFly wifly;

// Todo: read these from some configuration file!
const char mySSID[] = WIFI_SSID;
const char myPassword[] = WIFI_PASSWORD;

// The server to which to POST the data..
const char site[] = SERVER_ADDR;
const int site_port = SERVER_PORT;

void terminal();

void setup_wifly() {
    char buf[32];

    Serial.println("WiFly Module initialization");
    Serial.print("    Free memory: ");
    Serial.println(wifly.getFreeMemory(),DEC);

    wifiSerial.begin(9600);
    if (!wifly.begin(&wifiSerial, &Serial)) {
        Serial.println("    ERROR: Failed to start wifly");
	terminal();
    }

    /* Join wifi network if not already associated */
    if (!wifly.isAssociated()) {
	/* Setup the WiFly to connect to a wifi network */
	Serial.println("    INFO: Joining network");
	wifly.setSSID(mySSID);
	wifly.setPassphrase(myPassword);
	wifly.enableDHCP();

	if (wifly.join()) {
	    Serial.println("    INFO: Joined wifi network");
	} else {
	    Serial.println("    ERROR: Failed to join wifi network");
	    terminal();
	}
    } else {
        Serial.println("    INFO: Already joined network");
    }

    //terminal();

    Serial.print("    MAC: ");
    Serial.println(wifly.getMAC(buf, sizeof(buf)));
    Serial.print("    IP: ");
    Serial.println(wifly.getIP(buf, sizeof(buf)));
    Serial.print("    Netmask: ");
    Serial.println(wifly.getNetmask(buf, sizeof(buf)));
    Serial.print("    Gateway: ");
    Serial.println(wifly.getGateway(buf, sizeof(buf)));

    wifly.setDeviceID("Wifly-WebClient");
    Serial.print("    DeviceID: ");
    Serial.println(wifly.getDeviceID(buf, sizeof(buf)));

    if (wifly.isConnected()) {
        Serial.println("    INFO: Old connection active. Closing");
	wifly.close();
    }

    // Try to make a HTTP request..
    if (wifly.open(site, site_port)) {
        Serial.print("    INFO: Connected to ");
	Serial.println(site);

	/* Send the request */
	// wifly.println("GET / HTTP/1.0");
	// wifly.println();
	wifly.println("HELLO");
    } else {
        Serial.println("    ERROR: Failed to connect");
    }
}

uint32_t connectTime = 0;

void loop_wifly() {

  int available;

  // If not already connected, we need to establish
  // a TCP connection to the server

  if (wifly.isConnected() == false) {
	Serial.println("INFO: WiFly: Connecting to server");

	if (wifly.open(site, site_port)) {
	  Serial.print("INFO: WiFly: Connected to ");
	  connectTime = millis(); // ???
	}
	else {
	    Serial.print("ERROR: WiFly: Failed to connect to ");
	}
	Serial.print(site);
	Serial.print(":");
	Serial.println(site_port);
  }
  else {
    available = wifly.available();

    if (available < 0) {
      Serial.println("WARNING: WiFly: Disconnected");
    }

    else {
      // Print data from WiFly to serial port
      if (available > 0) {
	String recvdata = "";
        while (wifly.available() > 0) {
	  recvdata += wifly.read();
	}
	Serial.print("INFO: WiFly: [data] ");
	Serial.print(recvdata);
	Serial.println(" [/data]");
      }

      // Send sensor data
      Serial.println("INFO: WiFly: Sending sensors data");
      wifly.print("DATA:");
      wifly.print(" humidity=");
      wifly.print(sval_humidity);
      wifly.print(" temperature=");
      wifly.print(sval_temperature);
      wifly.println();

      // Disconnect after 10 seconds
      if ((millis() - connectTime) > MAX_CONNECTION_TIME) {
	Serial.println("INFO: WiFly: Disconnecting...");
	wifly.close();
      }
    }

    /* Send data from the serial monitor to the TCP server */
    // if (Serial.available()) {
    //   wifly.write(Serial.read());
    // }
  }
}


// Connect the WiFly serial to the serial monitor.
void terminal() {
  Serial.println("Opening WiFly <-> Serial communication..");
    while (1) {
	if (wifly.available() > 0) {
	    Serial.write(wifly.read());
	}
	if (Serial.available() > 0) {
	    wifly.write(Serial.read());
	}
    }
}





// Standard setup/loop functions
//------------------------------------------------------------

void setup() {
  setup_serial();
  setup_dht();
  setup_wifly();
  Serial.println("*** Initialization done");
}


void loop() {
  loop_dht();
  loop_serial();
  loop_wifly();
  delay(500);
}
