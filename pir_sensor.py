#####################################################################
#     This program is the PIR Sensor program                        #
#     Use python pir_sensor.py to run                               # 
####################################################################
import socket
import time
#import random
import sys
import Adafruit_BBIO.GPIO as GPIO

#########Functions###########
def connect():
    global connected,connection
    print 'waiting for a connection'
    try:
        connection, client_address = sock.accept()
        print 'client connected:', client_address
        connected=1
        return connection
    except:
        return

if __name__ == '__main__':
    #Initializing pins
   
    GPIO.setup("P8_12", GPIO.IN)
    GPIO.setup("P8_11", GPIO.OUT)

#Defining the port no and IP to send from
    serverName = "192.168.77.13"
    serverPort = int(sys.argv[1])

# Defining the socket and binding the port
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

#Getting a port number for the server
    sock.bind((serverName,serverPort))
    port=sock.getsockname()[1]
    sock.listen(1)

#Variable to check if a connection is made
    count=0
    x=0
    connection=connect()
    for i in range(0,5):   
        try:
            while True:
         
                if GPIO.input("P8_12"):
                    print "Alarm"
                    print "1"
                    connection.sendall("1")
                    time.sleep(1)
                else:
                    print "0"
                    connection.sendall("0")
                    time.sleep(1)
                    #print "No alarm"""
        except:
            sock.close()
            # Defining the socket and binding the port
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            #Getting a port number for the server
            sock.bind((serverName,serverPort))
            port=sock.getsockname()[1]
            sock.listen(1)

            connection=connect()
