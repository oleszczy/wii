#include "wiimote.h"
#include "wiimoteRegistry.h"

wiimote::wiimote()
{
    reading_count = 0;
    readings[0] = 0;
    readings[1] = 0;
    readings[2] = 0;
    isConnected = false;
    isCalibrated = false;

    sem_init(&sem,0,1);
    sem_init(&readings_sem,0,1);
}

bool wiimote::connect(char* address)
{
    disconnect();
    
    //Adres urzadzenia
    bdaddr_t btaddress;
    str2ba(address,&btaddress);

    printf("Please press 1 and 2 buttons simultaneously\n");
    device = cwiid_connect(&btaddress,CWIID_FLAG_MESG_IFC);

    if(!device)
    {
        fprintf(stderr,"Unable to connect to %s\n",address);
        return isConnected;
    }

    printf("Connected to %s\n",address);

    cwiid_set_mesg_callback(device,global_msg_callback);
    cwiid_command(device,CWIID_CMD_RPT_MODE,REP_MODE);

    isConnected = true;
    isCalibrated = false;
    wiimoteRegistry::addWiimote(device,this);
    return isConnected;
}

void wiimote::msg_callback(cwiid_wiimote_t* device,int cnt,union cwiid_mesg messages[], struct timespec * timestamp)
{
	int i,j;
	float acc_readings[3];
	float ir_readings[CWIID_IR_SRC_COUNT][2];

	for(i = 0;i < cnt;++i)
	{
		switch(messages[i].type)
		{
			case CWIID_MESG_STATUS:
				printf("Status message: Battery -  %d%%",(int)((float)messages[i].status_mesg.battery/CWIID_BATTERY_MAX*100));
				if(messages[i].status_mesg.ext_type & CWIID_EXT_NUNCHUK) printf(" - Nunchuk connected");
				printf("\n");
				break;
			case CWIID_MESG_ERROR:
				printf("Error message: ");
				switch(messages[i].error_mesg.error)
				{
					case CWIID_ERROR_DISCONNECT:
						fprintf(stderr,"Disconnected\n");
                                                wiimoteRegistry::removeWiimote(device);
                                                disconnect();
						break;
					case CWIID_ERROR_COMM:
						fprintf(stderr,"Communication error\n");
						break;
					default:
						fprintf(stderr,"Unknown\n");
						break;
				}
				break;
			case CWIID_MESG_BTN:
				printf("Button message:");
                                sem_wait(&sem);
				if(messages[i].btn_mesg.buttons & CWIID_BTN_1)
                                {
                                    printf(" 1");
                                    buttons.button1 = true;
                                }
                                else
                                {
                                    buttons.button1 = false;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_2)
                                {
                                    printf(" 2");
                                    buttons.button2 = true;
                                }
                                else
                                {
                                    buttons.button2 = false;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_A)
                                {
                                    printf(" A");
                                    buttons.buttonA = true;
                                }
                                else
                                {
                                    buttons.buttonA = false;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_B)
                                {
                                    printf(" B");
                                    buttons.buttonB = true;
                                }
                                else
                                {
                                    buttons.buttonB = false;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_MINUS)
                                {
                                    printf(" -");
                                    buttons.buttonMinus = true;
                                }
                                else
                                {
                                    buttons.buttonMinus = false;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_PLUS)
                                {
                                    printf(" +");
                                    buttons.buttonPlus = true;
                                }
                                else
                                {
                                    buttons.buttonPlus = false;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_HOME)
                                {
                                    printf(" H");
                                    buttons.buttonHome = true;
                                }
                                else
                                {
                                    buttons.buttonHome = false;
                                }
				if(messages[i].btn_mesg.buttons & CWIID_BTN_LEFT)
				{
					printf(" LEFT");
					buttons.left = true;
				}
				else
				{
					buttons.left = false;
				}
				if(messages[i].btn_mesg.buttons & CWIID_BTN_RIGHT)
				{
					printf(" RIGHT");
					buttons.right = true;
				}
				else
				{
					buttons.right = false;
				}
				if(messages[i].btn_mesg.buttons & CWIID_BTN_UP)
				{
					printf(" UP");
					buttons.up = true;
				}
				else
				{
					buttons.up = false;
				}
				if(messages[i].btn_mesg.buttons & CWIID_BTN_DOWN)
				{
					printf(" DOWN");
					buttons.down = true;
				}
				else
				{
					buttons.down = false;
				}
				sem_post(&sem);

				printf("\n");
				break;
			case CWIID_MESG_ACC:
				sem_wait(&sem);
				//printf("MAM: X:%d Y:%d Z:%d\n",messages[i].acc_mesg.acc[0],messages[i].acc_mesg.acc[1],messages[i].acc_mesg.acc[2]);
				get_orientation(messages[i].acc_mesg.acc[0],messages[i].acc_mesg.acc[1],messages[i].acc_mesg.acc[2],acc_readings);
				orientation.orientation_x = acc_readings[0];
				orientation.orientation_y = acc_readings[1];
				orientation.orientation_z = acc_readings[2];
				sem_post(&sem);
				//printf("Angles: X:%f Y:%f Z:%f\n",acc_readings[0],acc_readings[1],acc_readings[2]);
				break;
			case CWIID_MESG_IR:
                                printf("IR message\n");
				break;
			case CWIID_MESG_NUNCHUK:
				printf("Nunchuck message\n");
				break;
			case CWIID_MESG_CLASSIC:
				printf("Classic message\n");
				break;
			case CWIID_MESG_UNKNOWN:
				printf("Unknown message\n");
				break;
			default:
				printf("Oh dear...\n");
				break;
		}
	}
}

