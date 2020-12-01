/* Assignment 1, Name: Simon Page, URN: 6266007 Email: sp00307@surrey.ac.uk*/

#include "contiki.h"
#include "dev/light-sensor.h"
#include "dev/sht11-sensor.h"
#include "dev/leds.h"
#include <stdio.h> /* For printf() */

// Defining Boolean Values Type
#define BOOL char
#define FALSE 0
#define TRUE 1

/* Required for Integer part of float */
unsigned short d1(float f){
  return((unsigned short)f);
}
/* Required for Fractional part of float */
unsigned short d2(float f){
  return(1000*(f-d1(f)));
}

/* Method to calculate the mean of values in an array */
float meanCalc(float inArray[], int sizeOfArray){
	int k;
	float sum = 0;
	for (k=0;k<sizeOfArray;k++)
	{
		sum += inArray[k];
	}
	return sum/sizeOfArray;
}

void printDebugInfo(float tempData[10], float lightData[10]){
int j;
	for (j = 0; j < 10; j++ ) {
	        printf("Temp[%d] = %u.%03u C\n", j, d1(tempData[j]), d2(tempData[j]));
		printf("Light[%d] = %u.%03u Lux\n", j, d1(lightData[j]), d2(lightData[j]));
   	}
	float meanTemp = meanCalc(tempData, 10);
	float meanLight = meanCalc(lightData, 10);
	printf("Temp Average: %u.%03u C\n", d1(meanTemp), d2(meanTemp));
	printf("Light Average: %u.%03u C\n", d1(meanLight), d2(meanLight));
}

float tempThreshold = 0;
float lightThreshold = 0;

/*---------------------------------------------------------------------------*/
PROCESS(fire_alarm, "Fire Alarm");
AUTOSTART_PROCESSES(&fire_alarm);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(fire_alarm, ev, data){

static struct etimer timer;
PROCESS_BEGIN();
etimer_set(&timer, CLOCK_CONF_SECOND*4);
SENSORS_ACTIVATE(light_sensor);  
SENSORS_ACTIVATE(sht11_sensor);
leds_off(LEDS_ALL);

printf("########################################################################\n");
printf("INITIALISING FIRE ALARM WITH AVERAGE ROOM TEMPERATURE AND LIGHT READINGS\n");
printf("########################################################################\n");

float initTempReadings[10];
float initLightReadings[10];
int i;

/* Loop to take the next 10 readings to derive  the thresholds from */
for (i = 0; i < 10; i++) {
	initTempReadings[i] = 0.01*sht11_sensor.value(SHT11_SENSOR_TEMP)-39.6;
	float V_sensor = 1.5 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC)/4096;
    	float I = V_sensor/100000;
    	initLightReadings[i] = 0.625*1e6*I*1000;
	printf("[%i] : %i C - %i Lux\n",i ,d1(initTempReadings[i]), d1(initLightReadings[i]));
}
printDebugInfo(initTempReadings, initLightReadings);

tempThreshold = meanCalc(initTempReadings, 10)+0.5;
lightThreshold = meanCalc(initLightReadings, 10)+30;

printf("\nThresholds Calculated\n");
printf("Temperature: %u.%03u C\n", d1(tempThreshold), d2(tempThreshold));
printf("Light: %u.%03u C\n", d1(lightThreshold), d2(lightThreshold));

printf("\n##############################\n");
printf("STARTING FIRE ALARM MONITERING\n");
printf("##############################\n");

/* Start of main control loop */
while(1){
	PROCESS_WAIT_EVENT_UNTIL(ev=PROCESS_EVENT_TIMER);

    	float currTemp = 0.01*sht11_sensor.value(SHT11_SENSOR_TEMP)-39.6;

    	float V_sensor = 1.5 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC)/4096;
    	float I = V_sensor/100000;
    	float currLight = 0.625*1e6*I*1000;
	printf("\nTemp Now: %u.%03u C\n", d1(currTemp), d2(currTemp));
	printf("Light Now: %u.%03u Lux\n", d1(currLight), d2(currLight));
	
	/* Logic Determines if the current temperature and light levels are above the thresholds or not */
	BOOL tempOK = FALSE, lightOK = FALSE; 
	if (d1(currTemp) < d1(tempThreshold) || (d1(currTemp) == d1(tempThreshold) && d2(currTemp) < d2(tempThreshold))){
		tempOK = TRUE;
	}
	if (d1(currLight) < d1(lightThreshold) || (d1(currLight) == d1(lightThreshold) && d2(currLight) < d2(lightThreshold))){
		lightOK = TRUE;
	}
	
	/* Depending on the temperature and light levels LED's and console messages will be displayed  (Event Notification)*/
	if(tempOK == TRUE && lightOK == TRUE){
		printf("All is good\n");
		leds_off(LEDS_ALL);
	} else if (tempOK == FALSE && lightOK == TRUE){
		printf("WARNING TEMPERATURE HIGH!\n");
		leds_off(LEDS_ALL);
		leds_on(LEDS_BLUE);
	} else if (tempOK == TRUE && lightOK == FALSE){
		printf("WARNING HIGH LIGHT LEVELS!\n");
		leds_off(LEDS_ALL);
		leds_on(LEDS_GREEN);
	} else {
		printf("ALERT BOTH LIGHT AND TEMP LEVELS HIGH, FIRE!!!\n");
		leds_off(LEDS_ALL);
		leds_on(LEDS_RED);
	}
	etimer_reset(&timer);
}
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
