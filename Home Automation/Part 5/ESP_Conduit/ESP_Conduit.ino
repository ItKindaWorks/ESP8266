//ItKindaWorks - Creative Commons 2016
//github.com/ItKindaWorks
//
//Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//
//ESP8266 MQTT serial conduit


#include <PubSubClient.h>
#include <ESP8266WiFi.h>


//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "YOUR.MQTT.SERVER.IP"
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";


//topic to subscribe to for the door
char* doorTopic = "/house/door1";

//topic to subscribe to for the temperature
char* tempTopic = "/house/temp1";

//topic to subscribe to for the light
char* lightTopic = "/house/light1confirm";

//topic to subscribe to for the security system enabled state
char* secTopic = "/house/secStatus";



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

	//this is a fix because the payload passed to the callback isn't null terminated 
	//so we create a new string with a null termination and then turn that into a string
	char payloadFixed[100];
	memcpy(payloadFixed, payload, length);
	payloadFixed[length] = '\0';
	String payloadStr = (char*)payloadFixed;

	char byteToSend = 0;

	//handle doorTopic updates
	if(topicStr.equals(doorTopic)){

		if(payload[0] == '1'){
			byteToSend = 1;
		}

		else if (payload[0] == '0'){
			byteToSend = 0;
		}

		Serial.write("d");		//send a unique header - d
		Serial.write(byteToSend);	//send the current state

	}

	//handle tempTopic updates
	else if(topicStr.equals(tempTopic)){

		byteToSend = (char)payloadStr.toInt();	//convert payload String into an int (cast as char)  to be sent to the Teensy via serial

		Serial.write("t");		//send a unique header - t
		Serial.write(byteToSend);	//send the current temp
	}

	//handle lightTopic updates
	else if(topicStr.equals(lightTopic)){

		if(payload[0] == '1'){
			byteToSend = 1;
		}

		else if(payload[0] == '0'){
			byteToSend = 0;
		}

		Serial.write("l");		//send a unique header - l
		Serial.write(byteToSend);	//send the current state
	}

	else if(topicStr.equals(secTopic)){
		if(payloadStr.equals("disarmed")){
			byteToSend = 0;
		}
		else if(payloadStr.equals("armed_away")){
			byteToSend = 1;
		}
		else if(payloadStr.equals("triggered")){
			byteToSend = 2;
		}
		
		Serial.write("s");
		Serial.write(byteToSend);
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
				client.subscribe(doorTopic);
				client.subscribe(tempTopic);
				client.subscribe(lightTopic);
				client.subscribe(secTopic);
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








