#include "orientation.h"

//variables needed for counting the device's orientation
//their values is counted during the calibration phase
//float x_0,x_1,x_2,x_3,y_0,y_1,y_2,y_3,z_0,z_1,z_2,z_3;
float x_0 = 124.500000; float x_1 = 125.000000; float x_2 = 124.000000; float x_3 = 149.000000;
float y_0 = 124.000000; float y_1 = 125.000000; float y_2 = 149.000000; float y_3 = 123.000000;
float z_0 = 127.000000; float z_1 = 152.000000; float z_2 = 126.000000; float z_3 = 128.000000;

//device accelerometer readings
uint16_t reading_count = 0;
uint16_t readings[3] = {0,0,0};

//semaphore for accessing readings
sem_t readings_sem;

void calibrate(cwiid_wiimote_t* device)
{
	cwiid_command(device,CWIID_CMD_RPT_MODE,CWIID_RPT_ACC | CWIID_RPT_BTN);
	cwiid_set_mesg_callback(device,fill_acc_readings);

	sem_init(&readings_sem,0,1);

	printf("=======================================================\n");
	printf("Calibration started\n");
	printf("Step 1: Please put the device in horizontal position with the A button facing up and press the A button...");
	fflush(stdout);

	//clear the counter and the readings
	sem_wait(&readings_sem);
	reading_count = AGGREGATE_COUNT;
	readings[0] = 0;
	readings[1] = 0;
	readings[2] = 0;
	sem_post(&readings_sem);

	while(1)
	{
		sem_wait(&readings_sem);
		if(!reading_count)
		{
			x_1 = readings[0]/AGGREGATE_COUNT;
			y_1 = readings[1]/AGGREGATE_COUNT;
			z_1 = readings[2]/AGGREGATE_COUNT;
			sem_post(&readings_sem);
			break;
		}
		sem_post(&readings_sem);
	}

	printf("Thank you\n");
	sleep(1);

	printf("Step 2: Please put the device in vertical position with the IR sensor facing down and press the A button...");
	fflush(stdout);

	sem_wait(&readings_sem);
	reading_count = AGGREGATE_COUNT;
	readings[0] = 0;
	readings[1] = 0;
	readings[2] = 0;
	sem_post(&readings_sem);
	while(1)
	{
		sem_wait(&readings_sem);
		if(!reading_count)
		{
			x_2 = readings[0]/AGGREGATE_COUNT;
			y_2 = readings[1]/AGGREGATE_COUNT;
			z_2 = readings[2]/AGGREGATE_COUNT;
			sem_post(&readings_sem);
			break;
		}
		sem_post(&readings_sem);
	}
	printf("Thank you\n");
	sleep(1);

	printf("Step 3: Please lay the device on its right side and press the A button...");
	fflush(stdout);

	sem_wait(&readings_sem);
	reading_count = AGGREGATE_COUNT;
	readings[0] = 0;
	readings[1] = 0;
	readings[2] = 0;
	sem_post(&readings_sem);

	while(1)
	{
		sem_wait(&readings_sem);
		if(!reading_count)
		{
			x_3 = readings[0]/AGGREGATE_COUNT;
			y_3 = readings[1]/AGGREGATE_COUNT;
			z_3 = readings[2]/AGGREGATE_COUNT;
			sem_post(&readings_sem);
			break;
		}
		sem_post(&readings_sem);
	}
	printf("Thank you\n");
	sleep(1);

	x_0 = (x_1 + x_2) / 2;
	y_0 = (y_1 + y_3) / 2;
	z_0 = (z_2 + z_3) / 2;

	printf("Calibration finished\n");
	printf("x_0 = %f; x_1 = %f; x_2 = %f; x_3 = %f;\n",x_0,x_1,x_2,x_3);
	printf("y_0 = %f; y_1 = %f; y_2 = %f; y_3 = %f;\n",y_0,y_1,y_2,y_3);
	printf("z_0 = %f; z_1 = %f; z_2 = %f; z_3 = %f;\n",z_0,z_1,z_2,z_3);

	cwiid_set_mesg_callback(device,NULL);
}

void fill_acc_readings(cwiid_wiimote_t* device,int cnt,union cwiid_mesg messages[], struct timespec * timestamp)
{
	static uint8_t a_pressed = 0;

	sem_wait(&readings_sem);
	if(!reading_count)
	{
		a_pressed = 0;
		sem_post(&readings_sem);
		return;
	}
	sem_post(&readings_sem);

	int i;

	for(i = 0;i < cnt;++i)
	{
		//A pressed
		if(messages[i].type == CWIID_MESG_BTN && messages[i].btn_mesg.buttons & CWIID_BTN_A)
		{
			a_pressed = 1;
		}

		if(a_pressed && messages[i].type == CWIID_MESG_ACC)
		{
			sem_wait(&readings_sem);
			--reading_count;
			readings[0] += messages[i].acc_mesg.acc[0];
			readings[1] += messages[i].acc_mesg.acc[1];
			readings[2] += messages[i].acc_mesg.acc[2];
			sem_post(&readings_sem);
		}
	}
}


