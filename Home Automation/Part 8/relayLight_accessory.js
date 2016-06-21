var Accessory = require('../').Accessory;
var Service = require('../').Service;
var Characteristic = require('../').Characteristic;
var uuid = require('../').uuid;

////////////////CHANGE THESE SETTINGS TO MATCH YOUR SETUP BEFORE RUNNING!!!!!!!!!!!!!//////////////////////////
////////////////CHANGE THESE SETTINGS TO MATCH YOUR SETUP BEFORE RUNNING!!!!!!!!!!!!!//////////////////////////
var name = "Relay Light";
var UUID = "hap-nodejs:accessories:Relay1";
var USERNAME = "A2:AF:1A:1C:AD:CB";

var MQTT_IP = 'YOUR.MQTT.IP.ADDRESS'
var lightTopic = '/YOUR/LIGHT/TOPIC'
////////////////CHANGE THESE SETTINGS TO MATCH YOUR SETUP BEFORE RUNNING!!!!!!!!!!!!!//////////////////////////
////////////////CHANGE THESE SETTINGS TO MATCH YOUR SETUP BEFORE RUNNING!!!!!!!!!!!!!//////////////////////////

// MQTT Setup
var mqtt = require('mqtt');
var options = {
  port: 1883,
  host: MQTT_IP,
  clientId: 'cdfadfrAK343'
};
var client = mqtt.connect(options);
client.on('message', function(topic, message) {
  
});

//setup HK light object
var lightUUID = uuid.generate(UUID);
var light = exports.accessory = new Accessory(name, lightUUID);

// Add properties for publishing (in case we're using Core.js and not BridgedCore.js)
light.username = USERNAME;
light.pincode = "031-45-154";

//add a light service and setup the On Characteristic
light
  .addService(Service.Lightbulb)
  .getCharacteristic(Characteristic.On)
  .on('get', function(callback) {
    callback(null, lightAction.getState());
  });

  light
  .getService(Service.Lightbulb)
  .getCharacteristic(Characteristic.On)
  .on('set', function(value, callback) {
    lightAction.setState(value);
    callback();
  });



var lightAction = {

  //initialize the various state variables
  currentState: 0,

  //On Characteristic set/get
  getState: function() { return this.currentState;},
  setState: function(newState){

    if(newState != this.currentState ){
      console.log("Setting new outlet state: " + newState.toString());
      if(newState == true){
        client.publish(lightTopic, '1');
        this.currentState = 1;
      }
      else{
        client.publish(lightTopic, '0');
        this.currentState = 0;
      }
    }

  }  
}

// update the characteristic values so interested iOS devices can get notified
setInterval(function() {
  light
    .getService(Service.Lightbulb)
    .setCharacteristic(Characteristic.On, lightAction.currentState);

}, 2000);
