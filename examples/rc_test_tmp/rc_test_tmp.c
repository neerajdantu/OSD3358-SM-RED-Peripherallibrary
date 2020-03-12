/*******************************************************************************
* rc_test_imu.c
*
* This serves as an example of how to read the IMU with direct reads to the
* sensor registers. To use the DMP or interrupt-driven timing see test_dmp.c
*******************************************************************************/

#include "../../libraries/rc_usefulincludes.h"
#include "../../libraries/redperipherallib.h"
#include "../../libraries/tmp468/rc_tmp468_defs.h"

int main(){
	
	int ret;
	double temp_value;
	
	ret = tmp468_initialize();
	
	if(ret != 0){
		printf("Exiting\n");
		return 0;
	}
	
	sleep(2);
	
	
	temp_value = tmp468_read_temp(TMP468_CHANNEL_LOC);
	
	printf("temperature local sensor read value in degrees celsius: %f\n", temp_value);
	
	printf("exiting\n");
	return 0;
}
