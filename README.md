# IoT Network Traffic Regulator

##Thermal Sensor - Installation and Building Client/Server

Following arenecessary steps to install thermal sensor on BBBK node and run thermal client/server
1.) Build device tree for thermal sensor on BBBK node
2.) Build server and client programs binary for thermal reading


##1.) to build device tree and start thermal sensor in a BBBK node
use (refer http://archive.is/JzXkC or http://www.bonebrews.com/temperature-monitoring-with-the-ds18b20-on-a-beaglebone-black/)
Copy w1.dts file in the same directory as the following shell script and plug the thermal sensor pin in P8_11 on BBBK
Run command

bash startThermalSense.sh


##2.) to build thermal sensor files
for senseNet

Thermal sensor 1 (sensor node 1) 

###a.) read cpu Usage using mpstat
Build command 

gcc -w -o server1 tempServer.c senseTemp51001.c cpuUsage.c

Run command

./server1 <port No.>


###b.) read CPU utilization random values
Build command 

gcc -w -o server1 tempServer.c senseTemp51001.c cpuUsage2.c

Run command

./server1 <port No.>

Thermal sensor 2 (sensor node 2)

###c. ) read CPU utilization using mpstat
Build command

gcc -w -o server2 tempServer2.c senseTemp.c cpuUsage.c

Run command

./server2 <port No.>

###d.) read CPU utilization using random values
Build command

gcc -w -o server2 tempServer2.c senseTemp.c cpuUsage2.c

Run command

./server2 <Port No.>

###e.) Thermal client1 (terminal node)
Build command

gcc -w -o client1 tempClient.c

Run command

./client1 <IP> <port No.> <Mode>          //Clients can operate in either manual (user decides the rate of thermal sensing)
                                          //or auto mode (automatic thermal sensing rate adjustments as per cpu usage rate on server side)
                                          
for manual mode, we will have to write the delay value (in milliseconds) in delay.conf file.                                          

###f. )Thermal client 2 (Terminal node)
Build command

gcc -w -o client2 tempClient2.c

Run command

./client2 <IP> <port No.> <Mode>




##Motion Sensor - Installation Client/Server 
### On the server (on Sensor node 3)
Run the command: python pir_sensor.py <portno> 

### On the client (terminal node)
Run the command: python pir.py <serverip> <portno>



##Building and deploying the kernel module (On processing node):

### Make
Run the command "make" to generate tos.ko file using Makefile

### Deploy
Run the command (with sudo privileges) insmod tos.ko

### UnDeploy
Run the command (with sudo) rmmmod tos

### To view logs
Run the command dmesg

##Monitoring the Queue (on Intermediate Node):

###To start monitoring
bash qos_main.sh start                        

###To stop monitoring
bash qos_main.sh stop

###To monitor the queue
bash qos_main.sh monitor  

###To view status of the queue
bash qos_main.sh status                        
