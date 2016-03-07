//ItKindaWorks - Creative Commons 2016
//github.com/ItKindaWorks
//
//Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//
//ESP8266 Simple Serial breakout test via MQTT - teensy/arduino side
//
//This code demonstrates using an ESP8266 serial break board for bi-directional communication.
//It connects to a network and listens subscribes to an MQTT topic. When the ESP recieves a '1' it sends that to the 
//teensy and the teensy turns a light on and pings back on the serial line. When the ESP gets the ping back it publishes 
//a confirmation to the MQTT broker on a different topic


const int lightPin = 11;

void setup() {
	//initialize the light as an output and set to LOW (off)
	pinMode(lightPin, OUTPUT);
	digitalWrite(lightPin, LOW);

	//start the serial line for debugging
	Serial1.begin(115200);
	delay(100);

}



void loop(){

	if(Serial1.available()){
		char inByte = Serial1.read();
		if(inByte == '1'){
			digitalWrite(lightPin, HIGH);
			Serial1.write('1');
		}
		else if(inByte == '0'){
			digitalWrite(lightPin, LOW);
			Serial1.write('0');
		}
	}
}








