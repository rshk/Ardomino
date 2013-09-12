/////////////////////////////////////////////////////////////////////////////
//
// Arduino Sketch: Example HTTP POST with WiFly
//
// Library reference
//   https://github.com/adafruit/DHT-sensor-library
//   https://github.com/harlequin-tech/WiFlyHQ
//
/////////////////////////////////////////////////////////////////////////////

#include <WiFlyHQ.h>
#include <SoftwareSerial.h>
#include <dht.h>

// Macro Resettig for Microcontroller
#include <avr/io.h>
#include <avr/wdt.h>
#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}

// Reset every hour to prevent wifly comunication trouble
//#define Reset_After_1hour 3600000
//#define Periodo_Invio_Dati 3000     // minimun range time sending interval.(ms)

////////////////////////////////////////////////////////////
// Device configuration
////////////////////////////////////////////////////////////

const char DEVICE_NAME[] = "Ardomino Firenze";
const char DEVICE_LOCATION[] = "Firenze";

// WIFI SETTINGS wireless account RN 171 XV
const char WIFI_SSID[] = "Vodafone-26726417";
const char WIFI_PASSWORD[] = "xa5d59ivz3dbwi3";
//char MACnode[] = "00:06:66:71:d2:68";

// Server settings
//     serverName is used for address resolution
//     serverHostName is sent in the "Host:" header
const char SERVER_NAME[] = "ardomino-rshk.rhcloud.com"; // here you can put an IP
const char SERVER_HOST_NAME[] = "ardomino-rshk.rhcloud.com";
const int  SERVER_PORT = 80;

const char MY_USER_AGENT[] = "ArdOmino/0.1";
const char PAYLOAD[] = "{\"hello\": \"world\"}";
const int  PAYLOAD_SIZE = 18; // Length of the above string
const char PAYLOAD_CONTENT_TYPE[] = "application/json; charset=utf-8";



//char serverName[] = "http://149.139.8.55"; // Serve IP URL to connect

// Does DNS resolution work on Wifly? (I hope so..)

WiFly wifly;
char* macStr;
const char site_time[] = "hunt.net.nz"; // mh?

SoftwareSerial wifi(8, 9);

// Writing procedure
void print_P(const prog_char *str);
void println_P(const prog_char *str);

unsigned long time = 0;
unsigned long SendTime = 0;;

char rh_airBuffer[8];
char temp_airBuffer[8];
char timestamp[8];

// buffer to save for json string
char jsonMsgHead[128];
char jsonMsgBody[128];


void setup(){
    Serial.begin(9600);
    Serial.println("HTTP test sketch starting...");
    delay(1000);
    Serial.println("--- UP AND RUNNING ---");
    randomSeed(analogRead(A2));
}


void loop() {
    configure_wifi();
    send_data(jsonMsgBody, i);
    Reset_AVR();
}


/**
 * Wireless configuration and restart in error with next reset.
 * Function Author: Mirko Mancini
 */
int configure_wifi(){
    println_P(PSTR("Starting wifi configuration"));
    print_P(PSTR("Free memory: "));
    Serial.println(wifly.getFreeMemory(), DEC);

    wifi.begin(9600);    // define baud rate of Serial ArdOmino port
    if (!wifly.begin(&wifi, &Serial)) {
        println_P(PSTR("Failed to start wifly"));
	Reset_AVR();
    }

    char buf[32];
    /* Join wifi network if not already associated */
    if (!wifly.isAssociated()) {
	/* Setup the WiFly to connect to a wifi network */
	println_P(PSTR("Joining network"));
	wifly.setSSID(WIFI_SSID);
	//wifly.setPassphrase(WIFI_PASSWORD);
        wifly.setKey(WIFI_PASSWORD);
	wifly.enableDHCP();

	if (wifly.join()) {
	    println_P(PSTR("Joined wifi network"));
	} else {
	    println_P(PSTR("Failed to join wifi network"));
	    Reset_AVR();
	}
    } else {
        println_P(PSTR("Already joined network"));
    }

    Serial.print("MAC: ");
    macStr = (char *)(wifly.getMAC(buf, sizeof(buf)));
    Serial.println(macStr);

    Serial.print("IP: ");
    Serial.println(wifly.getIP(buf, sizeof(buf)));

    Serial.print("Netmask: ");
    Serial.println(wifly.getNetmask(buf, sizeof(buf)));

    Serial.print("Gateway: ");
    Serial.println(wifly.getGateway(buf, sizeof(buf)));

    Serial.print("SSID: ");
    Serial.println(wifly.getSSID(buf, sizeof(buf)));

    wifly.setDeviceID("Wifly-WebClient");

    Serial.print("DeviceID: ");
    Serial.println(wifly.getDeviceID(buf, sizeof(buf)));

    if (wifly.isConnected()) {
        println_P(PSTR("Old connection active. Closing"));
	wifly.close();
    }
}

