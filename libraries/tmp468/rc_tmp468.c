/*******************************************************************************
*  rc_bmp280.c
*******************************************************************************/

#include "../redperipherallib.h"
#include "../rc_defs.h"
#include "rc_tmp468_defs.h"
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h> // for uint8_t types etc
#include <fcntl.h>

//defines
#define TEMP_I2C_ADD                    0x48
#define TEMP_I2C_BUS                    0

// temperature registers
#define TMP468_TEMP_LOCAL	            0x00
#define TMP468_TEMP1    	            0x01
#define TMP468_TEMP2        	        0x02
#define TMP468_TEMP3        	        0x03
#define TMP468_TEMP4    		        0x04
#define TMP468_TEMP5    		        0x05
#define TMP468_TEMP6    		        0x06
#define TMP468_TEMP7    		        0x07
#define TMP468_TEMP8    		        0x08


#define TMP468_SOFTWARE_RESET           0x20

// status registers
#define TMP468_REMOTE_OPEN_STATUS	    0x23

// configuration registers
#define TMP468_CONF_REG             	0x30

// local temperature conf registers
#define TMP468_LOC_BLOCK_READ_RANGE     0x80

// remote remperature 1 conf registers
#define TMP468_REM_TEMP1_OFFSET     	0x40
#define TMP468_REM_TEMP1_NFACT    	    0x41


// remote remperature 2 conf registers
#define TMP468_REM_TEMP2_OFFSET     	0x48
#define TMP468_REM_TEMP2_NFACT    	    0x49


// remote remperature 3 conf registers
#define TMP468_REM_TEMP3_OFFSET     	0x50
#define TMP468_REM_TEMP3_NFACT    	    0x51


// remote remperature 4 conf registers
#define TMP468_REM_TEMP4_OFFSET     	0x58
#define TMP468_REM_TEMP4_NFACT    	    0x59


// remote remperature 5 conf registers
#define TMP468_REM_TEMP5_OFFSET     	0x60
#define TMP468_REM_TEMP5_NFACT    	    0x61


// remote remperature 6 conf registers
#define TMP468_REM_TEMP6_OFFSET     	0x68
#define TMP468_REM_TEMP6_NFACT    	    0x69


// remote remperature 7 conf registers
#define TMP468_REM_TEMP7_OFFSET     	0x70
#define TMP468_REM_TEMP7_NFACT    	    0x71


// remote remperature 8 conf registers
#define TMP468_REM_TEMP8_OFFSET     	0x78
#define TMP468_REM_TEMP8_NFACT    	    0x79


// lock register
#define TMP468_LOCK_REG                 0xC4


// identification registers
#define TMP468_MFG_ID_REG               0xFE
#define TMP468_DEV_REV_REG              0xFF



/************************************************************************************
 * Initialize the temperature sensor tmp468. This funciton reads the device ID of the 
 * temp sensor and verifies the presence of the device. It then soft-resets the sensor
 * registers.
 * **********************************************************************************/
int tmp468_initialize(){
	
	uint16_t *dev_ID = malloc(sizeof(uint16_t)), *lock_status = malloc(sizeof(uint16_t));
	uint16_t id = 1128, reset = (1 << 15), unlock_pwd = 0xEB19;
	*lock_status = 0x0000;

	// make sure the bus is not currently in use by another thread
	// do not proceed to prevent interfering with that process
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("i2c bus claimed by another process\n");
		printf("Continuing with tmp468_initialize() anyway.\n");
	}
	
	// initialize the bus
	if(rc_i2c_init(TEMP_I2C_BUS,TEMP_I2C_ADD)<0){
		printf("ERROR: failed to initialize i2c bus\n");
		printf("aborting tmp468_initialize\n");
		return -1;
	}

	// claiming the bus does no guarantee other code will not interfere 
	// with this process, but best to claim it so other code can check
	// like we did above
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	
	// read the device ID from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_DEV_REV_REG, dev_ID)<0){
		printf("ERROR: failed to read word on temp sensor\n");
		printf("aborting tmp468 initialize\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	if(id == *dev_ID){
		printf("tmp468 device ID verified\n");
	}
	
	// check if the registers are locked
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_LOCK_REG, lock_status)<0){
		printf("ERROR: failed to read lock status word on temp sensor\n");
		printf("aborting tmp468 initialize\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	if(*lock_status == 0x8000){
		
		if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_LOCK_REG, unlock_pwd)<0){
			printf("ERROR: failed to unlock temp sensor\n");
			printf("aborting tmp468 initialize\n");
			rc_i2c_release_bus(TEMP_I2C_BUS);
			return -1;
		}
		// check if the registers are locked
		if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_LOCK_REG, lock_status)<0){
			printf("ERROR: failed to read lock status word on temp sensor\n");
			printf("aborting tmp468 initialize\n");
			rc_i2c_release_bus(TEMP_I2C_BUS);
			return -1;
		}
		
		if(*lock_status == 0x0000){
			printf("sensor unlocked!\n");
		}
		
	}
	
	// reset the temperature sensor
	if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_SOFTWARE_RESET, reset)<0){
		printf("ERROR: failed to reset temp sensor\n");
		printf("aborting tmp468 initialize\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	else{
		printf("temperature sensor successfully initilaized\n");
	}
	
	if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_LOCK_REG, unlock_pwd)<0){
		printf("ERROR: failed to unlock temp sensor\n");
		printf("aborting tmp468 initialize\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	else{
		printf("temp sensor reset and registers unlocked\n");
	}
	// release control of the bus
	rc_i2c_release_bus(TEMP_I2C_BUS);
	
	return 0;
}

