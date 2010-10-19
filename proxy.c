#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <bluetooth/bluetooth.h>
#include "cwiid.h"

#include <semaphore.h>

#include "orientation.h"

//Do wyboru:
//CWIID_RPT_STATUS
//CWIID_RPT_BTN
//CWIID_RPT_ACC
//CWIID_RPT_IR
//CWIID_RPT_NUNCHUK
//CWIID_RPT_CLASSIC
//CWIID_RPT_EXT
#define REP_MODE (CWIID_RPT_ACC | CWIID_RPT_BTN)

#define PORT 40666

sem_t sem;

struct buttonStatus
{
	//0 - not pressed
	//1 - pressed
	int left;
	int right;
	int up;
	int down;
        int buttonA;
        int buttonB;
        int button1;
        int button2;
        int buttonPlus;
        int buttonMinus;
        int buttonHome;
} buttons;

struct orientationStatus
{
	float orientation_x;
	float orientation_y;
	float orientation_z;
} orientation;

enum VSP_REPORT
{
	VSP_REPLY_OK
};

enum VSP_COMMAND
{
	VSP_CONFIGURE_SENSOR,
	VSP_INITIATE_READING,
	VSP_GET_READING,
	VSP_TERMINATE
};

struct VSP_ECP_MSG
{
	VSP_REPORT vsp_report;
	buttonStatus buttons;
	orientationStatus orientation;
} reply;

struct ECP_VSP_MSG
{
	VSP_COMMAND i_code;
        struct
        {
          bool led_change;
          unsigned int led_status;
          bool rumble;
        } wii_command;
};

float limit(float value,float min,float max)
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

void test_device(cwiid_wiimote_t* device)
{
	sleep(1);
	int i;

	cwiid_command(device,CWIID_CMD_RUMBLE,0);
	cwiid_command(device,CWIID_CMD_LED,0);

	for(i = 0;i < 10;++i)
	{
		cwiid_command(device,CWIID_CMD_RUMBLE,i%2);
		cwiid_command(device,CWIID_CMD_LED,CWIID_LED1_ON);
		usleep(50000);
		cwiid_command(device,CWIID_CMD_LED,CWIID_LED2_ON);
		usleep(50000);
		cwiid_command(device,CWIID_CMD_LED,CWIID_LED3_ON);
		usleep(50000);
		cwiid_command(device,CWIID_CMD_LED,CWIID_LED4_ON);
		usleep(50000);
	}

	cwiid_command(device,CWIID_CMD_RUMBLE,0);
	cwiid_command(device,CWIID_CMD_LED,CWIID_LED1_ON | CWIID_LED4_ON);
}

