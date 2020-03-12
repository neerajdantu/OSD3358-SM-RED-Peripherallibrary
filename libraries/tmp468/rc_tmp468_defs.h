/*******************************************************************************
* tmp468_defs.h
* Neeraj Dantu 2017
*
* Register definitions for the TMP468 temperature sensor
 ******************************************************************************/
#ifndef _TMP468_H
#define _TMP468_H

// conversion rate definitions
#define TMP468_CONV_RATE_0_0625HZ       0
#define TMP468_CONV_RATE_0_125HZ        1
#define TMP468_CONV_RATE_0_25HZ         2
#define TMP468_CONV_RATE_0_5HZ          3
#define TMP468_CONV_RATE_1HZ            4
#define TMP468_CONV_RATE_2HZ            5
#define TMP468_CONV_RATE_3HZ            6
#define TMP468_CONV_RATE_CONT           7


// valid channel definitions
#define TMP468_CHANNEL_LOC              0
#define TMP468_CHANNEL_1                1
#define TMP468_CHANNEL_2                2
#define TMP468_CHANNEL_3                3
#define TMP468_CHANNEL_4                4
#define TMP468_CHANNEL_8                8


// function definitions
int tmp468_initialize();                                            // initailizes the temperature sensor
double tmp468_read_temp(int channel);                               // reads the data register for input channels  0 - 8
int tmp468_channel_isenabled(int channel);                          // reads channel open status for input channels 1 - 8. Returns 0 if channel is closed. Returns -2 if channel is open.
int tmp468_disable_channel(int channel);                            // disables the temperature measurement for input channels 1 - 8. Returns 0 if successful. Returns -1 if not successful.
int tmp468_enable_channel(int channel);                             // enables the temperature measurement for input channels 1 - 8. Returns 0 if successful. Returns -1 if not successful.
int tmp468_shutdown();                                              // disables all temperture measurement for all channels. Returns 0 if successful. Returns -1 if not successful.
int tmp468_setup_oneshot_conversion(int channel);                   // sets up a one shot conversion with all channels except the input disabled. Returns 0 if successful. Returns -1 if not successful.
double tmp468_read_temp_oneshot(int channel);                       // reads the temperature measruement register in single shot mode. Needs to be called after tmp468_setup_oneshot_conversion. Returns temperature measureument in double format
int tmp468_soft_reset();                                            // resets the temperature sensor. All registers change to default POR states. Returns 0 if successful. Returns -1 if not successful.
int tmp468_set_conversion_rate(uint16_t conversion_rate);           // sets a conversion rate for the sensor. input conversion_rate must be between 0 - 7. See table 10 in tmp468 data sheet for inputs and corresponding frequencies of conversion. Returns 0 if successful. Returns -1 if not successful.
int tmp468_set_eta_correction(int channel, uint16_t eta);           // adjusts the eta constant in the temperatre measurement formula. See section 7.6.1.8 for definition and table 12 for valid inputs and correspinding correction factors. Returns 0 if successful. Returns -1 if not successful.
int tmp468_lock();                                                  // locks all registers preventing reads and writes. Returns 0 if successful. Returns -1 if not successful.
int tmp468_unlock();                                                // unlocks the temperature sensor registers. Returns 0 if successful. Returns -1 if not successful.
uint16_t tmp468_get_device_id();                                    // returns 16 bit device id of the temperature sensor
uint16_t tmp468_get_mfg_id();                                       // returns 16 bit manufacturer id of the temperature sensor
int tmp468_set_offset_register(int channel, uint16_t offset);       // updates the offset register for precision calibration. Each channel has 1 offset register whose value will be added to the temperature value of the channel after measurement

#endif
