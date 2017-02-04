#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RATE = 1000
#define TestBranch 0

/*Function to extract temperature reading from the string*/
char* extractTemperature(char* tempString){

	char *temperature;
	char *pattern = "t=";
	char *temp ;
	char tPointer[2];
	temperature = (char*)calloc(1,sizeof(char));
	tPointer[1]= '\0';

	/*Find the location of the patter*/
	temp = strstr(tempString, pattern);
	if(temp != NULL){
		temp += strlen(pattern);

		 /*Reading the value of the string*/
		while(*temp != '\0'){
			/*Break the loop on encountering space or null*/
                	if(*temp == ' '){
                        	break;
                	}
			tPointer[0]=*temp;
                	temp = temp +1;
			strncat(temperature,tPointer,1);
        	}
	}else{
		/*Return temperature not Registered*/
		//puts("No tempretaure registered");
		//exit(1);
	}

	/*Return the registered temperature*/
        return temperature;

}


/*Function to read the temperatute */
float senseTemperature(){
	FILE *fp;
	char tempReading[1000];
	char *tempValue;
	int ti = 0;
	/*Open and Execute the command for reading temp*/
	//fp = popen("cat /sys/devices/w1_bus_master1/28-0000045d9d8a/w1_slave");

	//fp = popen("ls -a &","r");
	
	
#if TestBranch
	if(fp = popen("cat /sys/devices/w1_bus_master1/28-00000521e301/w1_slave","r")){
		
		if(fp == NULL){
			printf("Failed to sense Temperature");
			exit(1);
		}

		/* Read the output a line at a time - output it. */
	  	while (fgets(tempReading, sizeof(tempReading)-1, fp) != NULL) {
	    		//printf("%s", tempReading);
#endif
			char * myString= "$ cat /sys/devices/w1_bus_master1/28-0000045d9d8a/w1_slave\
			 07 01 4b 46 7f ff 09 10 da : crc=da YES\
			 07 01 4b 46 7f ff 09 10 da t=16437";
			char *tempString = "This is my string t=127231";
#if TestBranch
			tempValue = extractTemperature(tempReading);		
#else
			tempValue = extractTemperature(myString);
#endif		
			if(*tempValue != 0.000){
				/*Convert from string to integer*/
				int i;char ci;
				for(i=0; i < strlen(tempValue); i++){
					ci = tempValue[i];
					ti = (ti*10) + (ci - 0x30);
				}
				return ((float)ti/1000);

			}
#if TestBranch			
			else{
				//printf("I continue");
				continue;
			}
	  	}

		/* close */
	  	pclose(fp);

	}else{
		//do nothing
		//exit(1);
	}
#endif

	return (float)ti;	
}

/*Get the current Temperature*/
float getTemperature(){
	float temp = 0.0;
	return (temp = senseTemperature());
}

/*main function*/
/*
int main(){
	
	//while(1)
	printf("Temperature = %.3f deg Celsius\n",getTemperature());
	fflush(stdout);
	return 0;
}
*/
