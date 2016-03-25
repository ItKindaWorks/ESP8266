#!/usr/bin/python

import sys
import os
import paho.mqtt.client as mqtt
import smtplib
import string

#var to track the current status of the system (armed or disarmed)
securityEnStatus = None

#enable/disable email notifications - disable when testing/enable when in normal use
sendEmail = False

#This is like the confirm topic for the security system - it notifies other nodes about the status of the system
statusTopic = "/house/secStatus"

#this is the control topic that this script listens to for messages telling it to arm or disarm
enableTopic = "/house/secEn"



def on_connect(mqttc, obj, flags, rc):
    print("rc: "+str(rc))

def on_message(mqttc, obj, msg):
    global securityEnStatus
    global breachesSinceReset

    #if the topic is the enable topic and its to enable the system (1)
    #publish to the state topic that the system is armed and enable status to on
    if(str(msg.topic) == enableTopic and int(msg.payload) == 1):
        securityEnStatus = True
        mqttc.publish(statusTopic, "armed_away")
        print("Security Enabled")


    #if the topic is the enable topic and its to disable the system (0)
    #then set the enable status to off 
    #and publish to the state topic that were disabled and publish the number of breaches
    elif(str(msg.topic) == enableTopic and int(msg.payload) == 0):
        securityEnStatus = False
        mqttc.publish(statusTopic, "disarmed")
        print("Security Disabled")


    #all other topics are detector nodes such as motion detectors and door sensors
    #so as long as the security system is enabled (armed), if one of the detector nodes is triggered
    #send out an alert
    elif(securityEnStatus == True and int(msg.payload) == 1):
        mqttc.publish(statusTopic, "triggered")
        securityAlert(str(msg.topic))


def on_publish(mqttc, obj, mid):
    print("mid: "+str(mid))
def on_subscribe(mqttc, obj, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))
def on_log(mqttc, obj, level, string):
    print(string)




def securityAlert(alertLoc):
    global sendEmail


    print("Breach at: " + alertLoc)
    print("Sending Alert!\n")


    if(sendEmail == True):

        #to and from addresses
        fromaddr = '-----YOUR EMAIL ADDRESS-----'
        toaddrs  = '-----EMAIL ADDRESS TO ALERT-----'

        #form the email header+body
        header = 'To:' + toaddrs + '\n' + 'From: ' + fromaddr + '\n' + 'Subject:Home Security Breach! \n'
        msg = header + "\nBreach Location: " + alertLoc + "\n\n"


        #login credentials
        username = '-----YOUR EMAIL ADDRESS USERNAME-----'
        password = '-----YOUR EMAIL ADDRESS PASSWORD-----'

        # send the email
        server = smtplib.SMTP('smtp.gmail.com:587')
        server.ehlo()
        server.starttls()
        server.ehlo()
        server.login(username,password)
        server.sendmail(fromaddr, toaddrs, msg)
        server.quit()




mqttc = mqtt.Client()
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_publish = on_publish
mqttc.on_subscribe = on_subscribe

mqttc.connect("127.0.0.1", 1883, 60)

#Enable Monitor
mqttc.subscribe(enableTopic, 0)

#detector nodes
mqttc.subscribe("/house/door1", 0)


mqttc.loop_forever()


