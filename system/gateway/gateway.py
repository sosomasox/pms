from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
from AWSIoTPythonSDK.exception.AWSIoTExceptions import publishTimeoutException
from xbee import ZigBee
from datetime import datetime
import logging
import time
import argparse
import json
import os
import ast
import serial
import threading
import commands


AllowedActions = ['both', 'publish', 'subscribe']
PORT = '/dev/ttyUSB0'
BAND_RATE = 9600
SERIAL_PORT = serial.Serial(PORT, BAND_RATE)
MAIN_TOPIC = "MonitoringSystemForAgedPeopleLivingAlone/"
myAWSIoTMQTTClient = None



# Custom MQTT message callback
def customCallback(client, userdata, message):
	print("Received a new message: ")
	print(message.payload)
	print("from topic: ")
	print(message.topic)
	print("--------------\n\n")



def handleXBee(xbee_packet):
	#time_STR = datetime.now().strftime("%H:%M:%S")
	#date_STR = datetime.now().strftime("%Y%m%d")
	timestamp_STR = datetime.now().strftime("%Y/%m/%d %H:%M:%S")
	file_name_STR = datetime.now().strftime("%Y%m%d") + ".json"

	try:
		payload_DICT = ast.literal_eval((xbee_packet['rf_data']).decode('utf-8'))

		payload_DICT.update({'timestamp' : timestamp_STR})
		sub_topic = os.uname()[1]
		topic = MAIN_TOPIC + sub_topic + "/sensing_data"
		payload_JSON = json.dumps(payload_DICT)

		print "topic   :", topic
		print "payload :", payload_JSON
		
		with open(file_name_STR, 'a') as fp:
			json.dump(payload_DICT, fp)
			fp.write("\n")

		myAWSIoTMQTTClient.publish(topic, payload_JSON, 1)

	except publishTimeoutException:
		print("publishTimeoutException in handleXBeeh")
		os.system("/etc/ifplugd/action.d/action_wpa wlan0 up")
	
	except UnicodeDecodeError: 
		None
	
	except ValueError:
		None



if __name__ == '__main__':
    # Read in command-line parameters
    parser = argparse.ArgumentParser()
    parser.add_argument("-e", "--endpoint", action="store", required=True, dest="host", help="Your AWS IoT custom endpoint")
    parser.add_argument("-r", "--rootCA", action="store", required=True, dest="rootCAPath", help="Root CA file path")
    parser.add_argument("-c", "--cert", action="store", dest="certificatePath", help="Certificate file path")
    parser.add_argument("-k", "--key", action="store", dest="privateKeyPath", help="Private key file path")
    parser.add_argument("-p", "--port", action="store", dest="port", type=int, help="Port number override")
    parser.add_argument("-w", "--websocket", action="store_true", dest="useWebsocket", default=False,
                        help="Use MQTT over WebSocket")
    parser.add_argument("-id", "--clientId", action="store", dest="clientId", default="basicPubSub",
                        help="Targeted client id")
    parser.add_argument("-t", "--topic", action="store", dest="topic", default="sdk/test/Python", help="Targeted topic")
    parser.add_argument("-m", "--mode", action="store", dest="mode", default="both",
                        help="Operation modes: %s"%str(AllowedActions))
    parser.add_argument("-M", "--message", action="store", dest="message", default="Hello World!",
                        help="Message to publish")

    args = parser.parse_args()
    host = args.host
    rootCAPath = args.rootCAPath
    certificatePath = args.certificatePath
    privateKeyPath = args.privateKeyPath
    port = args.port
    useWebsocket = args.useWebsocket
    clientId = args.clientId
    topic = args.topic

    if args.mode not in AllowedActions:
        parser.error("Unknown --mode option %s. Must be one of %s" % (args.mode, str(AllowedActions)))
        exit(2)

    if args.useWebsocket and args.certificatePath and args.privateKeyPath:
        parser.error("X.509 cert authentication and WebSocket are mutual exclusive. Please pick one.")
        exit(2)

    if not args.useWebsocket and (not args.certificatePath or not args.privateKeyPath):
        parser.error("Missing credentials for authentication.")
        exit(2)

    # Port defaults
    if args.useWebsocket and not args.port:  # When no port override for WebSocket, default to 443
        port = 443
    if not args.useWebsocket and not args.port:  # When no port override for non-WebSocket, default to 8883
        port = 8883

    # Configure logging
    logger = logging.getLogger("AWSIoTPythonSDK.core")
    logger.setLevel(logging.DEBUG)
    streamHandler = logging.StreamHandler()
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    streamHandler.setFormatter(formatter)
    logger.addHandler(streamHandler)

    # Init AWSIoTMQTTClient
    if useWebsocket:
        myAWSIoTMQTTClient = AWSIoTMQTTClient(clientId, useWebsocket=True)
        myAWSIoTMQTTClient.configureEndpoint(host, port)
        myAWSIoTMQTTClient.configureCredentials(rootCAPath)
    else:
        myAWSIoTMQTTClient = AWSIoTMQTTClient(clientId)
        myAWSIoTMQTTClient.configureEndpoint(host, port)
        myAWSIoTMQTTClient.configureCredentials(rootCAPath, privateKeyPath, certificatePath)

    # AWSIoTMQTTClient connection configuration
    myAWSIoTMQTTClient.configureAutoReconnectBackoffTime(1, 32, 20)
    myAWSIoTMQTTClient.configureOfflinePublishQueueing(-1)  # Infinite offline Publish queueing
    myAWSIoTMQTTClient.configureDrainingFrequency(2)  # Draining: 2 Hz
    myAWSIoTMQTTClient.configureConnectDisconnectTimeout(10)  # 10 sec
    myAWSIoTMQTTClient.configureMQTTOperationTimeout(5)  # 5 sec


    # Connect and subscribe to AWS IoT
    myAWSIoTMQTTClient.connect()
    if args.mode == 'both' or args.mode == 'subscribe':
	    myAWSIoTMQTTClient.subscribe(topic, 1, customCallback)
    time.sleep(2)


    # Publish to the same topic in a loop forever
    loopCount = 0
    xbee = ZigBee(SERIAL_PORT, escaped=True, callback=handleXBee)
    os.chdir("/home/pi/sensing_data")


    try:
	    while True:
		    time.sleep(0.000001)

    except KeyboardInterrupt:
	    xbee.halt()
	    SERIAL_PORT.close()
	    os._exit(1)
