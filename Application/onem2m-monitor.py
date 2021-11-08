# Version 1.1
import threading

import requests
import json
from flask import Flask
from flask import request
from flask import Response
import sys
import random
import configparser
import argparse

# /////////////// Parameters /////////////////
config = configparser.ConfigParser()
with open('default.json', 'r') as f:
    config = json.load(f)
# CSE Params
csePoA = "http://" + config["cse"]["ip"] + ":" + str(config["cse"]["port"])
cseName = config["cse"]["name"]
cseRelease = config["cse"]["release"]
poa_in_nu = config["cse"]["poa_in_nu"]
# AE params
monitorId = config["monitor"]["id"]
monitorIP = config["monitor"]["ip"]
monitorPort = config["monitor"]["port"]
monitorPoA = "http://" + monitorIP + ":" + str(monitorPort)

proxtrig1 = config["proximitytrig1"]["isprox1trig"]
proxtrig2 = config["proximitytrig2"]["isprox2trig"]
weighttrig = config["weighttrigger"]["isweighttrig"]
isLedEmptyOn = config["LedEmpty"]["isLedEmptyOn"]
isLedHalfOn = config["LedHalf"]["isLedHalfOn"]
isLedFullOn = config["LedFull"]["isLedFullOn"]
isLedHeavyOn = config["LedHeavy"]["isLedHeavyOn"]
proximity1Threshold = config["ProximitySensor1"]["proximity1Threshold"]
proximity2Threshold = config["ProximitySensor2"]["proximity2Threshold"]
weightThreshold = config["WeightSensor"]["weightThreshold"]
requestNr = 0


def createSUB(sensorName):
    global requestNr
    global cseRelease
    global poa_in_nu
    headers = {
        'Content-Type': 'application/json;ty=23',
        'X-M2M-Origin': monitorId,
        "X-M2M-RI": "req" + str(requestNr),
    }

    if (cseRelease != "1"):
        headers.update({"X-M2M-RVI": cseRelease})

    notificationUri = [monitorPoA + "/" + "Notification_" + sensorName]

    response = requests.post(csePoA + '/' + cseName + "/" + sensorName + '/DATA',
                             json={
                                 "m2m:sub": {
                                     "rn": "SUB_Monitor",
                                     "nu": notificationUri,
                                     "nct": 1,
                                     "enc": {
                                         "net": [3]
                                     }
                                 }
                             },
                             headers=headers
                             )
    requestNr += 1
    if response.status_code != 201:
        print("SUB Creation error : ", response.text)
    else:
        print("SUB Creation :", response.status_code)


def createAE():
    global requestNr
    global cseRelease
    headers = {
        'Content-Type': 'application/json;ty=2',
        'X-M2M-Origin': monitorId,
        "X-M2M-RI": "req" + str(requestNr),
    }
    ae_json = {
        "m2m:ae": {
            "rn": "Monitor",
            "api": "Norg.demo.monitor-app",
            "rr": True,
            "poa": [monitorPoA]
        }
    }
    if (cseRelease != "1"):
        headers.update({"X-M2M-RVI": cseRelease})
        ae_json['m2m:ae'].update({"srv": [cseRelease]})

    response = requests.post(csePoA + "/" + cseName,
                             json=ae_json,
                             headers=headers
                             )
    requestNr += 1
    if response.status_code != 201:
        print("AE Creation error : ", response.text)
    else:
        print("AE Creation :", response.status_code)
    createSUB("ProximitySensor1")
    createSUB("ProximitySensor2")
    createSUB("WeightSensor")


def createCIN(actuatorName, commandName):
    global requestNr
    global cseRelease
    headers = {
        'Content-Type': 'application/json;ty=4',
        'X-M2M-Origin': monitorId,
        "X-M2M-RI": "req" + str(requestNr),
    }

    if (cseRelease != "1"):
        headers.update({"X-M2M-RVI": cseRelease})

    response = requests.post(csePoA + "/" + cseName + "/" + actuatorName + '/COMMAND',
                             json={
                                 "m2m:cin": {
                                     "con": commandName
                                 }
                             },
                             headers=headers
                             )
    requestNr += 1
    if response.status_code != 201:
        print("CIN Creation error : ", response.text)
    else:
        print("CIN Creation :", response.status_code)


api = Flask(__name__)


@api.route('/Notification_ProximitySensor1', methods=['POST'])
def processNotificationProximity1():
    notificationJSON = request.json
    print("Receieved notification : ")
    print(json.dumps(notificationJSON, indent=3))

    if 'vrq' in notificationJSON['m2m:sgn']:
        print("It is a verification notification, nothing to do")
    else:
        print("It is an actual notification ")
        sensorValue = int(notificationJSON['m2m:sgn']['nev']['rep']['m2m:cin']['con'])
        print("Receieved ProximitySensor1 value : ", sensorValue)

        commandLedLvl(sensorValue, "ProximitySensor1")

    response = Response('')
    response.headers["X-M2M-RSC"] = 2000
    if (cseRelease != "1"):
        response.headers["X-M2M-RVI"] = cseRelease
    return response


@api.route('/Notification_ProximitySensor2', methods=['POST'])
def processNotificationProximity2():
    notificationJSON = request.json
    print("Receieved notification : ")
    print(json.dumps(notificationJSON, indent=3))

    if 'vrq' in notificationJSON['m2m:sgn']:
        print("It is a verification notification, nothing to do")
    else:
        print("It is an actual notification ")
        sensorValue = int(notificationJSON['m2m:sgn']['nev']['rep']['m2m:cin']['con'])
        print("Receieved ProximitySensor2 value : ", sensorValue)

        commandLedLvl(sensorValue, "ProximitySensor2")

    response = Response('')
    response.headers["X-M2M-RSC"] = 2000
    if (cseRelease != "1"):
        response.headers["X-M2M-RVI"] = cseRelease
    return response


