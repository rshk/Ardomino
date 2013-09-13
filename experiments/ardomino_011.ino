/////////////////////////////////////////////////////////////////////////////
// ArdOmino Sketch Test
// Biometeorological ArdOmino Skecth to monitor Air Temperature and Relative Humidity.
// Application Case https://github.com/alfcrisci/Ardomino presented
// OfficinaIbimet IBIMET CNR http://www.fi.ibimet.cnr.it/
// Author: Alessandor Matese - Alfonso Crisci - OfficinaIbimet
// General scheme and function are done by Mirko Mancini in his work thesis
// DESIGN AND IMPLEMENTATION OF AN AMBIENT INTELLIGENCE SYSTEM BASED ON ARDUINO AND ANDROID Universita di Parma FAC. DI INGEGNERIA CORSO DI LAUREA IN INGEGNERIA INFORMATICA
// Mirko Mancin - mirkomancin90@gmail.com  website:  www.mancio90.it forum http://forum.arduino.cc/index.php?topic=157524.0;wap2
// Library reference
// https://github.com/adafruit/DHT-sensor-library
// https://github.com/harlequin-tech/WiFlyHQ

/////////////////////////////////////////////////////////////////////////////

// Library definition

#include <WiFlyHQ.h>
#include <SoftwareSerial.h>
#include "DHT.h" // Arduino-DHT

DHT dht; // global variable declaration for sensor's class

// Macro Resettig for Microcontroller

#include <avr/io.h>
#include <avr/wdt.h>
#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}

// Reset every hour to prevent wifly comunication trouble

#define Reset_After_1hour 3600000
#define Periodo_Invio_Dati 3000     // minimun range time sending interval.(ms)

// WIFI SETTINGS wireless account RN 171 XV

WiFly wifly;

char mySSID[] = "Vodafone-26726417";
char myPassword[] = "xa5d59ivz3dbwi3";
char MACnode[] = "00:06:66:71:d2:68";
char SERVER_NAME[] = "ardomino-rshk.rhcloud.com"; // here you can put an IP
char SERVER_HOST_NAME[] = "ardomino-rshk.rhcloud.com";
int  SERVER_PORT = 80;
char MY_USER_AGENT[] = "ArdOmino/0.1";
char PAYLOAD_CONTENT_TYPE[] = "application/json;";
char DEVICE_NAME[] = "Ardomino_Firenze";
char DEVICE_LOCATION[] = "Piazza Duomo";

char* macStr;
const char site_time[] = "hunt.net.nz";
char buf[1024];

SoftwareSerial wifi(8,9);


//SENSOR SETTINGS

unsigned long time = 0;
unsigned long SendTime = 0;;

char rh_airBuffer[8];
char temp_airBuffer[8];
char timestamp[8];

// buffer to save for json string
char jsonMsgBody[128];

void setup(){
  Serial.begin(9600);
  Serial.println(" --- ARDOMINO MONITORING --- ");
  delay(1000);
  Serial.println("Ardomino  started, End Setup !");
  randomSeed(analogRead(A2));
  dht.setup(2); // Pin defining for DHT22 sensors and variables

}


void loop(){
    delay (100);
    time = millis();
    configWIFI();
    SendTime = millis();

    delay(dht.getMinimumSamplingPeriod());
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();
    int tmp=0;
    tmp = analogRead(0);

  // value to string conversion

    dtostrf(temperature, 5, 2, temp_airBuffer);
    dtostrf(humidity , 5, 2, rh_airBuffer);


    // Create json strings for HTTP POST

	Serial.println("Create  JSON strings");

	    sprintf(jsonMsgBody,
"{"
"\"device_name\":\"%s\","
"\"location\":\"%s\","
"\"sensor_name\":\"%s\","
"\"sensor_value\":\"%s\","
"\"sensor_units\":\"%s\"" // Beware: no comma!
"}",
DEVICE_NAME,
DEVICE_LOCATION,
"temperature",
temp_airBuffer,
"degC");

    Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)");
    Serial.print(dht.getStatusString());
    Serial.print("\t");
    Serial.print(humidity, 1);
    Serial.print("\t\t");
    Serial.print(temperature, 1);
    Serial.print("\t\t");
    Serial.println(dht.toFahrenheit(temperature), 1);
    Serial.print(jsonMsgBody);
    Serial.println(" ");

    // JSON string length assessement to make HTTP POST
    int i;
    for(int i=0;jsonMsgBody[i]!=0; i++);

    Serial.println("Send sensors readings....");


    InvioWIFIHttp(jsonMsgBody,i);


   Serial.println("Sensors Readings Sent!.");
   Reset_AVR();

}





