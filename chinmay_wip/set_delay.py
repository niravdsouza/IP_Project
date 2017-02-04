while True:
    delay = raw_input("\nEnter delay: ")
    target = open("delay.conf", 'w')
    target.truncate()
    target.write(delay+"\n")