/***************************************************************************************
 * Read the temperature value of a channel. 
 * input: channel number. 0 -- local temperature
 *						  1 - 8 -- 8 remote channels
 * output: temperature in double format
 * ************************************************************************************/
double tmp468_read_temp(int channel){
	
	double temp_value;
	uint16_t *raw_temp_data = malloc(sizeof(uint16_t));
	uint8_t temp_address;
	int ret;
	
	if((channel<0) && (channel>8)){
		printf("the input channel should be 0 - 8\n");
		return -1;
	}
	
	ret = tmp468_channel_isenabled(channel);
	if (ret == 0)
		return 9999;
	
	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return 9999;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return 9999;
	}
	
	switch(channel){
		
		case 0:
			temp_address = TMP468_TEMP_LOCAL;
			break;
		
		case 1:
			temp_address = TMP468_TEMP1;
			break;
		
		case 2:
			temp_address = TMP468_TEMP2;
			break;
			
		case 3:
			temp_address = TMP468_TEMP3;
			break;
		
		case 4:
			temp_address = TMP468_TEMP4;
			break;
		
		case 5:
			temp_address = TMP468_TEMP5;
			break;
		
		case 6:
			temp_address = TMP468_TEMP6;
			break;
			
		case 7:
			temp_address = TMP468_TEMP7;
			break;
			
		case 8:
			temp_address = TMP468_TEMP8;
			break;
		
		default:
			printf("input channel not between 0 and 8\n");
			printf("exiting tmp468 read\n");
			rc_i2c_release_bus(TEMP_I2C_BUS);
			return 9999;
	}
	
	// read the value from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, temp_address, raw_temp_data)<0){
		printf("ERROR: failed to read temperature data on temp sensor\n");
		printf("aborting tmp468 read\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return 9999;
	}
	
	// if data is in twos complement form, convert to decimal
	if((raw_temp_data && (1<16)) == 0){
		temp_value = -(double)((~(*raw_temp_data) + 1) >> 3) * 0.0625;							////figure this one out please
	}
	else{
		temp_value = (double)(*raw_temp_data >> 3) * 0.0625;
	}
	
	rc_i2c_release_bus(TEMP_I2C_BUS);
	return temp_value;
}

/***************************************************************************************
 * Read the status of channels of the temperature sensor 
 * input: channel number. 1 - 8 -- 8 remote channels
 * output:  0 if channel is open and/or ADC conversion is disabled on the channel
 *		    1 if channel is closed and enabled 
 *		   -1 if an error occured
 * ************************************************************************************/