void get_acceleration(uint16_t x,uint16_t y,uint16_t z,float* acceleration)
{
	static long count = 0;
	static float aggX = 0;
	static float aggY = 0;
	static float aggZ = 0;

	if(count >= AGGREGATE_COUNT)
	{
		aggX -= aggX / AGGREGATE_COUNT;
		aggY -= aggY / AGGREGATE_COUNT;
		aggZ -= aggZ / AGGREGATE_COUNT;
	}

	++count;

	aggX += x;
	aggY += y;
	aggZ += z;

	if(count > AGGREGATE_COUNT)
	{
		acceleration[0] = (aggX / AGGREGATE_COUNT - x_0)/(x_3 - x_0);
		acceleration[1] = (aggY / AGGREGATE_COUNT - y_0)/(y_2 - y_0);
		acceleration[2] = (aggZ / AGGREGATE_COUNT - z_0)/(z_1 - z_0);
	}
}

void get_orientation(uint16_t y,uint16_t x,uint16_t z,float* orientation)
{
	static long count = 0;
	static float aggX = 0;
	static float aggY = 0;
	static float aggZ = 0;
	float tmp[3];

	if(count >= AGGREGATE_COUNT)
	{
		aggX -= aggX / AGGREGATE_COUNT;
		aggY -= aggY / AGGREGATE_COUNT;
		aggZ -= aggZ / AGGREGATE_COUNT;
	}

	++count;

	aggX += x;
	aggY += y;
	aggZ += z;

	if(count > AGGREGATE_COUNT)
	{
		get_acceleration(aggX / AGGREGATE_COUNT,aggY / AGGREGATE_COUNT,aggZ / AGGREGATE_COUNT,orientation);

		float normalize_factor = 1 / (fabs(orientation[0]) + fabs(orientation[1]) + fabs(orientation[2]));
//		orientation[0] = asin(M_PI/2*orientation[0]*normalize_factor);
//		orientation[1] = asin(-1*M_PI/2*orientation[1]*normalize_factor);
//		orientation[2] = asin(M_PI/2*orientation[2]*normalize_factor);
		tmp[0] = (/*M_PI/2**/orientation[0]*normalize_factor);
		tmp[1] = (/*M_PI/2**/orientation[1]*normalize_factor);
		tmp[2] = (/*M_PI/2**/orientation[2]*normalize_factor);

		//printf("Forces: %f %f %f\n",tmp[0],tmp[1],tmp[2]);
		if(fabs(tmp[2]) > IGNORE_LIMIT)
		{
			orientation[0] = atan(tmp[1]/tmp[2]);
		}
		else
		{
			//orientation[0] = M_PI/2;
		}

		if(tmp[2] < 0)
		{
			if(tmp[1] > 0)
			{
				orientation[0] = M_PI + orientation[0];
			}
			else
			{
				orientation[0] = -1*M_PI + orientation[0];
			}
		}

		if(fabs(tmp[2]) > IGNORE_LIMIT)
		{
			orientation[1] = atan(tmp[0]/tmp[2]);
		}
		else
		{
			//orientation[1] = M_PI/2;
		}

		if(tmp[2] < 0)
		{
			if(tmp[0] > 0)
			{
				orientation[1] = M_PI + orientation[1];
			}
			else
			{
				orientation[1] = -1*M_PI + orientation[1];
			}
		}

		orientation[1] *= -1;

		if(fabs(tmp[1]) > IGNORE_LIMIT)
		{
			orientation[2] = atan(tmp[0]/tmp[1]);
		}
		else
		{
			//orientation[2] = M_PI/2;
		}

		orientation[2] = atan(tmp[0]/tmp[1]);

		if(tmp[1] < 0)
		{
			if(tmp[0] > 0)
			{
				orientation[2] = M_PI + orientation[2];
			}
			else
			{
				orientation[2] = -1*M_PI + orientation[2];
			}
		}

		orientation[2] *= -1;

		orientation[0] = limit(orientation[0],-1*M_PI/2,M_PI/2);
		orientation[1] = limit(orientation[1],-1*M_PI/2,M_PI/2);
		orientation[2] = limit(orientation[2],-1*M_PI/2,M_PI/2);
	}
}

void get_position(float ir[CWIID_IR_SRC_COUNT][2])
{
	int i;
	printf("Distance: %.4f\t",sqrt(pow(ir[0][0] - ir[1][0],2) + pow(ir[0][1] - ir[1][1],2)));
	//printf("Distance: %.4f\t",fabs(ir[0][0] - ir[1][0]));
	for(i = 0;i < CWIID_IR_SRC_COUNT;++i)
	{
		//printf(" (%d:%d - %d)",messages[i].ir_mesg.src[j].pos[0],messages[i].ir_mesg.src[j].pos[1],messages[i].ir_mesg.src[j].size);
	//	printf(" (%f:%f)",ir[i][0],ir[i][1]);
	}
	printf("\n");
	//printf("Distance: %f\n",sqrt(pow(ir[0][0] - ir[1][0],2) + pow(ir[0][1] - ir[1][1],2)));
	//printf("\n");
}
