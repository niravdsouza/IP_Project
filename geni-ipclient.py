import socket
import sys
import io

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

data=[0,0]
data[0]='Is there motion in the room?\n'
data[1]='No '
with open('motion.txt', 'w') as file:
    file.writelines( data )


# Connect the socket to the port on the server given by the caller
server_address = ("192.168.77.13",8001)
print >>sys.stderr, 'connecting to %s port %s' % server_address
sock.connect(server_address)
while 1:
	try:
		output = sock.recv(2048)
		if output=="0":
			data[1]='No '
		else:
			data[1]='Yes'
		with open('motion.txt', 'w') as file:
    			file.writelines( data )
		
	except:
		sock.close()
