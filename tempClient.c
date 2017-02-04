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
#include <stdlib.h>

#define cpuThreshold 75
#define cpuThresholdLow 30
#define highDelay 2000000
#define lowDelay 300000
#define medDelay 1000000

void error(char *msg)
{
    perror(msg);
    exit(0);
}

char* getToken(char *buffer, int tokenNum){

	int x = 0;
	char nextChar = *buffer;
	char *tokenVal = (char*)calloc(30,sizeof(char)); 
	memset(tokenVal,0x00,sizeof(tokenVal));
	int tokenCount = 0;
	
	if(nextChar ==  ' '){
		while(nextChar == ' '){
			nextChar = *(++buffer);
		}
	}
	
	while(tokenCount != tokenNum && buffer > 0x00){
		int i=0;
		memset(tokenVal,0x00,30);
		
		while(nextChar != ' '){
			tokenVal[i] = *(buffer);
			nextChar = *(++buffer);	
			i++;		
		}
		tokenCount += 1;
		if(tokenCount == tokenNum)break;
				
			if(nextChar ==  ' '){
				while(nextChar == ' '){
				nextChar = *(++buffer);
			}
	
		}
		
	}
	return tokenVal;
}

/*getSensor from the server data*/
char* getSensorType(char *buffer){	
	return getToken(buffer,1);
}

/*getsensorNodeId from the server data*/
int getSensorNodeId(char *buffer){
	return atoi(getToken(buffer,2));
}

/*getTemperature from the server data*/
float getTemperature(char *buffer){
	char* temp = getToken(buffer,3);
	float x = atof(temp);
	return atof(temp);
}

/*getcpuUsage from the server data*/
float getcpuUsage(char *buffer){
	return atof(getToken(buffer,4));
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, total;

    struct sockaddr_in serv_addr;
    struct hostent *server;
    FILE *fp;
    int newRate = 0;
    int prevRate = 0;
    int mode = 0; //Default (Auto) mode is 0,  userMode is 1 
    
    char buffer[1000];

/*	struct timeval tv;
	tv.tv_sec = 10;  // 30 Secs Timeout
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
*/

    if (argc < 4) {
       fprintf(stderr,"usage: %s <hostname> <port> <mode Manual=1, Auto=0>\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    mode = argv[3];

    if (sockfd < 0) 
        error("ERROR opening socket");
	
    //setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

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
    
    //printf("Please enter the required Rate: ");
    //bzero(buffer,sizeof(buffer));
    
    //fgets(buffer,sizeof(buffer)-1,stdin);
    //n = write(sockfd,buffer,strlen(buffer)+1);
    
         //error("ERROR writing to socket");
    	
	bzero(buffer,sizeof(buffer));
	while(1){
		
		if(fp = popen("cat delay.conf","r")){
			if(fp == NULL){
				printf("Failed to read the Rate");
				exit(1);
			}
		}
		
		while (fgets(buffer, sizeof(buffer)-1, fp) != NULL){
			newRate = atoi(buffer);			
		}
		
		if(1){	
			n = write(sockfd,buffer,strlen(buffer)+1);
			prevRate = newRate;
		}

		if (n < 0) 
         	error("ERROR writing to socket");
		
		pclose(fp);	

		total =0;
		bzero(buffer,sizeof(buffer));

		/*A do while loop is added to ensure the entire TCP pdu is read*/
		do{
			total+= (n = read(sockfd,buffer+total,sizeof(buffer)-1-total));
			
			if (n < 0) {error("ERROR reading from socket");
				close(sockfd);
			}
        
		}while(buffer[total-1] > 0);
	
		buffer[total] = 0;
	
		printf("Client Side: \n%s\n",buffer);
		int nodeId = getSensorNodeId(buffer);
		float temp = getTemperature(buffer);
		float cpu = getcpuUsage(buffer);
		printf("Sensor ID: %d\n", nodeId);			
		printf("Temperature: %.2f deg Celsius\n", temp);
		printf("Cpu Usage: %.2f %\n\n", cpu);
		
		if(mode == 0){

			char str1[30]; char str2[30];char str3[30];
			memset(str1,0x00,sizeof(str1));
			memset(str2,0x00,sizeof(str2));
			memset(str3,0x00,sizeof(str3));
			
			if(cpu > 0 || cpu < 100){
				fp = popen("rm ./delay.conf","r");
				pclose(fp);
	
				if(fp = fopen("./delay.conf","w")){
					if(fp == NULL){
						printf("Failed to read the Rate");
						exit(1);
					}
				}			
				if(cpu > cpuThreshold){				
					sprintf(str1, "%d", highDelay);
					fputs(str1, fp);
				}
	
				if(cpu < cpuThresholdLow){
					sprintf(str2, "%d", lowDelay);
					fputs(str2,fp);
				}
				
				if(cpu <= cpuThreshold &&  cpu >= cpuThresholdLow){
					sprintf(str3, "%d", medDelay);
					fputs(str3,fp);
				}
				fclose(fp);
			}
		}
							
		bzero(buffer,sizeof(buffer));
	}
    //close (sockfd);
    return 0;
}
