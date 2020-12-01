/* Assignment 2, Name: Simon Page, URN: 6266007 Email: sp00307@surrey.ac.uk*/

#include "contiki.h"
#include "dev/light-sensor.h"
#include <stdio.h> /* For printf() */
#include <math.h>

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
	for (k=0;k<sizeOfArray;k++)	{
		sum += inArray[k];
	}
	return sum/sizeOfArray;
}

/* Method to calculate the Standard Deviation of values in an array */
float standardDeviationCalc(float inArray[], int sizeOfArray){
	int i;
	float mean = meanCalc(inArray, sizeOfArray), sum = 0;
	for (i = 0; i < sizeOfArray; i++)	{
		sum += (inArray[i] - mean) * (inArray[i] - mean);
	}
	sum = sqrtf(sum / sizeOfArray);	

	return sum;

}

/* Method to print resultant aggregated data from an input array */
void printAggData(float inArray[], int sizeOfArray){
	int k;
	printf("Aggregated Data is: (");
	for (k=0;k<sizeOfArray;k++){
		if (k == sizeOfArray-1){
			printf("%u.%03u)\n", d1(inArray[k]), d2(inArray[k]));
		} else {
			printf("%u.%03u, ", d1(inArray[k]), d2(inArray[k]));
		}
	}
}

/* Storage Buffer */
float dataBuffer[12];
short count = 0;

/*---------------------------------------------------------------------------*/
PROCESS(aggregation_algorithm, "Aggregation Algorithm");
AUTOSTART_PROCESSES(&aggregation_algorithm);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(aggregation_algorithm, ev, data){

static struct etimer timer;
PROCESS_BEGIN();
etimer_set(&timer, CLOCK_CONF_SECOND*2);
SENSORS_ACTIVATE(light_sensor);  

/* Control Loop */
while(1){
	PROCESS_WAIT_EVENT_UNTIL(ev=PROCESS_EVENT_TIMER);

	if (count == 0){
		printf("##################################\n");
		printf("RECORDING 12 LIGHT SENSOR READINGS\n");
		printf("##################################\n");
	}

	if (count > 11){ //Once 12 values are recorded

		printf("##############################\n");
		printf("CALCULATING STANDARD DEVIATION\n");
		printf("##############################\n");

		float stdDev = standardDeviationCalc(dataBuffer, 12);
		printf("SD: %u.%03u\n",d1(stdDev),d2(stdDev));
		
		printf("################\n");
		printf("AGGREGATING DATA\n");
		printf("################\n");

		int k;
		/* Following logic determines whether any data aggregation is to be applied based on Standard Deviation Values */
		if(stdDev > 200){ // No Data Aggregation
			float output[12];
			for(k=0;k<12;k++){
				output[k] = dataBuffer[k];
			}
			printAggData(output, 12);
		} else if (stdDev > 100 && stdDev <= 200){ // Data is paired up
			float output[6];
			for(k=0;k<6;k++){
				output[k] = (dataBuffer[2*k] + dataBuffer[(2*k)+1]) /2;
			}
			printAggData(output, 6);
		} else if (stdDev > 50 && stdDev <= 100){ //Data split into 3's
			float output[4];
			for(k=0;k<4;k++){
				output[k] = (dataBuffer[3*k] + dataBuffer[(3*k)+1] + dataBuffer[(3*k)+2]) /3;
			}
			printAggData(output, 4);
		} else if (stdDev > 30 && stdDev <= 50){ //Data split into 4's
			float output[3];
			for(k=0;k<3;k++){
				output[k] = (dataBuffer[4*k] + dataBuffer[(4*k)+1] + dataBuffer[(3*k)+2] + dataBuffer[(3*k)+3]) /4;
			}
			printAggData(output, 3);
		} else if (stdDev > 15 && stdDev <= 30) { //Data split into 6's
			float output[2];
			for(k=0;k<2;k++){
				output[k] = (dataBuffer[6*k] + dataBuffer[(6*k)+1] + dataBuffer[(3*k)+2] + dataBuffer[(3*k)+3] + dataBuffer[(3*k)+4] + dataBuffer[(3*k)+5]) /6;
			}
			printAggData(output, 2);
		} else { //Maximum Data aggregation
			float output[1];
			output[0] = meanCalc(dataBuffer, 12);
			printAggData(output, 1);
		}

		count = 0;

	} else {
		float V_sensor = 1.5 * light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC)/4096;
    		float I = V_sensor/100000;
    		dataBuffer[count] = 0.625*1e6*I*1000;
		printf("%d: %u.%03u Lux\n", count,d1(dataBuffer[count]),d2(dataBuffer[count]));
		count++;
	}

	etimer_reset(&timer);
}
PROCESS_END();
}
/*---------------------------------------------------------------------------*/



