dtc -O dtb -o w1-00A0.dtbo -b 0 -@ w1.dts
sudo cp w1-00A0.dtbo /lib/firmware
sudo chmod 777 /sys/devices/bone_capemgr.9/slots
echo w1 > /sys/devices/bone_capemgr.9/slots
cat /sys/devices/bone_capemgr.9/slots
ls /sys/devices/w1_bus_master1
cat /sys/devices/w1_bus_master1/28-00000521e301/w1_slave
