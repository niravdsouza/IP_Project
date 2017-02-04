# Reference https://gist.github.com/nacx/733177
# Basic QoS script that uses Traffic Control to enqueue packets
# in four queues, each one with a different priority depending
# on the TOS bits
# The first queue has the highest priority and gets marked SSH 
# packets of tos 3
# The second one has a high priority and gets packets with 
# tos value 2.
# The third queue has the lower priority and gets packets with
# tos value 1
# The fourth queue has lowest priority and is for all unmarked 
# packets.

#################Variables used###############################

DEV=eth1           	# The gateway device
MAX=10000    	 	# The line capacity in kbps. 
#For all untagged traffic we are setting a cap of Max rate -1000 kbps
MIN=$[$MAX-1000]

#options to the admin deploting the script
case "$1" in
start)

        ### 1. Initialization ###

        tc qdisc del dev ${DEV} root 2>/dev/null    

        ### 2. Queue definition ###

        echo "Creating queues"

        # Create an HTB parent queue and assigning default packets to queue 40
        tc qdisc add dev ${DEV} root handle 1: htb default 40

        # Create a parent class to distribute the unused bandwith to the rest of queues
        tc class add dev ${DEV} parent 1: classid 1:1 htb rate ${MAX}kbit ceil ${MAX}kbit
	echo "Creating classes"
        
	# Create the classes in which the packets will be classified, with the appropiate
        # bandwith assigned
        tc class add dev ${DEV} parent 1:1 classid 1:10 htb rate ${MAX}kbit ceil ${MAX}kbit prio 1
        tc class add dev ${DEV} parent 1:1 classid 1:20 htb rate ${MAX}kbit ceil ${MAX}kbit prio 2
        tc class add dev ${DEV} parent 1:1 classid 1:30 htb rate ${MAX}kbit ceil ${MAX}kbit prio 3
        tc class add dev ${DEV} parent 1:1 classid 1:40 htb rate ${MIN}kbit ceil ${MAX}kbit prio 4
        # Add the SQF policy to the leaf classes
        #tc qdisc add dev ${DEV} parent 1:10 handle 10: sfq perturb 10
        #tc qdisc add dev ${DEV} parent 1:20 handle 20: sfq perturb 10
        #tc qdisc add dev ${DEV} parent 1:30 handle 30: sfq perturb 10
        #tc qdisc add dev ${DEV} parent 1:40 handle 40: sfq perturb 10

        ### 3. Create classifier filters ###

        echo "Creating classifier filters..."

        # Classify the marked pakets in the corresponding classes 
	tc filter add dev ${DEV} parent 1: protocol ip prio 1 u32 match ip tos 0x03 0xff flowid 1:10
        tc filter add dev ${DEV} parent 1: protocol ip prio 2 u32 match ip tos 0x02 0xff flowid 1:20
        tc filter add dev ${DEV} parent 1: protocol ip prio 3 u32 match ip tos 0x01 0xff flowid 1:30

;;
stop)
        echo "Removing existing queues and classes..."
        tc qdisc del dev ${DEV} root 2>/dev/null
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
#end of case
esac

exit 0
