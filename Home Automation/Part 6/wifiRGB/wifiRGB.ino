//ItKindaWorks - Creative Commons 2016
//github.com/ItKindaWorks
//
//Requires PubSubClient found here: https://github.com/knolleary/pubsubclient
//Requires HSBColor Library found here: https://github.com/julioterra/HSB_Color/blob/master/HSBColor.h
//
//ESP8266 Simple MQTT RGB light controller
//
//MQTT commands should look like this:
//
//HSB Update: hX.XXX,Y.YYY,Z.ZZZ
//RGB Update: rRRR,GGG.BBB
//Power (mode control) Update: p1 or p0



#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <HSBColor.h>
#include "myTypes.h"

//EDIT THESE LINES TO MATCH YOUR SETUP
#define MQTT_SERVER "YOUR.MQTT.SERVER.IP"
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";



const int redPin = 14;
const int greenPin = 16;
const int bluePin = 12;


enum mainModes {SET, MOOD};	//SET for siri/network control - MOOD for auto color rotation
enum moodColors{RED, GREEN, BLUE};	//colors to rotate through for mood light
enum LEDmodes {NORMAL, FADING};		//fading modes either fading or none/normal
enum updateTypes{HSB, RGB, POWER};	//different kinds of network commands

lightState nextState;	//the next state (usually updated from MQTT)

int mainMode = SET;	//overall mode of the light (moodlight, network controlled, etc)
boolean newCommand = false;	//informs the LED task of a new command waiting

unsigned long currentMillis = 0;	//current time in milliseconds for multitasking purposes

char* lightTopic = "/house/RGBlight1";	//topic to subscribe to for the light

int timeout = 0; //timeout timer to fail to standard moodlight when no known wifi is found within 10 seconds 


WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() {
	//initialize the light as an output and set to LOW (off)
	pinMode(redPin, OUTPUT);
	pinMode(bluePin, OUTPUT);
	pinMode(greenPin, OUTPUT);
	digitalWrite(redPin, LOW);	//all off
	digitalWrite(greenPin, LOW);
	digitalWrite(bluePin, LOW);

	delay(1000);

	colorTest();



	Serial.begin(115200);


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
	
	//increment wifi timeout whenever there is no wifi and after 10 seconds (2ms * 5000 loops) fail to moodlight
	if(WiFi.status() != WL_CONNECTED){timeout++;}
	if(timeout == 5000){
		mainMode = MOOD;
		nextState.fadePeriod = 5000;
		newCommand = true;
		timeout = 0;
	}

	//maintain MQTT connection
	client.loop();

	currentMillis = millis();

	lightHandler();

	//MUST delay to allow ESP8266 WIFI functions to run
	delay(2); 
}









void lightHandler(){
	static lightState newState;		//lightState to hold whatever new light state we may recieve (from MQTT or moodlight changer)
	static lightState currentState;		//the current state of the LEDs
	static int currentMoodColor = 0;	//if using the moodlight, this keeps track of which color we are current on (referenced from the moodColors enum at the top)

	static boolean isFading = false;	//keeps track of whether the lights are currently fading to a new color or not


	if(mainMode == MOOD && !isFading){	//change the moodlight color if the mode is current set to MOOD and the lights are done fading from the last color

		if(currentMoodColor == RED){		//change the next state information to go from red to green
			nextState.red = 0;
			nextState.green = 1023;
			nextState.blue = 0;
			nextState.updateType = RGB;
			newCommand = true;
			currentMoodColor = GREEN;

		}
		else if(currentMoodColor == GREEN){		//change the next state information to go from green to blue
			nextState.red = 0;
			nextState.green = 0;
			nextState.blue = 1023;
			nextState.updateType = RGB;
			newCommand = true;
			currentMoodColor = BLUE;
		}
		else if(currentMoodColor == BLUE){	//change the next state information to go from blue to green
			nextState.red = 1023;
			nextState.green = 0;
			nextState.blue = 0;
			nextState.updateType = RGB;
			newCommand = true;
			currentMoodColor = RED;
		}
	}

	lightUpdater(&newState, currentState);	//do new calculations for the updated light state (if there are any)

	isFading = lightChanger(newState, &currentState);	//change the lights 

}