int tmp468_channel_isenabled(int channel){
	
	uint16_t *channel_status = malloc(sizeof(uint16_t)), *config_reg = malloc(sizeof(uint16_t));
	uint16_t config_check;
	
	config_check = (1 << (channel + 7));
	
	// make sure input channel is in bounds
	if((channel<1) && (channel>8)){
		printf("the input channel should be 1 - 8\n");
		return -1;
	}
	
	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the staus register from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_REMOTE_OPEN_STATUS, channel_status)<0){
		printf("ERROR: failed to read status on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}

	if((*channel_status & (1 << (channel + 7))) == 0){
		
		// read the config register from the temp sensor
		if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
			printf("ERROR: failed to read config register on temp sensor\n");
			printf("aborting..\n");
			rc_i2c_release_bus(TEMP_I2C_BUS);
			return -1;
		}

		if((*config_reg & config_check) != 0){
			rc_i2c_release_bus(TEMP_I2C_BUS);
			return 1;
		}
		else{
			rc_i2c_release_bus(TEMP_I2C_BUS);
			return 0;			
		}
	}
	else{
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return 0;
	}

}

/***************************************************************************************
 * Disable channel ADC conversion for the input channel
 * input: channel number. 0 -- local channel
 *						  1 - 8 -- 8 remote channels
 * output: -1 if action was unsuccessful
 *		    0 if action was successful 
 * ************************************************************************************/

