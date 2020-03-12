/*******************************************************************************
* rc_test_adc.c
*
* James Strawson 2016
* prints voltages read by all adc channels
*******************************************************************************/

#include "../../libraries/rc_usefulincludes.h"
#include "../../libraries/redperipherallib.h"

int main(){
	
	int i;
	
	char *adc_file[7];
	adc_file[0] = "/sys/bus/iio/devices/iio:device0/in_voltage0_raw";
	adc_file[1] = "/sys/bus/iio/devices/iio:device0/in_voltage1_raw";
	adc_file[2] = "/sys/bus/iio/devices/iio:device0/in_voltage2_raw";
	adc_file[3] = "/sys/bus/iio/devices/iio:device0/in_voltage3_raw";
	adc_file[4] = "/sys/bus/iio/devices/iio:device0/in_voltage4_raw";
	adc_file[5] = "/sys/bus/iio/devices/iio:device0/in_voltage5_raw";
	adc_file[6] = "/sys/bus/iio/devices/iio:device0/in_voltage6_raw";
	
	char *data = malloc(5*sizeof(char));
	
	FILE *fp;
	
	for(i=0; i<7; i++){
		fp = fopen(adc_file[i], "r");
	
		if(fp < 0){
			printf("Error: iio file not found. Probably a device tree issue\n");
			return -1;
		}
	
		fgets(data, 5, fp);
	
		fclose(fp);
	
		printf(" adc_%d: %s\n", i, data);
	}
	free(data);
	fflush(stdout);
	return 0;
}
