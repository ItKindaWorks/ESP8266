var Accessory = require('../').Accessory;
var Service = require('../').Service;
var Characteristic = require('../').Characteristic;
var uuid = require('../').uuid;


////////////////   CHANGE THESE VALUES FOR EVERY ACCESSORY   !!!!!!!!!!!!!//////////////////////////
////////////////   CHANGE THESE VALUES FOR EVERY ACCESSORY   !!!!!!!!!!!!!//////////////////////////
////////////////   CHANGE THESE VALUES FOR EVERY ACCESSORY   !!!!!!!!!!!!!//////////////////////////

//These 3 values MUST be unique for every accessory you make. If they are not then IOS may have issues and mess
//the entire homekit setup and you will have to reset homekit on IOS.
var NAME = "YOUR ACCESSORY NAME";     //give you accessory a name!
var USERNAME = "11:22:33:44:55:66";   //this is like a mac address for the accessory
var SERIAL = '123456789abc'           //unique serial address for the accessory

var MQTT_ID = 'homekit' + SERIAL
var MQTT_IP = 'YOUR.MQTT.IP.ADDRESS'

var relayTopic = '/your/mqtt/topic'       //this will be the topic that you publish to, to update the accessory
var statusTopic = relayTopic + "/status"; //this will the topic that this script subscribes to in order to get updates on the current status of the accessory

////////////////   CHANGE THESE VALUES FOR EVERY ACCESSORY   !!!!!!!!!!!!!//////////////////////////
////////////////   CHANGE THESE VALUES FOR EVERY ACCESSORY   !!!!!!!!!!!!!//////////////////////////
////////////////   CHANGE THESE VALUES FOR EVERY ACCESSORY   !!!!!!!!!!!!!//////////////////////////



// MQTT Setup
var mqtt = require('mqtt');
var options = {
  port: 1883,
  host: MQTT_IP,
  clientId: MQTT_ID
};

//connect to MQTT
var client = mqtt.connect(options);

//on new message from the status topic take action if needed on IOS
client.on('message', function(topic, message) {

  if(topic == statusTopic){

    if(parseInt(message) == 1 && LightController.power != true){
      //console.log("recieved new status on");
      LightController.setPower(true);
    }

    else if(parseInt(message) == 0 && LightController.power != false){
      //console.log("recieved new status off");
      LightController.setPower(false);
    }

  }

});

//subscribe to the status topic
client.subscribe(statusTopic, {qos: 1});




var LightController = {
  name: NAME, //name of accessory
  pincode: "031-45-154",
  username: USERNAME, // MAC like address used by HomeKit to differentiate accessories. 
  manufacturer: "HAP-NodeJS", //manufacturer (optional)
  model: "v1.0", //model (optional)
  serialNumber: SERIAL, //serial number (optional)

  power: false, //curent power status

  outputLogs: false, //output logs

  setPower: function(status) { //set power of accessory

    //only publish a new state if the new state and current state are different
    if((status == true && this.power == false) || (status == false && this.power == true) ){

      //console.log("Setting new outlet state: " + status.toString());
      if(status == true){
        client.publish(relayTopic, '1');
        this.power = true;
      }
      else{
        client.publish(relayTopic, '0');
        this.power = false;
      }

      this.updateIOS();
    }


    if(this.outputLogs) console.log("Turning the '%s' %s", this.name, status ? "on" : "off");
    this.power = status;
  },

  //get power of accessory
  getPower: function() { 
    if(this.outputLogs) console.log("'%s' is %s.", this.name, this.power ? "on" : "off");
    return this.power ? true : false;
  },

  //update the IOS device with the current state of the accessory
  updateIOS: function(){
    lightAccessory
      .getService(Service.Lightbulb)
      .getCharacteristic(Characteristic.On)
      .updateValue(this.power);
  },

  identify: function() { //identify the accessory
    if(this.outputLogs) console.log("Identify the '%s'", this.name);
  }
}

// Generate a consistent UUID for our light Accessory that will remain the same even when
// restarting our server. We use the `uuid.generate` helper function to create a deterministic
// UUID based on an arbitrary "namespace" and the word "light".
var lightUUID = uuid.generate('hap-nodejs:accessories:light' + LightController.serialNumber);

// This is the Accessory that we'll return to HAP-NodeJS that represents our light.
var lightAccessory = exports.accessory = new Accessory(LightController.name, lightUUID);

// Add properties for publishing (in case we're using Core.js and not BridgedCore.js)
lightAccessory.username = LightController.username;
lightAccessory.pincode = LightController.pincode;

// set some basic properties (these values are arbitrary and setting them is optional)
lightAccessory
  .getService(Service.AccessoryInformation)
    .setCharacteristic(Characteristic.Manufacturer, LightController.manufacturer)
    .setCharacteristic(Characteristic.Model, LightController.model)
    .setCharacteristic(Characteristic.SerialNumber, LightController.serialNumber);

// listen for the "identify" event for this Accessory
lightAccessory.on('identify', function(paired, callback) {
  LightController.identify();
  callback();
});

// Add the actual Lightbulb Service and listen for change events from iOS.
// We can see the complete list of Services and Characteristics in `lib/gen/HomeKitTypes.js`
lightAccessory
  .addService(Service.Lightbulb, LightController.name) // services exposed to the user should have "names" like "Light" for this case
  .getCharacteristic(Characteristic.On)
  .on('set', function(value, callback) {
    LightController.setPower(value);

    // Our light is synchronous - this value has been successfully set
    // Invoke the callback when you finished processing the request
    // If it's going to take more than 1s to finish the request, try to invoke the callback
    // after getting the request instead of after finishing it. This avoids blocking other
    // requests from HomeKit.
    callback();
  })
  
  // We want to intercept requests for our current power state so we can query the hardware itself instead of
  // allowing HAP-NodeJS to return the cached Characteristic.value.
  .on('get', function(callback) {
    callback(null, LightController.getPower());
  });
