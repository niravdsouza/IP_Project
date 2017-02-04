#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RATE = 1000
#define TestBranch 0

static int count = 0;
/*Get the current Temperature*/
float getTemperature(){

    int a[12] = {20, 34, 33, 31, 36, 35, 33, 26, 25, 21, 22, 20}; 
	float temp = 0.0;
    count++;
	return (float) a[(count-1)%12];
}
