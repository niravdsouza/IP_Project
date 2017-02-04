import socket
import sys

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port on the server given by the caller
server_address = (sys.argv[1], int(sys.argv[2]))
print >>sys.stderr, 'connecting to %s port %s' % server_address
sock.connect(server_address)
while 1:
	try:
		data = sock.recv(2048)
		if data:
			print 'received "%s"' % data

	except:
		sock.close()