//Change the current light values and keep track of how much time has been spent fading
boolean lightChanger(lightState newState, lightState *currentState){
	static timer changeTimer;	//timer for handling light changes
	changeTimer.interval = 2;		//light changes should happen once every 2ms 

	static int changeMode = NORMAL;	//initialize changeMode to normal (not fading)
	static int currentPeriod = 0;		//keep track of the total time the lights have been fading

	if(checkTimer(&changeTimer.previousTime, changeTimer.interval)){	//check to see whether it's time to do something
		
		if(newCommand){	//look for a new command and if so change the mode to FADING
			newCommand = false;
			changeMode = FADING;
		}


		if(changeMode == FADING){	//change the lights if we are in FADING mode

			//run when the current color states dont match the new color states or that the current fade period is not over
			if((newState.red != currentState->red || newState.blue != currentState->blue || newState.green != currentState->green) || (currentPeriod <= currentState->fadePeriod)){

				if(currentPeriod % newState.redRate == 0){	//update red
					if(newState.red > currentState->red){currentState->red++;}		//fade up
					else if (newState.red < currentState->red){currentState->red--;}	//fade down
				}

				if(currentPeriod % newState.greenRate == 0){	//update green
					if(newState.green > currentState->green){currentState->green++;}		//fade up
					else if (newState.green < currentState->green){currentState->green--;}	//fade down
				}

				if(currentPeriod % newState.blueRate == 0){		//update blue
					if(newState.blue > currentState->blue){currentState->blue++;}	//fade up
					else if (newState.blue < currentState->blue){currentState->blue--;}	//fade down
				}

				//Write the new fade values to the lights
				analogWrite(redPin, currentState->red);
				analogWrite(greenPin, currentState->green);
				analogWrite(bluePin, currentState->blue);

				//increment the current period
				currentPeriod++;
				return true;	//true on fading
			}
			else{
				currentPeriod = 0;	//reset the current period
				changeMode = NORMAL;	//set mode back to normal
				return false;	//fase on not fading
			}

		}


		else if (changeMode == NORMAL){	//if mode is normal already - return false, not fading
			return false;
		}
	

	}

}

//do new calculations for lights
void lightUpdater (lightState *newState, lightState currentState){

	if (newCommand){	//check for new LED command

		if(nextState.updateType == HSB){	//calculate for HSB command

			//convert HSB to RGB
			int newRGB[3];
			H2R_HSBtoRGBfloat(nextState.hue, nextState.saturation, nextState.brightness, newRGB);	
			newState->red = newRGB[0];
			newState->green = newRGB[1];
			newState->blue = newRGB[2];

			//calculcate the difference in current LED brightnesses to the new brightnesses
			int redDiff = abs(newState->red - currentState.red);
			int greenDiff = abs(newState->green - currentState.green);
			int blueDiff = abs(newState->blue - currentState.blue);


			//calculate how long each light will take to fade to the new value
			if(redDiff > 0){newState->redRate = (nextState.fadePeriod / redDiff) + 1;}		//dont allow dividing by 0
			else{newState->redRate = nextState.fadePeriod;}

			if(greenDiff > 0){newState->greenRate = (nextState.fadePeriod / greenDiff) + 1;}	//dont allow dividing by 0
			else{newState->greenRate = nextState.fadePeriod;}

			if(blueDiff > 0){newState->blueRate = (nextState.fadePeriod / blueDiff) + 1;}	//dont allow dividing by 0
			else{newState->blueRate = nextState.fadePeriod;}

			newState->fadePeriod = nextState.fadePeriod;	//set the new fade period

		}

		else if(nextState.updateType == RGB){	//calculate for RGB command

			//grab the new RGB values
			newState->red = nextState.red;
			newState->green = nextState.green;
			newState->blue = nextState.blue;

			//calculcate the difference in current LED brightnesses to the new brightnesses
			int redDiff = abs(newState->red - currentState.red);
			int greenDiff = abs(newState->green - currentState.green);
			int blueDiff = abs(newState->blue - currentState.blue);

			//calculate how long each light will take to fade to the new value
			if(redDiff > 0){newState->redRate = (nextState.fadePeriod / redDiff) + 1;}	//dont allow dividing by 0
			else{newState->redRate = nextState.fadePeriod;}

			if(greenDiff > 0){newState->greenRate = (nextState.fadePeriod / greenDiff) + 1;}	//dont allow dividing by 0
			else{newState->greenRate = nextState.fadePeriod;}

			if(blueDiff > 0){newState->blueRate = (nextState.fadePeriod / blueDiff) + 1;}	 //dont allow dividing by 0
			else{newState->blueRate = nextState.fadePeriod;}

			newState->fadePeriod = nextState.fadePeriod;	//set the new fade period

		}
	}

}





