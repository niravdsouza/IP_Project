#!/bin/bash

# Basic QoS script that uses OpenDPI (http://www.opendpi.org/)
# Netfilter wrapper to identify packets by the protocol.
#
# This script enqueues packets in three queues, each one
# with a different priority:
#
# The first queue has the higher priority and gets the TCP SYN
# packets, ACKs, ICMPs and packets with Minimize-Delay TOS.
# The second one has a normal priority and gets all unmarked
# packets.
# The third queue has the lower priority and gets all traffic
# which delivering can be delayed (p2p traffic, etc) in order
# to guarantee that the rest of packets will be delivered as
# soon as possible.


############################
### Script configuration ###
############################

DEV=eth1           # The gateway device
BANDWIDTH=10000     # The line upload capacity in kbps

################################
### End script configuration ###
################################


# We will set the maximum bandwith to use to the 90% of the upload capacity.
# This will guarantee that the traffic will be enqueues in our operating system
# and not in the router.
# We also set the maximum bandwith for the queues with lower priority to 10kbps
# less than the queue with higher priority.
MAX=$[$BANDWIDTH*90/100]
P1=$[$MAX]
P2=$[$MAX-10]
P3=$[$MAX-20]

case "$1" in
start)

        ### 1. Initialization ###

        #modprobe ip_conntrack   # Required to mark packets in the connection

        tc qdisc del dev ${DEV} root 2>/dev/null    # Delete existing queues and classes


        ### 2. Queue definition ###

        echo "Creating queues and classes..."

        # Create an HTB parent queue and Default class 30
        tc qdisc add dev ${DEV} root handle 1: htb default 30

        # Create a parent class to distribute the unused bandwith to the rest of queues
        tc class add dev ${DEV} parent 1: classid 1:1 htb rate ${MAX}kbit ceil ${MAX}kbit

        # Create the classes in which the packets will be classified, with the appropiate
        # bandwith assigned
        tc class add dev ${DEV} parent 1:1 classid 1:10 htb rate ${P1}kbit ceil ${MAX}kbit prio 1
        tc class add dev ${DEV} parent 1:1 classid 1:20 htb rate ${P2}kbit ceil ${MAX}kbit prio 2
        tc class add dev ${DEV} parent 1:1 classid 1:30 htb rate ${P3}kbit ceil ${MAX}kbit prio 3

        # Add the SQF policy to the leaf classes
        tc qdisc add dev ${DEV} parent 1:10 handle 10: sfq perturb 10
        tc qdisc add dev ${DEV} parent 1:20 handle 20: sfq perturb 10
        tc qdisc add dev ${DEV} parent 1:30 handle 30: sfq perturb 10


        ### 3. Create classifier filters ###

        echo "Creating classifier filters..."

        # Classify the marked pakets in the corresponding classes
        # Unmarked packets will be classified in the default class (1:30)
        tc filter add dev ${DEV} parent 1: protocol ip prio 1 u32 match ip tos 0x02 0xff                #Motion Data
        tc filter add dev ${DEV} parent 1: protocol ip prio 1 u32 match ip sport 22 0xffff              #SSH Traffic
        tc filter add dev ${DEV} parent 1: protocol ip prio 2 u32 match ip tos 0x01 0xff                #Thermal Data
        tc filter add dev ${DEV} parent 1: protocol ip prio 2 u32 match ip tos 0x07 0xff                #IPERF Data
        tc filter add dev ${DEV} parent 1: protocol ip prio 1 u32 match ip tos 0x08 0xff                #IPERF Data
        
;;
stop)
        echo "Removing existing queues and classes..."
        tc qdisc del dev ${DEV} root 2>/dev/null

        echo "Removing packet marking rules..."
        #iptables -t mangle -F
;;
restart)
        ${0} stop
        ${0} start
;;
status)
        tc -s qdisc show dev ${DEV}
;;
monitor)
        watch -n 1 "tc -s qdisc show dev ${DEV}"
;;
*)
        echo "Usage: ${0} {start|stop|restart|status|monitor}"
        exit 1
;;
esac

exit 0
