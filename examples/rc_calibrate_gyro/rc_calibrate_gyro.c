/*******************************************************************************
* rc_calibrate_gyro.c
* James Strawson - 2016
*
* This program exists as an interface to the rc_calibrate_gyro_routine which
* manages collecting gyroscope data for averaging to find the steady state
* offsets.
*******************************************************************************/

#include "../../libraries/rc_usefulincludes.h"
#include "../../libraries/redperipherallib.h"

int main(){

	printf("Starting calibration routine\n");
	if(rc_calibrate_gyro_routine()<0){
		printf("Failed to complete gyro calibration\n");
		return -1;
	}
	printf("\ngyro calibration file written\n");
	printf("run rc_test_imu to check performance\n");
	return 0;
}