// to print variable in memory
void print_P(const prog_char *str)
{
    char ch;
    while ((ch=pgm_read_byte(str++)) != 0) {
	Serial.write(ch);
    }
}

void println_P(const prog_char *str)
{
    print_P(str);
    Serial.println();
}

void send_header(char* name, char* value) {
  wifly.print(name);
  wifly.print(": ");
  wifly.print(value);
  wifly.print("\r\n");
}


// Sending Json formatted data procedure
// Waiting server's response to send next data packet.
// Function Author Mirko Mancini

void send_data() {
  // todo: we can calculate content-length inside this function
  // todo: can we use something smarter, like a "printf()", for sending headers?

  char buf[1024];

  Serial.println("Sending HTTP POST request");

  // Open connection to server
  if (wifly.open(serverName, serverPort)) {
    Serial.print("Connected to ");
    Serial.println(serverName);
  } else {
    Serial.println("Failed to connect");
    Reset_AVR();
    return;
  }

  wifly.print("POST / HTTP/1.0\r\n");

  sprintf(buf, "%s:%d", SERVER_HOST_NAME, SERVER_PORT)
  send_header("Host", buf);

  send_header("Content-type", PAYLOAD_CONTENT_TYPE);
  send_header("User-agent", MY_USER_AGENT);

  sprintf(buf, "%d", PAYLOAD_SIZE);
  send_header("Content-length", buf);

  wifly.print("\r\n"); // End of headers

  wifly.print(PAYLOAD);

  // We need to pass the Host header, on most web servers
  wifly.print("Host: ");
  wifly.print(serverHostName);
  wifly.print("\r\n");

  wifly.print("Content-Length: ");
  wifly.print(lungh);
  wifly.print("\r\n");

  wifly.print("\r\n");
  //wifly.print(jsonStringHead);
  wifly.print(jsonStringBody);


  Serial.println("Waiting server 's response");


  // Loop while reading response
  while (1) {
    if (client.available()) {
      char c = wifly.read();
      Serial.print(c);
    }
    if (!client.connected()) {
      Serial.println("Disconnecting...");
      wifly.stop();
      return;
    }
  };



  // Waiting server 's response
  // while (wifly.available() == 0) {
  //   delay(50);
  // }

  // if (wifly.available() > 0) {
  //   char buf[200] = "buffer";
  //   int exit = 0;

  //   while (exit < 2){
  //     wifly.gets(buf, sizeof(buf));
  //     Serial.println(buf);
  //     if(buf[0]==0){
  // 	exit++;
  //     }
  //     // if(buf[0]=='{') {
  //     // 	delay(50);
  //     // 	long timeSend = parsingJSONString(buf, sizeof(buf));
  //     // 	timeSend *= 1000;
  //     // 	// Wating this value to reset microcontroller
  //     // 	delay(timeSend);
  //     // }
  //   }
  // }
}

// function for raw JSON  parsing code Author Mirko Mancini
// long parsingJSONString(char buffer[], int len){
//     int k;
//     unsigned long m;

//     for (int i = 0; i < len; i++) {
//       if (
// 	  (buffer[i]=='"') &&
// 	  (buffer[i+1]=='c') &&
// 	  (buffer[i+2]=='f') &&
// 	  (buffer[i+3]=='g') &&
// 	  (buffer[i+4]=='"')) {

// 	for (k = i + 23; buffer[k] != '"'; k++) {
// 	  Serial.print(buffer[k]);
// 	}

// 	unsigned long value = 0;
// 	Serial.println();

// 	for(int l = k - 1, m = 1; l > i + 22; l--){
// 	  value += m * ((int)buffer[l] - 48);
// 	  m *= 10;
// 	}

// 	return value;
//       }
//     }

//     return 0;
// }
