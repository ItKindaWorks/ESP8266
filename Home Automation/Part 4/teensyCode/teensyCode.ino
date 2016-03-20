//ItKindaWorks - Creative Commons 2016
//github.com/ItKindaWorks
//
//Teensy monitor node

//change these to match what you want for temperature range
//note that there is no LOW_TEMP because anything under MID_TEMP 
//is to be considered LOW_TEMP
#define HIGH_TEMP 72
#define MID_TEMP 70


//LED pins for temperature monitor
const int tempLowPin = 14;
const int tempMidPin = 13;
const int tempHighPin = 12;

//LED pin for door monitor
const int doorPin = 10;

//LED pin for the light monitor
const int lightPin = 9;


void setup(){
	//setup pins as outputs
	pinMode(tempHighPin, OUTPUT);
	pinMode(tempMidPin, OUTPUT);
	pinMode(tempLowPin, OUTPUT);
	pinMode(doorPin, OUTPUT);
	pinMode(lightPin, OUTPUT);

	//begin the serial lines
	Serial1.begin(115200);		//com to ESP
	Serial.begin(115200);		//com to computer

	delay(100);
}

void loop(){

	//if we have data & the next byte isn't one of our header bytes, read until we get to a good header value
	//-we can assume that if its not a valid header byte that the data is garbage
	while(Serial1.available() && (Serial1.peek() != 'd' && Serial1.peek() != 't' && Serial1.peek() != 'l')){	
		Serial1.read();
	}

	//our "packets" are 2 bytes in size (1 header + 1 payload) so if we have 2 byte we have a potential valid packet
	if(Serial1.available() >= 2){
		char headerByte = Serial1.read();	//read the header byte
		char payloadByte = Serial1.read();	//read the payload byte
		Serial.println(headerByte);
		
		//handle door state packet
		if(headerByte == 'd'){
			if(payloadByte == 1){
				digitalWrite(doorPin, HIGH);
			}
			else if(payloadByte == 0){
				digitalWrite(doorPin, LOW);
			}
		}

		//handle temperature packet
		else if(headerByte == 't'){

			//high temp - red light on
			if(payloadByte > HIGH_TEMP){
				digitalWrite(tempHighPin, HIGH);
				digitalWrite(tempMidPin, LOW);
				digitalWrite(tempLowPin, LOW);
			}

			//mid temp - green light on
			else if(payloadByte >= MID_TEMP){
				digitalWrite(tempHighPin, LOW);
				digitalWrite(tempMidPin, HIGH);
				digitalWrite(tempLowPin, LOW);
			}

			//low temp - blue light on
			else{
				digitalWrite(tempHighPin, LOW);
				digitalWrite(tempMidPin, LOW);
				digitalWrite(tempLowPin, HIGH);
			}
		}

		//handle light state packet
		else if(headerByte == 'l'){
			if(payloadByte == 1){
				digitalWrite(lightPin, HIGH);
			}
			else if(payloadByte == 0){
				digitalWrite(lightPin, LOW);
			}
		}
	}
	
}