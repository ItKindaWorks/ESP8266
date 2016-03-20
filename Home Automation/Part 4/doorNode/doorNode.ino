//ItKindaWorks - Creative Commons 2016
//github.com/ItKindaWorks
//
//Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//
//ESP8266 MQTT door sensor node


#include <PubSubClient.h>
#include <ESP8266WiFi.h>


//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "YOUR.MQTT.SERVER.IP"
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

//input pin for the reed switch used to detect the door state
const int doorPin = 2;

//var to keep the door state in so we can compare it to the most recent reading 
//- this is to prevent us from constantly publishing the state but only publish changes
bool isOpen = false;

//topic to publish to for the door
char* doorTopic = "/house/door1";


WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() {
	//initialize the light as an output and set to LOW (off)
	pinMode(doorPin, INPUT);

	//start the serial line for debugging
	Serial.begin(115200);
	delay(100);


	//start wifi subsystem
	WiFi.begin(ssid, password);

	//attempt to connect to the WIFI network and then connect to the MQTT server
	reconnect();

	//wait a bit before starting the main loop
    	delay(2000);
}



void loop(){

	//grab the current door state
	bool doorState = digitalRead(doorPin);		//LOW is closed HIGH is open

	if(!doorState && !isOpen){	//if door is open and the state closed, publish
		client.publish(doorTopic,"0");	//send closed
		isOpen = true;
		delay(500);
	}
	else if(doorState && isOpen){ 	//if door is closed and the state is open, publish
		client.publish(doorTopic,"1");	//send closed
		isOpen = false;
		delay(500);
	}




	//reconnect if connection is lost
	if (!client.connected() && WiFi.status() == 3) {reconnect();}
	//maintain MQTT connection
	client.loop();
	//MUST delay to allow ESP8266 WIFI functions to run
	delay(10); 
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