void colorTest(){
	digitalWrite(redPin, HIGH);	//red on
	delay(500);
	digitalWrite(redPin, LOW);	//green on
	digitalWrite(greenPin, HIGH);
	delay(500);
	digitalWrite(greenPin, LOW);	//blue on
	digitalWrite(bluePin, HIGH);
	delay(500);

	digitalWrite(redPin, HIGH);	//all on
	digitalWrite(greenPin, HIGH);
	digitalWrite(bluePin, HIGH);
	delay(500);
	digitalWrite(redPin, LOW);	//all off
	digitalWrite(greenPin, LOW);
	digitalWrite(bluePin, LOW);
}


char *ftoa(char *a, double f, int precision){
	long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};

	char *ret = a;
	long heiltal = (long)f;
	itoa(heiltal, a, 10);
	while (*a != '\0') a++;
	*a++ = '.';
	long desimal = abs((long)((f - heiltal) * p[precision]));
	itoa(desimal, a, 10);
	return ret;
}






//networking functions

//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {

	//create a new version of the payload that is null terminated so atoi and atof dont break
	char newPayload[50];
	memcpy(newPayload, payload, length);
	newPayload[length] = '\0';


	//new HSB command
	if(payload[0] == 'h'){
		//grab the HSB values from the string
		nextState.hue = atof(&newPayload[1]);
		nextState.saturation = atof(&newPayload[7]);
		nextState.brightness = atof(&newPayload[13]);


		nextState.updateType = HSB;		//set the fade type of HSB
		nextState.fadePeriod = 1050;		//fade period of 1050
		newCommand = true;			//tell the lighthandler that there is a new command waiting
		mainMode = SET;			//make sure the mode is set to SET and not MOOD

	}

	//new RGB command
	else if (payload[0] == 'r'){
		//grab the RGB values from the string
		int newRed = atoi(&newPayload[1]);
		int newGreen = atoi(&newPayload[5]);
		int newBlue = atoi(&newPayload[9]);

		nextState.red = newRed;
		nextState.green = newGreen;
		nextState.blue = newBlue;

		nextState.updateType = RGB;	//set the fade type of RGB
		nextState.fadePeriod = 1050;		//fade period of 1050
		newCommand = true;			//tell the lighthandler that there is a new command waiting
		mainMode = SET;			//make sure the mode is set to SET and not MOOD
	}

	//new "power" command - moodlight vs set changer. 
	//Apple homekit doesnt have any other light setting so in this case the power switch in homekit changes the 
	//light from normal to moodlight
	else if(payload[0] == 'p'){

		if(payload[1] == '1'){		//power is set to MOOD
			mainMode = MOOD;
			nextState.fadePeriod = 5000;
			newCommand = true;
		}
		else if(payload[1] == '0'){	//power is set to SET
			mainMode = SET;
			nextState.fadePeriod = 1050;
			newCommand = true;
		}
	}

}

void reconnect() {
	Serial.println("Attempting Reconnect");

	//attempt to connect to the wifi if connection is lost
	if(WiFi.status() != WL_CONNECTED){
		
		while (WiFi.status() != WL_CONNECTED) {
			delay(500);
			Serial.print(".");
		}
	}

	//make sure we are connected to WIFI before attemping to reconnect to MQTT
	if(WiFi.status() == WL_CONNECTED){
		Serial.println("---WIFI Connected!---");

		while (!client.connected()) {
			Serial.print("Attemping MQTT connection");

			// Generate client name based on MAC address and last 8 bits of microsecond counter
			String clientName;
			clientName += "esp8266-";
			uint8_t mac[6];
			WiFi.macAddress(mac);
			clientName += macToStr(mac);

			//if connected, subscribe to the topic(s) we want to be notified about
			if (client.connect((char*) clientName.c_str())) {
				Serial.println(" -- Connected");
				client.subscribe(lightTopic);
			}
			else{Serial.println(" -- Failed");}

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

//this functions simplifies checking whether a task needs to run by checking our timing variables 
//and returning true or false depending on whether we need to run or not
boolean checkTimer(unsigned long *previousTime, unsigned long interval){
  if(currentMillis - *previousTime >= interval){
    *previousTime = currentMillis;
    return true;
  }
  return false;
}