// Wireless configuration and restart in error with next reset.
// Function Author: Mirko Mancini


void configWIFI(){
    println_P(PSTR("Starting"));
    print_P(PSTR("Free memory: "));
    Serial.println(wifly.getFreeMemory(),DEC);

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
	wifly.setSSID(mySSID);
	wifly.setPassphrase(myPassword);
    //wifly.setKey(myPassword);
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

    print_P(PSTR("MAC: "));
    macStr = (char *)(wifly.getMAC(buf, sizeof(buf)));
    Serial.println(macStr);
    print_P(PSTR("IP: "));
    Serial.println(wifly.getIP(buf, sizeof(buf)));
    print_P(PSTR("Netmask: "));
    Serial.println(wifly.getNetmask(buf, sizeof(buf)));
    print_P(PSTR("Gateway: "));
    Serial.println(wifly.getGateway(buf, sizeof(buf)));
    print_P(PSTR("SSID: "));
    Serial.println(wifly.getSSID(buf, sizeof(buf)));

    wifly.setDeviceID("Wifly-WebClient");
    print_P(PSTR("DeviceID: "));
    Serial.println(wifly.getDeviceID(buf, sizeof(buf)));

    if (wifly.isConnected()) {
	println_P(PSTR("Old connection active. Closing"));
	wifly.close();
    }

    if (wifly.open(SERVER_NAME, SERVER_PORT)) {
	print_P(PSTR("Connected to "));
	Serial.println(SERVER_NAME);

	Serial.println("WIFI ALREADY");
    } else {
	println_P(PSTR("Failed to connect"));
	Reset_AVR();
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

// Sending Json formatted data procedure
// Waiting server's response to send next data packet.
// Function Author Mirko Mancini



void InvioWIFIHttp(char* jsonMsgBody, int lungh)
{
    Serial.println("Creating POST HTTP....");
     /* Send the request */
    wifly.println("POST / HTTP/1.0"); // The path here is relative
    wifly.println("Host: ardomino-rshk.rhcloud.com");
    wifly.println("Content-type: application/json");
    wifly.print("Content-Length: ");
    wifly.println(lungh);
    wifly.println("Connection: close");
    wifly.println(""); // End of headers
    wifly.println(jsonMsgBody);


  Serial.println("Waiting server 's response");
  //Waiting server 's response
   // Loop while reading response
  //Waiting server 's response
  while(wifly.available()==0){}

     if(wifly.available() > 0) {
       char buf[200] = "buffer";
       int exit = 0;

       while(exit<2){
	   wifly.gets(buf, sizeof(buf));
	   Serial.println(buf);
	   if(buf[0]==0){
	     exit++;
	   }
	   if(buf[0]=='{'){
	     delay(50);
	     long timeSend = parsingJSONString(buf, sizeof(buf));
	     timeSend *= 1000;
	    // Wating this value to reset microcontroller
	     delay(timeSend);
	   }
       }
    }
}

// function for raw JSON  parsing code Author Mirko Mancini

long parsingJSONString(char buffer[], int len){
    int k;
    unsigned long m;

    for(int i=0; i<len; i++){
      if((buffer[i]=='"')&&(buffer[i+1]=='c')&&(buffer[i+2]=='f')&&(buffer[i+3]=='g')&&(buffer[i+4]=='"')){
	for(k=i+23; buffer[k]!='"'; k++){
	  Serial.print(buffer[k]);
	}
	unsigned long value=0;
	Serial.println();

	for(int l=k-1, m=1; l>i+22; l--){
	  value += m*((int)buffer[l]-48);
	  m *= 10;
	}

	return value;
      }
    }

    return 0;
}
