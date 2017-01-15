/*    
    Copyright (c) 2016 ItKindaWorks All right reserved.
    github.com/ItKindaWorks

    This file is part of RelayControl

    RelayControl is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    RelayControl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RelayControl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ESPHelper.h"

#define STATUS "/status"	//dont change this - this is for the status topic which is whatever your mqtt topic is plus status (ex /home/light/status)
#define TOPIC "/your/mqtt/topic"
#define NETWORK_HOSTNAME "YOUR OTA HOST NAME"
#define OTA_PASSWORD "YOUR OTA PASSWORD"
#define RELAY_PIN 3		//rx pin on esp
#define BLINK_PIN 1		//tx/led on esp-01



char* relayTopic = TOPIC;
char* hostnameStr = NETWORK_HOSTNAME;
const int relayPin = RELAY_PIN;
const int blinkPin = BLINK_PIN;		//tx pin on esp

String statusTopic;

//set this info for your own network
netInfo homeNet = {.name = "NETWORK NICKNAME", .mqtt = "YOUR MQTT-IP", .ssid = "YOUR SSID", .pass = "YOUR NETWORK PASS"};

ESPHelper myESP(&homeNet);

void setup() {

	//create the status topic string
	statusTopic = relayTopic;
	statusTopic.concat(STATUS);

	//setup ota
	myESP.OTA_enable();
	myESP.OTA_setPassword(OTA_PASSWORD);
	myESP.OTA_setHostnameWithVersion(hostnameStr);
	

	//setup the rest of ESPHelper
	myESP.enableHeartbeat(blinkPin);	//comment out to disable the heartbeat
	myESP.addSubscription(relayTopic);	//add the relay topic to the subscription list
	myESP.begin();
	myESP.setCallback(callback);
	

	pinMode(relayPin, OUTPUT);
    delay(500);
}


void loop(){
	//loop ESPHelper and wait for commands from mqtt
	myESP.loop();
	yield();
}


//mqtt callback
void callback(char* topic, byte* payload, unsigned int length) {
	String topicStr = topic;

	//convert the status topic String to a char* so the mqtt lib can use it
	char replyTopic[50];
	statusTopic.toCharArray(replyTopic, 50);

	//if the payload from mqtt was 1, turn the relay on and update the status topic with 1
	if(payload[0] == '1'){
		digitalWrite(relayPin, HIGH);
		myESP.publish(replyTopic, "1",true);
	}

	//else turn the relay off and update the status topic with 0
	else if (payload[0] == '0'){
		digitalWrite(relayPin, LOW);
		myESP.client.publish(replyTopic, "0", true);
	}

}
