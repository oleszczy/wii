#ifndef ORIENTATION_H_
#define ORIENTATION_H_

#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "cwiid.h"

#define AGGREGATE_COUNT 5
#define IGNORE_LIMIT 1e-3

float limit(float value,float min,float max);
void calibrate(cwiid_wiimote_t* device);
void fill_acc_readings(cwiid_wiimote_t* device,int cnt,union cwiid_mesg messages[], struct timespec * timestamp);
void get_acceleration(uint16_t x,uint16_t y,uint16_t z,float* orientation);
void get_orientation(uint16_t x,uint16_t y,uint16_t z,float* orientation);
void get_position(float ir[CWIID_IR_SRC_COUNT][2]);
















#endif /*ORIENTATION_H_*/
