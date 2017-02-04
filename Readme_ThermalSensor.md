#Thermal Sensor - Installation and Building Client/Server

Following arenecessary steps to install thermal sensor on BBBK node and run thermal client/server
1.) Build device tree for thermal sensor on BBBK node
2.) Build server and client programs binary for thermal reading


#1.) to build device tree and start thermal sensor in a BBBK node, 
use (refer http://archive.is/JzXkC or http://www.bonebrews.com/temperature-monitoring-with-the-ds18b20-on-a-beaglebone-black/)
Copy w1.dts file in the same directory as the following shell script and plug the thermal sensor pin in P8_11 on BBBK
Run command

bash startThermalSense.sh


#2.) to build thermal sensor files
for senseNet

Thermal sensor 1 (Server 1) 

a.) read cpu Usage using mpstat
Build command 

gcc -w -o server1 tempServer.c senseTemp51001.c cpuUsage.c

Run command

./server1 <port No.>


b.) read CPU utilization random values
Build command 

gcc -w -o server1 tempServer.c senseTemp51001.c cpuUsage2.c

Run command

./server1 <port No.>

Thermal sensor 2 (server 2)

c. ) read CPU utilization using mpstat
Build command

gcc -w -o server2 tempServer2.c senseTemp.c cpuUsage.c

Run command

./server2 <port No.>

d.) read CPU utilization using random values
Build command

gcc -w -o server2 tempServer2.c senseTemp.c cpuUsage2.c

Run command

./server2 <Port No.>

e.) Thermal client1
Build command

gcc -w -o client1 tempClient.c

Run command

./client1 <IP> <port No.> <Mode>          //Clients can operate in either manual (user decides the rate of thermal sensing)
                                          //or auto mode (automatic thermal sensing rate adjustments as per cpu usage rate on server side)

f. )Thermal client 2
Build command

gcc -w -o client2 tempClient2.c

Run command

./client2 <IP> <port No.> <Mode>
