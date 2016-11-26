#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  integracion_1.py
#  



from __future__ import print_function
import zmq,sys
from pubnub import Pubnub
import time   #,sys


def main():
	
	#-------------------------------------------------------------------
	# ZEROmq
	#-------------------------------------------------------------------
	
	print("Connecting to Data Service…")
	sys.stdout.flush()
	context = zmq.Context()
	
	#  Socket to talk to server
	socket = context.socket(zmq.REQ)
	socket.connect("tcp://127.0.0.1:5555")
	print("Connected....")
	
	#-------------------------------------------------------------------
	# PUBNUB
	#-------------------------------------------------------------------
	
	pubnub = Pubnub(publish_key="pub-c-7d62813d-6e07-4b09-b009-98dc15011db4", subscribe_key="sub-c-9931e424-aad5-11e6-af18-02ee2ddab7fe")
	
	def callback(message, channel):
		print(message)
	#pubnub.time(callback=_callback)
  
	def error(message):
		print("ERROR : " + str(message))
  
	def connect(message):
		print("CONNECTED")
		#print pubnub.publish(channel='prueba', message='Hello from the PubNub Python SDK')
  
	def reconnect(message):
		print("RECONNECTED")
    
	def disconnect(message):
		print("DISCONNECTED")
	
	pubnub.subscribe(channels='pruebaa', callback=callback, error=callback, connect=connect, reconnect=reconnect, disconnect=disconnect)

	#-------------------------------------------------------------------
	# Comienzo
	#-------------------------------------------------------------------
	
	
	
	while(True):
		
		
		# ----------------------------------------
		# ZERO mq
		# ----------------------------------------
		#print("\r\nSending request …")
		socket.send("Requesting... ")
	
		#  Get the reply.
		message = socket.recv()
	
		#print("Time %s" % message, end='\r')
		print(message)
		time.sleep(1)
		
		
		# ----------------------------------------
		# PUBNUB
		# ----------------------------------------
		
		pubnub.subscribe(channels='prueba', callback=callback, error=callback, connect=connect, reconnect=reconnect, disconnect=disconnect)
		pubnub.publish(channel='prueba', message=message )
		#time.sleep(3)
		
	

	
	return 0

if __name__ == '__main__':
	main()