void wiimote::get_acceleration(uint16_t x,uint16_t y,uint16_t z,float* acceleration)
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
		acceleration[0] = (aggX / AGGREGATE_COUNT - calibration[0][0])/(calibration[0][3] - calibration[0][0]);
		acceleration[1] = (aggY / AGGREGATE_COUNT - calibration[1][0])/(calibration[1][2] - calibration[1][0]);
		acceleration[2] = (aggZ / AGGREGATE_COUNT - calibration[2][0])/(calibration[2][1] - calibration[2][0]);
	}
}

void wiimote::get_orientation(uint16_t y,uint16_t x,uint16_t z,float* orientation)
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

float wiimote::limit(float value,float min,float max)
{
	if(value < min)
	{
		return min;
	}

	if(value > max)
	{
		return max;
	}

	return value;
}

void wiimote::precalibrate(float calibration[3][4])
{
	for(int i = 0;i < 3;++i)
	{
		for(int j = 0;j < 4;++j)
		{
		  this->calibration[i][j] = calibration[i][j];
		}
	}

    isCalibrated = true;
}

float** wiimote::calibrate()
{
    cwiid_command(device,CWIID_CMD_RPT_MODE,REP_MODE);
    cwiid_set_mesg_callback(device,global_fill_acc_readings);

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
                    calibration[0][1] = readings[0]/AGGREGATE_COUNT;
                    calibration[1][1] = readings[1]/AGGREGATE_COUNT;
                    calibration[2][1] = readings[2]/AGGREGATE_COUNT;
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
                    calibration[0][2] = readings[0]/AGGREGATE_COUNT;
                    calibration[1][2] = readings[1]/AGGREGATE_COUNT;
                    calibration[2][2] = readings[2]/AGGREGATE_COUNT;
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
                    calibration[0][3] = readings[0]/AGGREGATE_COUNT;
                    calibration[1][3] = readings[1]/AGGREGATE_COUNT;
                    calibration[2][3] = readings[2]/AGGREGATE_COUNT;
                    sem_post(&readings_sem);
                    break;
            }
            sem_post(&readings_sem);
    }
    printf("Thank you\n");
    sleep(1);

    calibration[0][0] = (calibration[0][1] + calibration[0][2]) / 2;
    calibration[1][0] = (calibration[1][1] + calibration[1][3]) / 2;
    calibration[2][0] = (calibration[2][2] + calibration[2][3]) / 2;

    printf("Calibration finished\n");
    printf("x_0 = %f; x_1 = %f; x_2 = %f; x_3 = %f;\n",calibration[0][0],calibration[0][1],calibration[0][2],calibration[0][3]);
    printf("y_0 = %f; y_1 = %f; y_2 = %f; y_3 = %f;\n",calibration[1][0],calibration[1][1],calibration[1][2],calibration[1][3]);
    printf("z_0 = %f; z_1 = %f; z_2 = %f; z_3 = %f;\n",calibration[2][0],calibration[2][1],calibration[2][2],calibration[2][3]);

    cwiid_set_mesg_callback(device,global_msg_callback);
    cwiid_command(device,CWIID_CMD_RPT_MODE,REP_MODE);

    isCalibrated = true;
}

void wiimote::fill_acc_readings(cwiid_wiimote_t* device,int cnt,union cwiid_mesg messages[], struct timespec * timestamp)
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

void global_msg_callback(cwiid_wiimote_t* device,int cnt,union cwiid_mesg messages[], struct timespec * timestamp)
{
    wiimote* wii = wiimoteRegistry::getWiimote(device);
    if(wii)
    {
        wii->msg_callback(device,cnt,messages,timestamp);
    }
}

void global_fill_acc_readings(cwiid_wiimote_t* device,int cnt,union cwiid_mesg messages[], struct timespec * timestamp)
{
    wiimote* wii = wiimoteRegistry::getWiimote(device);
    if(wii)
    {
        wii->fill_acc_readings(device,cnt,messages,timestamp);
    }
}

void wiimote::setRumble(bool on)
{
    if(on)
    {
        cwiid_command(device,CWIID_CMD_RUMBLE,1);
    }
    else
    {
        cwiid_command(device,CWIID_CMD_RUMBLE,0);
    }
}

void wiimote::setLeds(int leds)
{
    cwiid_command(device,CWIID_CMD_LED,leds);
}

void wiimote::disconnect()
{
    if(device)
    {
        cwiid_disconnect(device);
        isConnected = false;
    }
}

buttonStatus wiimote::getButtons()
{
    sem_wait(&sem);
    buttonStatus buttonsCopy = buttons;
    sem_post(&sem);
    return buttonsCopy;
}

orientationStatus wiimote::getOrientation()
{
    sem_wait(&sem);
    orientationStatus orientationCopy = orientation;
    sem_post(&sem);
    return orientationCopy;
}

bool wiimote::getIsConnected()
{
    return isConnected;
}

bool wiimote::getIsCalibrated()
{
    return isCalibrated;
}
