
while True:
    rate = raw_input("\nEnter delay: ")
    target = open("rate.config", 'w')
    target.truncate()
    target.write(rate+"\n")


