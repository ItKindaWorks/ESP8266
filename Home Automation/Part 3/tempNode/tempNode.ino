//ItKindaWorks - Creative Commons 2016
//github.com/ItKindaWorks
//
//Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//
//ESP8266 MQTT temp sensor node


#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//create 1-wire connection on pin 2 and connect it to the dallasTemp library
OneWire oneWire(2);
DallasTemperature sensors(&oneWire);


//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "YOUR.MQTT.SERVER.IP"
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";


//topic to publish to for the temperature
char* tempTopic = "/house/temp1";
char currentTemp[2];


WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() {

	//null terminate the temp string to be published
	currentTemp[1] = '\0';

	//start the serial line for debugging
	Serial.begin(115200);
	delay(100);


	//start wifi subsystem
	WiFi.begin(ssid, password);

	//attempt to connect to the WIFI network and then connect to the MQTT server
	reconnect();

	//start the temperature sensors
	sensors.begin();

	//wait a bit before starting the main loop
    	delay(2000);
}



void loop(){

	// Send the command to update temperatures
	sensors.requestTemperatures(); 

	//get the new temperature
	float currentTempFloat = sensors.getTempCByIndex(0);
	currentTemp[0] = currentTempFloat;

	//publish the new temperature
	client.publish(tempTopic, currentTemp);




	//reconnect if connection is lost
	if (!client.connected() && WiFi.status() == 3) {reconnect();}
	//maintain MQTT connection
	client.loop();
	//MUST delay to allow ESP8266 WIFI functions to run
	delay(5000); 
}


//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {}








//networking functions

void reconnect() {

	//attempt to connect to the wifi if connection is lost
	if(WiFi.status() != WL_CONNECTED){

		//loop while we wait for connection
		while (WiFi.status() != WL_CONNECTED) {
			delay(500);
		}

	}

	//make sure we are connected to WIFI before attemping to reconnect to MQTT
	if(WiFi.status() == WL_CONNECTED){
	// Loop until we're reconnected to the MQTT server
		while (!client.connected()) {

			// Generate client name based on MAC address and last 8 bits of microsecond counter
			String clientName;
			clientName += "esp8266-";
			uint8_t mac[6];
			WiFi.macAddress(mac);
			clientName += macToStr(mac);

			//if connected, subscribe to the topic(s) we want to be notified about
			if (client.connect((char*) clientName.c_str())) {
				//subscribe to topics here
			}
		}
	}
}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}


