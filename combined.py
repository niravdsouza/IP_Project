import time


n=9
while True:
    with open('/home/npathak2/c1.txt', 'r') as file:
        # read a list of lines into data
        thermal = file.readlines()
    #print thermal
    start=len(thermal)-n
    t1=thermal[start:len(thermal)-2]
    with open('/home/npathak2/c2.txt', 'r') as file:
        # read a list of lines into data
        thermal = file.readlines()
    #print thermal
    start=len(thermal)-n
    t2=thermal[start:len(thermal)-2]
    #t2=tail(thermal,9)
    with open('/home/nedsouza/motion.txt', 'r') as file:
        # read a list of lines into data
        thermal = file.readlines()
    #print thermal
    t3=thermal
    data=[0]
    data=t1+t2+t3

    with open('stats.txt', 'w') as file:
        file.writelines( data )
    time.sleep(1)
