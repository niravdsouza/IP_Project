/*ld instruction : gcc -o tcp-client_corrected tcp-client_corrected.c

To run: ./tcp-client_corrected 127.0.0.1 9876

where 127.0.0.1 is the local IP.
where 9876 is a port number
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, total;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[1000];
    if (argc < 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("Please enter the required Rate: ");
    bzero(buffer,sizeof(buffer));
    fgets(buffer,sizeof(buffer)-1,stdin);
    n = write(sockfd,buffer,strlen(buffer)+1);
    if (n < 0) 
         error("ERROR writing to socket");
    
    bzero(buffer,sizeof(buffer));
	
	total =0;
	//A do while loop is added to ensure the entire TCP pdu is read
		do{
				total+= (n = read(sockfd,buffer+total,sizeof(buffer)-1-total));
				if (n < 0) {error("ERROR reading from socket");
					close(sockfd);
				}
	
		}while(buffer[total-1] > 0);
	buffer[total] = 0;
	printf("%s\n",buffer);
	bzero(buffer,sizeof(buffer));

    close (sockfd);
    return 0;
}