@api.route('/Notification_WeightSensor', methods=['POST'])
def processNotificationWeight():
    notificationJSON = request.json
    print("Receieved notification : ")
    print(json.dumps(notificationJSON, indent=3))

    if 'vrq' in notificationJSON['m2m:sgn']:
        print("It is a verification notification, nothing to do")
    else:
        print("It is an actual notification ")
        sensorValue = int(notificationJSON['m2m:sgn']['nev']['rep']['m2m:cin']['con'])
        print("Receieved WeightSensor value : ", sensorValue)

        commandLedLvl(sensorValue, "WeightSensor")

    response = Response('')
    response.headers["X-M2M-RSC"] = 2000
    if (cseRelease != "1"):
        response.headers["X-M2M-RVI"] = cseRelease
    return response

def commandLedLvl(sensorValue, sensorName):
    global isLedEmptyOn
    global isLedHalfOn
    global isLedFullOn
    global isLedHeavyOn
    global proxtrig1
    global proxtrig2
    global weighttrig

    if (sensorName == "ProximitySensor1") and (sensorValue < proximity1Threshold):
        proxtrig1 = True
    elif(sensorName == "ProximitySensor1") and (sensorValue > proximity1Threshold):
        proxtrig1 = False

    if (sensorName == "ProximitySensor2") and (sensorValue < proximity2Threshold):
        proxtrig2 = True
    elif(sensorName == "ProximitySensor2") and (sensorValue > proximity2Threshold):
        proxtrig2 = False

    if (sensorName == "WeightSensor") and (sensorValue > weightThreshold):
        weighttrig = True
    elif(sensorName == "WeightSensor") and (sensorValue < weightThreshold):
        weighttrig = False


    if (proxtrig1 == False) and (proxtrig2 == False):
        print("Proximities normal => Switch On LedEmpty")
        createCIN("LedEmpty", "[switchOn]")
        isLedEmptyOn = True
    else:
        print("Proximities reduced => Switch Off LedEmpty")
        createCIN("LedEmpty", "[switchOff]")
        isLedEmptyOn = False


    if (proxtrig1 == True) and (proxtrig2 == False):
        print("Proximity1 reduced => Switch On LedHalf")
        createCIN("LedHalf", "[switchOn]")
        isLedHalfOn = True
    else:
        print("Proximities normal => Switch Off LedHalf")
        createCIN("LedHalf", "[switchOff]")
        isLedHalfOn = False


    if (proxtrig1 == True) and (proxtrig2 == True):
        print("Proximities reduced => Switch On LedFull")
        createCIN("LedFull", "[switchOn]")
        isLedFullOn = True
    else:
        print("Proximities normal => Switch Off LedFull")
        createCIN("LedFull", "[switchOff]")
        isLedFullOn = False


    if (weighttrig == True):
        print("Overweight in container => Switch On LedHeavy")
        createCIN("LedHeavy", "[switchOn]")
        isLedHeavyOn = True
    else:
        print("weight normal => Switch Off LedHeavy")
        createCIN("LedHeavy", "[switchOff]")
        isLedHeavyOn = False



def commandActuator(args):
    global actuatorToTrigger
    requestNr = random.randint(0, 1000)
    print("The command " + args.command + " will be sent to the actuator " + args.actuator)
    actuatorToTrigger = args.actuator + "Actuator"
    createCIN(actuatorToTrigger, "[" + args.command + "]")
    sys.exit()


def getAll(args):
    global requestNr
    global cseRelease

    requestNr = random.randint(0, 1000)
    print("Sending request >>> ")
    headers = {
        'X-M2M-Origin': monitorId,
        "X-M2M-RI": "req" + str(requestNr),
        'Accept': 'application/json'
    }
    if (cseRelease != "1"):
        headers.update({"X-M2M-RVI": cseRelease})

    response = requests.get(csePoA + "/" + cseName + "/" + args.sensor + "Sensor/DATA" + '?rcn=4',
                            headers=headers)

    print("<<< Response received ! ")

    if response.status_code != 200:
        print("Error = ", response.text)
    else:
        print("Effective content of CINs = ")
        contentInstanceInJSON = json.loads(response.content)
        for elt in contentInstanceInJSON['m2m:cnt']['m2m:cin']:
            print("   " + elt['con'])

    sys.exit()


def getLatest(args):
    global requestNr
    global cseRelease
    requestNr = random.randint(0, 1000)
    print("Sending request >>> ")
    headers = {
        'X-M2M-Origin': monitorId,
        "X-M2M-RI": "req" + str(requestNr),
        'Accept': 'application/json'
    }
    if (cseRelease != "1"):
        headers.update({"X-M2M-RVI": cseRelease})

    response = requests.get(csePoA + "/" + cseName + "/" + args.sensor + "Sensor/DATA/la",
                            headers=headers)

    requestNr += 1
    print("<<< Response received ! ")

    if response.status_code != 200:
        print("Error = ", response.text)
    else:
        cin = json.loads(response.content)
        print("Effective content of CIN = ", cin['m2m:cin']['con'])

    sys.exit()

def flaskThread():
    api.run(host=monitorIP, port=monitorPort)

if __name__ == '__main__':



    threading.Thread(target=flaskThread).start()
    createAE()
