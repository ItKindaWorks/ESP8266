//ItKindaWorks - Creative Commons 2016
//github.com/ItKindaWorks
//
//Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//
//ESP8266 Simple Serial breakout test via MQTT - ESP side
//
//This code demonstrates using an ESP8266 serial break board for bi-directional communication.
//It connects to a network and listens subscribes to an MQTT topic. When the ESP recieves a '1' it sends that to the 
//teensy and the teensy turns a light on and pings back on the serial line. When the ESP gets the ping back it publishes 
//a confirmation to the MQTT broker on a different topic


#include <PubSubClient.h>
#include <ESP8266WiFi.h>


//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "YOUR.MQTT.SERVER.IP"
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

//topic to subscribe to for the light
char* lightTopic = "/test/esp1";

//confirmation topic
char* lightConfirm = "/test/esp1Confirm";


WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() {

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

	//reconnect if connection is lost
	if (!client.connected() && WiFi.status() == 3) {reconnect();}

	//maintain MQTT connection
	client.loop();

	//MUST delay to allow ESP8266 WIFI functions to run
	delay(10); 
}


//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {

	//convert topic to string to make it easier to work with
	String topicStr = topic; 

	//turn the light on via serial to another micro if the payload is '1' and wait for ping back from other micro and confirm via confirm topic
	if(payload[0] == '1'){
		Serial.write('1');
		for(int i = 0; i < 10; i++){	//wait up to 100 ms for ping from other micro - loop 10 times each time waiting 10ms
			if(Serial.available() >= 1){
				if(Serial.read() == '1'){
					client.publish(lightConfirm, "Light turned on confirm");
					break;
				}
			}
			delay(10);
		}
	}

	//turn the light off via serial to another micro if the payload is '0' and wait for ping back from other micro and confirm via confirm topic
	else if (payload[0] == '0'){
		Serial.write('0');
		for(int i = 0; i < 10; i++){	//wait up to 100 ms for ping from other micro - loop 10 times each time waiting 10ms
			if(Serial.available() >= 1){
				if(Serial.read() == '0'){
					client.publish(lightConfirm, "Light turned off confirm");
					break;
				}	
			}
			delay(10);
		}
	}

}








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
				client.subscribe(lightTopic);
			}

			//otherwise print failed for debugging
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








