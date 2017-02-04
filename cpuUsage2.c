#include "cpuUsage.h"
#include <stdlib.h>
#define RATE = 1000
#define TestBranch 0
#define bufSize 1000
#define cpuTest 0
static int count = 0;

char* extractCpuUtil(char* temp){
	int i =0,k=0;
	char* util;
	util = (char*)calloc(10,sizeof(char));	
	int spaceFound = 0;

	i =0;
	while(i < 3){
		char x = temp[k];
		if(x == ' '){
			if(spaceFound == 0){			
				i++;
				spaceFound = 1;	
			}
		}
		
		if(x != ' '){
			spaceFound = 0;
		}
		k++;

	}

	int x =0;
	while(temp[k] == ' '){
		k++;
		continue;
	}

	while(temp[k] != ' '){
		util[x++] = temp[k++];	
	}

	util[x] = 0x00;
	return util;
}

float getCpuUsage(){
	
	float cpuUsage;	
	FILE *fp;
	char cpuReading[bufSize];
	char *cpuUtil;
	if(fp = popen("mpstat | tail -1","r")){
	//if(fp = popen("top -b -d1 -n1|grep -i \"Cpu(s)\"|head -c21|cut -d ' ' -f2|cut -d '%' -f1","r")){	
		if(fp == NULL){
			printf("Failed to trace CPU utilization");
			exit(1);
		}
	}

	while (fgets(cpuReading, sizeof(cpuReading)-1, fp) != NULL){
		cpuUtil  = extractCpuUtil(cpuReading);
		
	}
	pclose(fp);
	//return atof(cpuUtil);
	//"Generate random cpu utilization values"
	float randUsage = 0;
	if(count <= 10){	
		randUsage = (float)(rand() % (30 + 1 - 0)) + 0;
		count++;
	}else if(count <= 20){
		randUsage = (float)(rand() % (75 + 1 - 20)) + 30;		
		count++;
	}else if(count <= 30){
		randUsage = (float)(rand() % (99 + 1 - 75)) + 75;
		count++;
	}else{
		randUsage = (float) (rand()%100);
		count = 0;
	}

	return randUsage;
}

#if cpuTest
int main(){
	float x = getCpuUsage();
	printf("%.2f",x);

}

#endif