int tmp468_disable_channel(int channel){
	
	uint16_t *config_reg = malloc(sizeof(uint16_t));
	uint16_t config_data;
	
	// make sure input channel is in bounds
	if((channel<0) && (channel>8)){
		printf("the input channel should be 0 - 8\n");
		return -1;
	}
	
	config_data = ~(1 << (channel + 7));

	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}

	// detetmine the register to write	
	config_data = (*config_reg & config_data);
	
	// write data to config register
	if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_data)<0){
		printf("ERROR: failed to disable channel\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor to confirm write
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// release I2C bus
	rc_i2c_release_bus(TEMP_I2C_BUS);
	
	if(*config_reg == config_data){
		return 0;
	}
	else{
		return -1;
	}
	
}

/***************************************************************************************
 * Enable ADC onversion of input channel
 * input: channel number. 0 -- local channel
 *						  1 - 8 -- 8 remote channels
 * output:  0 if channel is enabled
 *		   -1 if action was unsuccessful
 * ************************************************************************************/
int tmp468_enable_channel(int channel){
	
	uint16_t *config_reg = malloc(sizeof(uint16_t)), *channel_status = malloc(sizeof(uint16_t));
	uint16_t config_data;
	
	// make sure input channel is in bounds
	if((channel<0) && (channel>8)){
		printf("the input channel should be 0 - 8\n");
		return -1;
	}
	

	config_data = (1 << (channel + 7));

	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the staus register from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_REMOTE_OPEN_STATUS, channel_status)<0){
		printf("ERROR: failed to read status on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	if((*channel_status & (1 << (channel + 7))) != 0){
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}

	// determine the register to write	
	config_data = (*config_reg | config_data);
	
	// write data to config register
	if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_data)<0){
		printf("ERROR: failed to enable channel\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor to confirm write
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// release I2C bus
	rc_i2c_release_bus(TEMP_I2C_BUS);
	
	if(*config_reg == config_data){
		return 0;
	}
	else{
		return -1;
	}
	
}


/***************************************************************************************
 * Shutdown ADC conversion for all channels of the temperature sensor
 * input: none
 * output:  0 if successful
 *		   -1 if unsuccessful
 * ************************************************************************************/
int tmp468_shutdown(){
	
	uint16_t *config_reg = malloc(sizeof(uint16_t));
	uint16_t config_data;
	
	// setting shutdown bit
	config_data = (1 << 5);

	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}

	// determine the register to write	
	config_data = (*config_reg | config_data);
	
	// write data to config register
	if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_data)<0){
		printf("ERROR: failed to shutdown conversion\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor to confirm write
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// release I2C bus
	rc_i2c_release_bus(TEMP_I2C_BUS);
	
	if(*config_reg == (config_data & ~(1 << 1))){
		return 0;
	}
	else{
		return -1;
	}
}

/***************************************************************************************
 * Setup single shot conversion for input channel. All other channels will be disabled 
 * and ADC is shutdown.
 * input: channel number. 0 -- local channel
 *						  1 - 8 -- 8 remote channels
 * output:  0 if successful
 *		   -1 if unsuccessful
 * ************************************************************************************/

int tmp468_setup_oneshot_conversion(int channel){

	uint16_t *config_reg = malloc(sizeof(uint16_t));
	uint16_t config_data, channel_select;
	
	// setting shutdown bit
	config_data = (1 << 5);
	// setting all enable bits except the input channel to 0
	channel_select = (1 << (channel + 7)) | 0x007F;


	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}

	// determine the register to write	
	config_data = (*config_reg | config_data) & channel_select;
	
	// write data to config register
	if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_data)<0){
		printf("ERROR: failed to shutdown conversion and set enable bit for input channel\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor to confirm write
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// verifying if oneshot conversion setup was successful
	if(*config_reg != (config_data & ~(1 << 1))){
		return -1;
	}
	
	// release I2C bus
	rc_i2c_release_bus(TEMP_I2C_BUS);
	return 0;
	
}

/***************************************************************************************
 * enable an instance of single shot conversion read the temperature value on the input
 * channel
 * 
 * input: channel number. 0 -- local channel
 *						  1 - 8 -- 8 remote channels
 * output:  value of temperature sensor in double format
 * ************************************************************************************/
double tmp468_read_temp_oneshot(int channel){
	
	uint16_t *config_reg = malloc(sizeof(uint16_t));
	uint16_t config_data;
	double temp_value;
	
	// make sure input channel is in bounds
	if((channel<0) && (channel>8)){
		printf("the input channel should be 0 - 8\n");
		return -1;
	}
	
	config_data = (1 << 6);

	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	if((*config_reg & (1 << 5)) == 0){
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}

	// determine the register to write	
	config_data = (*config_reg | config_data);
	
	// write data to config register
	if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_data)<0){
		printf("ERROR: failed to set one shot bit in config register\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor to confirm write
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// release I2C bus
	rc_i2c_release_bus(TEMP_I2C_BUS);
	
	// delay for data to be available at the temp sensor register
	usleep(20000);
	temp_value = tmp468_read_temp(channel);
	
	return temp_value;
}

/***************************************************************************************
 * Soft reset the temperature sensor and unlock the registers 
 * 
 * input: none
 * output:  0 if successful
 *		   -1 if unsuccessful
 * ************************************************************************************/
int tmp468_soft_reset(){
	
	uint16_t reset = (1 << 15), unlock_pwd = 0xEB19;
	
	// reset the temperature sensor
	if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_SOFTWARE_RESET, reset)<0){
		printf("ERROR: failed to reset temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_LOCK_REG, unlock_pwd)<0){
		printf("ERROR: failed to unlock temp sensor\n");
		printf("aborting tmp468 reset\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	// release control of the bus
	rc_i2c_release_bus(TEMP_I2C_BUS);
	
	return 0;
}

/***************************************************************************************
 * Set conversion rate of the temperature sensor 
 * 
 * input: uint16_t conversion rate between 0 - 7.
 *			please see tmp468 datasheet table 10 for details on input
 * output:  0 if successful
 *		   -1 if unsuccessful
 * ************************************************************************************/
int tmp468_set_conversion_rate(uint16_t conversion_rate){
	
	uint16_t *config_reg = malloc(sizeof(uint16_t));
	uint16_t config_data_or, config_data_and, config_data;
	
	// make sure input conversion_rate is in bounds
	if(conversion_rate>7){
		printf("the input channel should be 0 - 7\n");
		return -1;
	}
	
	config_data_or = (conversion_rate <<  2) | 0x001C;
	config_data_and = (conversion_rate <<  2) | 0xFFE3;

	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}

	// determine the register to write	
	config_data = ((*config_reg | config_data_or) & config_data_and);
	
	printf("config register before conversion rate setup: %d \n", *config_reg);
	
	// write data to config register
	if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_data)<0){
		printf("ERROR: failed to enable channel\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor to confirm write
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_CONF_REG, config_reg)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	printf("config register after conversion rate setup: %d \n", *config_reg);
	
	// release I2C bus
	rc_i2c_release_bus(TEMP_I2C_BUS);
	
	if(*config_reg == config_data){
		return 0;
	}
	else{
		return -1;
	}
}

/***************************************************************************************
 * Lock the tmp468 registers. Please use caution when using this function. It may cause 
 * future I2C writes to fail
 * 
 * input: none
 * output:  0 if successful
 *		   -1 if unsuccessful
 * ************************************************************************************/
