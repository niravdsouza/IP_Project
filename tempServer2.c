/*
Build instruction : gcc -o tcp-iterative_corrected tcp-iteartive_corrected.c

To run: ./tcp-iterative_corrected 9876

where 9876 is a port number
*/


/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "senseTemp.h"
#include "cpuUsage.h"

#define PORT_NO 6666
/*
void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
	    
	while (goal > clock());
	
}
*/

void error(char *msg)
{
    perror(msg);
    exit(1);
}


int readRate(){
	FILE *fp;
	char rateReading[1000];
	int rate = 1000;
 
	if(fp = popen("cat ./rateFile.txt","r")){
		if(fp == NULL){
				printf("Failed to sense Temperature");
				exit(1);
		}
		fgets(rateReading, sizeof(rateReading)-1, fp);
		
		int i=0;
		while(rateReading[i] != '\0'){
			rate = (rate*10) + (atoi(rateReading[i]));
			i++;
		}
	}

	return rate;	

}


int main(int argc, char *argv[])
{
	char buffer[1000];
	char *sName = "ThermalData";
	char *id = "51002";
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	char dtTm[64];
	char temperature[10];
	char cpuUtil[10];		
	int portno;// = PORT_NO;
	
	int sockfd, newsockfd, clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n, total;
	int rate = 1000;
	
	if (argc < 2) {
		fprintf(stderr,"usage: %s <port>\n", argv[0]);
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
		error("ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);

	for(;;){
		
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		
		if (newsockfd < 0)
			error("ERROR on accept");
		bzero(buffer,sizeof(buffer));
		
		/*
		//do while loop is added to ensure the entire TCP pdu is read
		total = 0;
		do{
			total += (n = read(newsockfd,buffer+total,sizeof(buffer)-1-total));
			if (n < 0) error("ERROR reading from socket");

		}while(buffer[total-1] > 0);
		*/
		
		//buffer[total] = 0;		
		while(1){
			
			memset(buffer,0x00,sizeof(buffer)/sizeof(char));

			/*do while loop is added to ensure the entire TCP pdu is read*/
			total = 0;
			do{
				total += (n = read(newsockfd,buffer+total,sizeof(buffer)-1-total));
				if (n < 0) error("ERROR reading from socket");

			}while(buffer[total-1] > 0);
			buffer[total] = 0;
						
			rate = atoi(buffer);
									

			memset(buffer,0x00,sizeof(buffer)/sizeof(char));			
			memset(temperature,0x00,sizeof(temperature)/sizeof(char));
			memset(dtTm,0x00,sizeof(dtTm)/sizeof(char));
			snprintf(temperature,sizeof(temperature),"%f",getTemperature());
			strftime(dtTm, sizeof(dtTm), "%m/%d/%Y %H:%M",tm);
			snprintf(cpuUtil,sizeof(cpuUtil),"%f",getCpuUsage());
		
			memcpy(buffer,sName,strlen(sName));
			memcpy(buffer + strlen(buffer)," ",strlen(" "));
			memcpy(buffer+strlen(buffer),id,strlen(id));
			memcpy(buffer + strlen(buffer)," ",strlen(" "));
			memcpy(buffer+strlen(buffer),temperature,strlen(temperature));
			memcpy(buffer + strlen(buffer)," ",strlen(" "));
			memcpy(buffer+strlen(buffer),cpuUtil,strlen(cpuUtil));
			//memcpy(buffer + strlen(buffer), "\n", 1);
			memcpy(buffer + strlen(buffer)," ",strlen(" "));
			memcpy(buffer + strlen(buffer), dtTm, strlen(dtTm));
			memcpy(buffer + strlen(buffer), "$", 1);
			
			printf("Server side Logged values: \n%s\n",buffer);
			printf("Current rate is %d ms\n",rate/1000);
			printf("Temperature is %.2f deg Celsius\n", getTemperature());
			printf("Cpu Usage is %.2f %\n\n", atof(cpuUtil));
			
			n = write(newsockfd,buffer,strlen(buffer)+1);
					
			if (n < 0){
				error("ERROR writing to socket");				
				close(newsockfd);
				break;			
			}

			usleep(rate);
		}	
	}
}