void msg_callback(cwiid_wiimote_t* device,int cnt,union cwiid_mesg messages[], struct timespec * timestamp)
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
						exit(EXIT_FAILURE);
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
                                    buttons.button1 = 1;
                                }
                                else
                                {
                                    buttons.button1 = 0;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_2)
                                {
                                    printf(" 2");
                                    buttons.button2 = 1;
                                }
                                else
                                {
                                    buttons.button2 = 0;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_A)
                                {
                                    printf(" A");
                                    buttons.buttonA = 1;
                                }
                                else
                                {
                                    buttons.buttonA = 0;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_B)
                                {
                                    printf(" B");
                                    buttons.buttonB = 1;
                                }
                                else
                                {
                                    buttons.buttonB = 0;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_MINUS)
                                {
                                    printf(" -");
                                    buttons.buttonMinus = 1;
                                }
                                else
                                {
                                    buttons.buttonMinus = 0;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_PLUS)
                                {
                                    printf(" +");
                                    buttons.buttonPlus = 1;
                                }
                                else
                                {
                                    buttons.buttonPlus = 0;
                                }
                                if(messages[i].btn_mesg.buttons & CWIID_BTN_HOME)
                                {
                                    printf(" H");
                                    buttons.buttonHome = 1;
                                }
                                else
                                {
                                    buttons.buttonHome = 0;
                                }
				if(messages[i].btn_mesg.buttons & CWIID_BTN_LEFT)
				{
					printf(" LEFT");
					buttons.left = 1;
				}
				else
				{
					buttons.left = 0;
				}
				if(messages[i].btn_mesg.buttons & CWIID_BTN_RIGHT)
				{
					printf(" RIGHT");
					buttons.right = 1;
				}
				else
				{
					buttons.right = 0;
				}
				if(messages[i].btn_mesg.buttons & CWIID_BTN_UP)
				{
					printf(" UP");
					buttons.up = 1;
				}
				else
				{
					buttons.up = 0;
				}
				if(messages[i].btn_mesg.buttons & CWIID_BTN_DOWN)
				{
					printf(" DOWN");
					buttons.down = 1;
				}
				else
				{
					buttons.down = 0;
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
				for(j = 0;j < CWIID_IR_SRC_COUNT;++j)
				{
					ir_readings[j][0] = messages[i].ir_mesg.src[j].pos[0];
					ir_readings[j][1] = messages[i].ir_mesg.src[j].pos[1];
				}
				get_position(ir_readings);
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


/**
 * Program przyjmuje z linii polecen 1 parametr: ciag znakow bedacy adresem urzadzenia wii-lot
 *
 */
int main(int argc,char** argv)
{
	//Adres urzadzenia
	bdaddr_t address;
	//Uchwyt urzadzenia
	cwiid_wiimote_t* device;
        //rumble!!
        bool rumble = false;

	if(argc < 2)
	{
		fprintf(stderr,"USAGE: %s adres\n",argv[0]);
		return EXIT_FAILURE;
	}

	str2ba(argv[1],&address);

	sem_init(&sem,0,1);

	printf("Please press 1 and 2 buttons simultaneously\n");
	device = cwiid_connect(&address,CWIID_FLAG_MESG_IFC);

	if(!device)
	{
		fprintf(stderr,"Unable to connect to %s\n",argv[1]);
		return EXIT_FAILURE;
	}

	printf("Connected to %s\n",argv[1]);

	//calibrate(device);
	cwiid_set_mesg_callback(device,msg_callback);
	cwiid_command(device,CWIID_CMD_RPT_MODE,REP_MODE);

//	while(1)
//	{
//		sleep(1);
//	}
//
//	return EXIT_SUCCESS;

	cwiid_set_mesg_callback(device,msg_callback);
	cwiid_command(device,CWIID_CMD_RPT_MODE,REP_MODE);
	//test_device(device);


	int sockfd,new_fd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	socklen_t sin_size;
	int yes = 1;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);

	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sockfd, 0) == -1) {
		perror("listen");
		exit(1);
	}

	fd_set sockets;
	while(1)
	{
		sin_size = sizeof their_addr;
		printf("[SERVER] Waiting for connection\n");
		if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
			perror("accept");
			continue;
		}

		printf("[SERVER] Got connection from %s\n",inet_ntoa(their_addr.sin_addr));

		FD_ZERO(&sockets);
		FD_SET(new_fd,&sockets);
		int selectValue;
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		int cnt = 0;

		union cwiid_mesg response;

		while(1)
		{
			char Buffer[1024];
			int size;
			ECP_VSP_MSG command;
			memset(Buffer,'\0',1024);
			Buffer[1023] = '\n';

			selectValue = select(FD_SETSIZE,&sockets,(fd_set*)0,(fd_set*)0,&timeout);
			if(selectValue < 0)
			{
				perror("select");
				exit(EXIT_FAILURE);
			}

			//printf("[SERVER] Waiting for data\n");
			if ((size = recv(new_fd,Buffer,1023,0)) == -1) perror("recv");
			else if(!size) break;
			//printf("[SERVER][%05d] Received: %s\n",size,Buffer);
			
			if(size >= sizeof(ECP_VSP_MSG))
			{
				memcpy(&command,Buffer,sizeof(ECP_VSP_MSG));
				switch(command.i_code)
				{
					case VSP_GET_READING:
						sem_wait(&sem);
						reply.buttons = buttons;
						reply.orientation = orientation;
						sem_post(&sem);
						reply.vsp_report = VSP_REPLY_OK;
 					//	printf("Reply: %d %d %d %d %d %d\n",reply.buttons.left,reply.buttons.up,reply.buttons.right,reply.buttons.down,reply.buttons.buttonA,reply.buttons.buttonB);
						send(new_fd,&reply,sizeof(VSP_ECP_MSG),0);
                                                break;
					default:
						reply.vsp_report = VSP_REPLY_OK;
						send(new_fd,&reply,sizeof(VSP_ECP_MSG),0);
				}

                                //parse wii commands
                                if(command.wii_command.rumble)
                                {
                                   if(!rumble)
                                   {
                                       cwiid_command(device,CWIID_CMD_RUMBLE,1);
                                       rumble = true;
                                   }
                                }
                                else
                                {
                                    cwiid_command(device,CWIID_CMD_RUMBLE,0);
                                    rumble = false;
                                }
                                if(command.wii_command.led_change)
                                {
                                    int led_status = 0;
                                    if(command.wii_command.led_status & 0x1) led_status |= CWIID_LED1_ON;
                                    if(command.wii_command.led_status & 0x2) led_status |= CWIID_LED2_ON;
                                    if(command.wii_command.led_status & 0x4) led_status |= CWIID_LED3_ON;
                                    if(command.wii_command.led_status & 0x8) led_status |= CWIID_LED4_ON;
                                    cwiid_command(device,CWIID_CMD_LED,led_status);
                                }
			}
		}
		printf("[SERVER] Connection closed - Unloading all\n");
		close(new_fd);
	}
	close(sockfd);

	return EXIT_SUCCESS;

	cwiid_disconnect(device);
	printf("Bye bye!\n");
}
