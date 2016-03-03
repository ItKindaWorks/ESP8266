#!/usr/bin/python
import sys
import os
import paho.mqtt.client as mqtt
import string
import datetime
import time
import logging

#keeps track of when we last turned the light on
onStartTime = 0

##############################

#Create and setup the logging subsystem
logger = None

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

# create a file handler
timeFormat = "%a %b %d %Y %H.%M.%S"
today = datetime.datetime.today()
timestamp = today.strftime(timeFormat)
logFile = 'logs/logs' + timestamp + '.log'
handler = logging.FileHandler(logFile)
handler.setLevel(logging.INFO)

# create a logging format
formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
handler.setFormatter(formatter)

# add the handlers to the logger
logger.addHandler(handler)

##############################



def on_message(mqttc, obj, msg):

    #define our global vars for logger and the start time tracker
    global onStartTime
    global logger

    #get the local time in an easy to read format
    localtime = time.asctime( time.localtime(time.time()) )

    #print the message topic and payload for debugging
    print msg.topic + " - " + msg.payload

    #check to see that the topic is our light1confirm 
    #- not needed in this example because we are only subscribed to 1 topic as it is
    #- but I prefer to play it safe
    if (msg.topic == "/house/light1confirm"):   

        #to do if the message said that we turned the light On
        if(msg.payload == "On"):
            #take note of when we turned the light on
            onStartTime = time.time()

            #log the light on time and print
            logMessage = "Light turned on at: " + localtime
            print logMessage
            logger.info(logMessage)

        #to do if the message said that we turned the light Off
        else:
            #take note of the total run time
            runTime = time.time() - onStartTime

            #log & print when the light turned off
            logMessage = "Light turned off at: " + localtime
            print logMessage
            logger.info(logMessage)

            #log & print the total time the light was on for
            logMessage = "The light was on for a total of " + str(int(runTime)) + " seconds"
            print logMessage
            logger.info(logMessage)





#create our MQTT client
mqttc = mqtt.Client()

#tell it what to do when we recieve a message
mqttc.on_message = on_message

#connect to the broker (most likely it is localhost if running MQTT lotcally)
mqttc.connect("127.0.0.1", 1883, 60)

#subscribe to our light confirmation topic
mqttc.subscribe("/house/light1confirm", 0)

#start the MQTT client loop in a separate thread
mqttc.loop_start()



#just loop a bunch - yeah I know this is not the best way to do things
while(True):
    time.sleep(1)