int tmp468_lock(){
	
	uint16_t *lock_status = malloc(sizeof(uint16_t));
	uint16_t lock_pwd = 0x5CA6;
	
	*lock_status = 0x0000;
	
	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// check if the registers are locked
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_LOCK_REG, lock_status)<0){
		printf("ERROR: failed to read lock status word on temp sensor\n");
		printf("aborting tmp468 initialize\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	if(*lock_status == 0x8000){
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return 0;
	}
	else{
		if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_LOCK_REG, lock_pwd)<0){
			printf("ERROR: failed to unlock temp sensor\n");
			printf("aborting tmp468 initialize\n");
			rc_i2c_release_bus(TEMP_I2C_BUS);
			return -1;
		}
		// check if the registers are locked
		if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_LOCK_REG, lock_status)<0){
			printf("ERROR: failed to read lock status word on temp sensor\n");
			printf("aborting tmp468 initialize\n");
			rc_i2c_release_bus(TEMP_I2C_BUS);
			usleep(50000);
			return -1;
		}
		if(*lock_status == 0x8000){
			rc_i2c_release_bus(TEMP_I2C_BUS);
			usleep(50000);
			return 0;
		}
		else{
			rc_i2c_release_bus(TEMP_I2C_BUS);
			usleep(50000);
			return -1;
		}
	}
	
}

/***************************************************************************************
 * unlock the tmp468 registers. 
 * input: none
 * output:  0 if successful
 *		   -1 if unsuccessful
 * ************************************************************************************/
