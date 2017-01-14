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
var client = mqtt.connect(options);
client.on('message', function(topic, message) {
  //incoming MQTT parse here
  if(topic == statusTopic){
    if(message != 'p1' || message != 'p0'){   //ignore 'p0' and 'p1' moodlight activate/disable statuses
      var messageParsed = message.toString().split(",");    //split the message into strings
      //console.log("Message = " + messageParsed);

      //parse the current HSB values from the status message
      var hue = Math.round(parseFloat(messageParsed[0].slice(1)) * 360);
      var sat = Math.round(parseFloat(messageParsed[1]) * 100);
      var bri = Math.round(parseFloat(messageParsed[2]) * 100);

      //set the power state of the light based on the brightness being not zero
      if(bri != 0){
        LightController.power = true;
      }
      else{
        LightController.power = false;
      }

      //set the HSB values
      LightController.brightness = bri;
      LightController.hue = hue;
      LightController.saturation = sat;

      //update the IOS device
      LightController.updateIOS();
    }
  }

});


client.subscribe(statusTopic, {qos: 1});


var LightController = {
  name: NAME, //name of accessory
  pincode: "031-45-154",
  username: USERNAME, // MAC like address used by HomeKit to differentiate accessories. 
  manufacturer: "HAP-NodeJS", //manufacturer (optional)
  model: "v1.0", //model (optional)
  serialNumber: SERIAL, //serial number (optional)

  power: false, //curent power status
  brightness: 100, //current brightness
  hue: 0, //current hue
  saturation: 0, //current saturation

  lastSaturation: 0,
  lastHue: 0,
  lastBrightness: 100,

  savedBrightness: 100, //this is what the power switch will return to when flipped on

  outputLogs: false, //output logs

  //set power state of accessory
  setPower: function(status) { 

    if((status == true && this.power == false) || (status == false && this.power == true) ){
      
      //if turned on set the brightness to the last brightness before it was turned off 
      if(status == true){
        this.brightness = this.savedBrightness;
        this.updateLight();
        this.power = true;
      }

      //if turned off set the brightness to 0 and update the light
      else{
        this.brightness = 0;
        this.updateLight();
        this.power = false;
      }
    }

  },

  //get power of accessory
  getPower: function() { 
    if(this.outputLogs) console.log("'%s' is in %s mode.", this.name, this.power ? "mood" : "set");
    return this.power;
  },



  //RGB ONLY

  //set the brightness of the accessory
  setBrightness: function(brightness) { //set brightness
    if(this.outputLogs) console.log("Setting '%s' brightness to %s", this.name, brightness);

    this.brightness = brightness;
    this.savedBrightness = brightness;

    //if the brightness is above 0 then set the power state to on otherwise set it to off
    if(brightness > 0){this.power = true;}
    else{this.power = false;}

    this.updateLight();
  },

  //get the current brightness of the light
  getBrightness: function() { //get brightness
    if(this.outputLogs) console.log("'%s' brightness is %s", this.name, this.brightness);
    return this.brightness;
  },

  //set the saturation value of the light
  setSaturation: function(saturation) {
    if(this.outputLogs) console.log("Setting '%s' saturation to %s", this.name, saturation);
    this.saturation = saturation;
    this.updateLight();
  },

  //get the saturation of the light
  getSaturation: function() { 
    if(this.outputLogs) console.log("'%s' saturation is %s", this.name, this.saturation);
    return this.saturation;
  },

  //set the hue of the light
  setHue: function(hue) { 
    if(this.outputLogs) console.log("Setting '%s' hue to %s", this.name, hue);
    this.hue = hue;
    this.updateLight();
  },

  //get the hue of the light
  getHue: function() { 
    if(this.outputLogs) console.log("'%s' hue is %s", this.name, this.hue);
    return this.hue;
  },

  //uodate the values on the IOS device all at once
  updateIOS: function(){
    lightAccessory
      .getService(Service.Lightbulb)
      .getCharacteristic(Characteristic.On)
      .updateValue(this.power);

    lightAccessory
      .getService(Service.Lightbulb)
      .getCharacteristic(Characteristic.Brightness)
      .updateValue(this.brightness);

    lightAccessory
      .getService(Service.Lightbulb)
      .getCharacteristic(Characteristic.Hue)
      .updateValue(this.hue);

    lightAccessory
      .getService(Service.Lightbulb)
      .getCharacteristic(Characteristic.Saturation)
      .updateValue(this.saturation);
  },


  identify: function() { //identify the accessory
    if(this.outputLogs) console.log("Identify the '%s'", this.name);
  },

  //Sends an mqtt update to the light if needed
  updateLight: function(){
    if(this.lastSaturation != this.saturation || this.lastHue != this.hue || this.lastBrightness != this.brightness){

      pubBrightness = this.brightness / 100;
      pubHue = this.hue / 360;
      pubSaturation = this.saturation / 100;

      toPublish = 'h' + pubHue.toFixed(3).toString() + ',' + pubSaturation.toFixed(3).toString() + ',' + pubBrightness.toFixed(3).toString();

      client.publish(lightTopic, toPublish);

      this.lastBrightness = this.brightness;
      this.lastHue = this.hue;
      this.lastSaturation = this.saturation;
    }
  }
}

// Generate a consistent UUID for our light Accessory that will remain the same even when
// restarting our server. We use the `uuid.generate` helper function to create a deterministic
// UUID based on an arbitrary "namespace" and the word "light".
var lightUUID = uuid.generate('hap-nodejs:accessories:light' + LightController.name);

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






/***
  RGB ONLY
*/


// also add an "optional" Characteristic for Brightness
lightAccessory
  .getService(Service.Lightbulb)
  .addCharacteristic(Characteristic.Brightness)
  .on('set', function(value, callback) {
    LightController.setBrightness(value);
    callback();
  })
  .on('get', function(callback) {
    callback(null, LightController.getBrightness());
  });

// also add an "optional" Characteristic for Saturation
lightAccessory
  .getService(Service.Lightbulb)
  .addCharacteristic(Characteristic.Saturation)
  .on('set', function(value, callback) {
    LightController.setSaturation(value);
    callback();
  })
  .on('get', function(callback) {
    callback(null, LightController.getSaturation());
  });

// also add an "optional" Characteristic for Hue
lightAccessory
  .getService(Service.Lightbulb)
  .addCharacteristic(Characteristic.Hue)
  .on('set', function(value, callback) {
    LightController.setHue(value);
    callback();
  })
  .on('get', function(callback) {
    callback(null, LightController.getHue());
  });