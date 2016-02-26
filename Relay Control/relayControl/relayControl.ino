//ItKindaWorks - Creative Commons 2016
//github.com/ItKindaWorks
//
//ESP8266 aRest UI Relay tutorial


#include <ESP8266WiFi.h>
#include <aREST.h>
#include <aREST_UI.h>

// Create aREST instance
aREST_UI myRest = aREST_UI();

// WiFi parameters
const char* ssid = "YOUR_SSID_HERE";
const char* password = "YOUR_PASSWORD_HERE";

// The port to listen for incoming TCP connections 
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);


void setup(void)
{  
  // Start Serial for debugging
  Serial.begin(115200);
  
  // Create a title for our webpage
  myRest.title("Relay Control");

  // Create button to control pin 2
  myRest.button(2);
    
  // Give name and ID to device
  myRest.set_id("1");
  myRest.set_name("esp8266");
  
  // Connect to the network and start the server
  connectWifi();
}

void loop() {
  restLoop();	//automatically handle the aRest functionality
}


boolean restLoop(){
  WiFiClient client = server.available();
  
  if (!client) {	//check to see if the client is connected or not and return if not connected
    return false;
  }

  while(!client.available()){	//wait for input from the client - note that this is a blocking operation
    delay(1);
  }

  myRest.handle(client);	//handle any requests from the client

  return true;	//and return
}

void connectWifi(){
	Serial.print("Connecting to ");
	Serial.println(ssid);

	//Start the wifi subsystem
	WiFi.begin(ssid, password);

	//wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	//print connection IP upon success
	Serial.println("");
	Serial.println("WiFi connected");  
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	// Start the server
	server.begin();
	Serial.println("Server started");
}