int tmp468_unlock(){
	
	uint16_t *lock_status = malloc(sizeof(uint16_t));
	uint16_t unlock_pwd = 0xEB19;
	
	*lock_status = 0x0000;
	
	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// check if the registers are locked
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_LOCK_REG, lock_status)<0){
		printf("ERROR: failed to read lock status word on temp sensor\n");
		printf("aborting tmp468 initialize\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	if(*lock_status == 0x8000){
		
		if(rc_i2c_write_word(TEMP_I2C_BUS, TMP468_LOCK_REG, unlock_pwd)<0){
			printf("ERROR: failed to unlock temp sensor\n");
			printf("aborting tmp468 initialize\n");
			rc_i2c_release_bus(TEMP_I2C_BUS);
			return -1;
		}
		// check if the registers are locked
		if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_LOCK_REG, lock_status)<0){
			printf("ERROR: failed to read lock status word on temp sensor\n");
			printf("aborting tmp468 initialize\n");
			rc_i2c_release_bus(TEMP_I2C_BUS);
			return -1;
		}
		if(*lock_status == 0x0000){
			rc_i2c_release_bus(TEMP_I2C_BUS);
			usleep(50000);
			return 0;
		}
		else{
			rc_i2c_release_bus(TEMP_I2C_BUS);
			usleep(50000);
			return -1;
		}
	}

	else if (*lock_status == 0x0000){
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return 0;
	}
	else{
		printf("An error has occured. No valid value in lock register\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
}

/***************************************************************************************
 * get device ID of the temp sensor 
 * input: none
 * output: device ID of the temp sensor in uint16_t format
 * ************************************************************************************/
uint16_t tmp468_get_device_id(){
	
	uint16_t *dev_ID = malloc(sizeof(uint16_t));
	
	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the device ID from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_DEV_REV_REG, dev_ID)<0){
		printf("ERROR: failed to read word on temp sensor\n");
		printf("aborting tmp468 initialize\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	rc_i2c_release_bus(TEMP_I2C_BUS);
	
	return *dev_ID;
}


/***************************************************************************************
 * get manufacturer ID of the temp sensor 
 * input: none
 * output: device ID of the temp sensor in uint16_t format
 * ************************************************************************************/
uint16_t tmp468_get_mfg_id(){
	
	uint16_t *mfg_ID = malloc(sizeof(uint16_t));
	
	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the device ID from the temp sensor
	if(rc_i2c_read_word(TEMP_I2C_BUS, TMP468_MFG_ID_REG, mfg_ID)<0){
		printf("ERROR: failed to read word on temp sensor\n");
		printf("aborting tmp468 initialize\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	return *mfg_ID;
}

/***************************************************************************************
 * set temperature offset register for the corresponding channel
 * 
 * input: integer channel, uint16_t offset (same as temp register)
 * NOTE: The last 3 LSBs of the register are always 0 same as temperature register.
 * 
 * output: 0 if successful
 *		  -1 if unsuccessful
 * ************************************************************************************/
 
int tmp468_set_offset_register(int channel, uint16_t offset){
	
	uint8_t address;
	uint16_t *offset_read = malloc(sizeof(uint16_t));
	
	if((channel < 1)||(channel > 8)){
		printf("The input channel should be between 1 - 8\n");
		return -1;
	}
	
	switch(channel){
		
		case 1:
			address = TMP468_REM_TEMP1_OFFSET;
			break;
		case 2:
			address = TMP468_REM_TEMP2_OFFSET;
			break;
		case 3:
			address = TMP468_REM_TEMP3_OFFSET;
			break;
		case 4:
			address = TMP468_REM_TEMP4_OFFSET;
			break;
		case 5:
			address = TMP468_REM_TEMP5_OFFSET;
			break;
		case 6:
			address = TMP468_REM_TEMP6_OFFSET;
			break;
		case 7:
			address = TMP468_REM_TEMP7_OFFSET;
			break;
		case 8:
			address = TMP468_REM_TEMP8_OFFSET;
			break;
		default:
			printf("Error: something went wrong. channel is not between 1 - 8\n");
			return -1;
	}
	
	
	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	
	// write data to config register
	if(rc_i2c_write_word(TEMP_I2C_BUS, address, offset)<0){
		printf("ERROR: failed to shutdown conversion and set enable bit for input channel\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor to confirm write
	if(rc_i2c_read_word(TEMP_I2C_BUS, address, offset_read)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	rc_i2c_release_bus(TEMP_I2C_BUS);
	
	// wait half a sec to let it settle 
	usleep(500000);

	if(*offset_read == offset)
		return 0;
	else
		return -1;
		
}

/***************************************************************************************
 * set eta correction register for the corresponding channel for calibration
 * 
 * input: integer channel, uint16_t correction 
 * NOTE: The last 8 LSBs of the register are always 0. Please see table 12 for possible
 * inputs of correction factor
 * 
 * output: 0 if successful
 *		  -1 if unsuccessful
 * ************************************************************************************/
 
int tmp468_set_eta_correction(int channel, uint16_t correction){
	
	uint8_t address;
	uint16_t *correction_read = malloc(sizeof(uint16_t));
	
	if((channel < 1)||(channel > 8)){
		printf("The input channel should be between 1 - 8\n");
		return -1;
	}
	
	switch(channel){
		
		case 1:
			address = TMP468_REM_TEMP1_NFACT;
			break;
		case 2:
			address = TMP468_REM_TEMP2_NFACT;
			break;
		case 3:
			address = TMP468_REM_TEMP3_NFACT;
			break;
		case 4:
			address = TMP468_REM_TEMP4_NFACT;
			break;
		case 5:
			address = TMP468_REM_TEMP5_NFACT;
			break;
		case 6:
			address = TMP468_REM_TEMP6_NFACT;
			break;
		case 7:
			address = TMP468_REM_TEMP7_NFACT;
			break;
		case 8:
			address = TMP468_REM_TEMP8_NFACT;
			break;
		default:
			printf("Error: something went wrong. channel is not between 1 - 8\n");
			return -1;
	}
	
	
	// check claim bus state to avoid stepping on other reads
	if(rc_i2c_get_in_use_state(TEMP_I2C_BUS)){
		printf("WARNING: i2c bus is claimed, aborting tmp468_read_temp\n");
		return -1;
	}
	
	// claim bus for ourselves and set the device address
	rc_i2c_claim_bus(TEMP_I2C_BUS);
	if(rc_i2c_set_device_address(TEMP_I2C_BUS, TEMP_I2C_ADD)<0){
		printf("ERROR: failed to set the i2c device address to temp sensor address\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	
	// write data to config register
	if(rc_i2c_write_word(TEMP_I2C_BUS, address, correction)<0){
		printf("ERROR: failed to shutdown conversion and set enable bit for input channel\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	// read the config register from the temp sensor to confirm write
	if(rc_i2c_read_word(TEMP_I2C_BUS, address, correction_read)<0){
		printf("ERROR: failed to read config register on temp sensor\n");
		printf("aborting..\n");
		rc_i2c_release_bus(TEMP_I2C_BUS);
		return -1;
	}
	
	rc_i2c_release_bus(TEMP_I2C_BUS);
	
	// wait half a sec to let it settle 
	usleep(500000);

	if(*correction_read == correction)
		return 0;
	else
		return -1;
		
}
