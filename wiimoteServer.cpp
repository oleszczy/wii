/* 
 * File:   wiimoteServer.cpp
 * Author: jedrzej
 * 
 * Created on 21 kwiecień 2010, 21:39
 */

#include "wiimoteServer.h"

wiimoteServer::wiimoteServer()
{
    wii = NULL;
    stopServer = false;
    sem_init(&sem,0,1);
}

wiimoteServer::wiimoteServer(wiimote* wii)
{
    this->wii = wii;
    stopServer = false;
    sem_init(&sem,0,1);
}

void wiimoteServer::setWiimote(wiimote* wii)
{
    sem_wait(&sem);
    if(isRunning)
    {
        sem_post(&sem);
        throw "Cannot change device object while server is running";
    }
    else
    {
        sem_post(&sem);
    }
    this->wii = wii;
}

bool wiimoteServer::getIsRunning()
{   
    bool isRunning;
    sem_wait(&sem);
    isRunning = this->isRunning;
    sem_post(&sem);
    return isRunning;

}

void wiimoteServer::run(int port)
{
    sem_wait(&sem);
    if(isRunning)
    {
        sem_post(&sem);
        throw "Server is already running";
    }
    else
    {
        isRunning = true;
        sem_post(&sem);
    }

    stopServer = false;
    
    int sockfd,new_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    socklen_t sin_size;
    int yes = 1;

    bool rumble = false;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
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
    while(wii->getIsConnected())
    {
            sem_wait(&sem);
            if(stopServer)
            {
                isRunning = false;
                stopServer = false;
                sem_post(&sem);
                return;
            }
            else
            {
                sem_post(&sem);
            }

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

            while(wii->getIsConnected())
            {
                    sem_wait(&sem);
                    if(stopServer)
                    {
                        isRunning = false;
                        stopServer = false;
                        sem_post(&sem);
                        return;
                    }
                    else
                    {
                        sem_post(&sem);
                    }
                    char Buffer[128];
                    int size;
                    ECP_VSP_MSG command;
                    memset(Buffer,'\0',128);
                    Buffer[127] = '\n';

                    selectValue = select(FD_SETSIZE,&sockets,(fd_set*)0,(fd_set*)0,&timeout);
                    if(selectValue < 0)
                    {
                            perror("select");
                            exit(EXIT_FAILURE);
                    }

                    //printf("[SERVER] Waiting for data\n");
                    if ((size = recv(new_fd,Buffer,127,0)) == -1) perror("recv");
                    else if(!size) break;
                    //printf("[SERVER][%05d] Received: %s\n",size,Buffer);
                    if(size >= sizeof(ECP_VSP_MSG))
                    {
                            memcpy(&command,Buffer,sizeof(ECP_VSP_MSG));
printf("%d\n",command.i_code);
                            switch(command.i_code)
                            {
                                    case VSP_GET_READING:
                                            reply.buttons = wii->getButtons();
                                            reply.orientation = wii->getOrientation();
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
                                   wii->setRumble(true);
                                   rumble = true;
                               }
                            }
                            else
                            {
                                wii->setRumble(false);
                                rumble = false;
                            }
                            if(command.wii_command.led_change)
                            {
                                int led_status = 0;
                                if(command.wii_command.led_status & 0x1) led_status |= CWIID_LED1_ON;
                                if(command.wii_command.led_status & 0x2) led_status |= CWIID_LED2_ON;
                                if(command.wii_command.led_status & 0x4) led_status |= CWIID_LED3_ON;
                                if(command.wii_command.led_status & 0x8) led_status |= CWIID_LED4_ON;
                                wii->setLeds(led_status);
                            }
                    }
            }
            printf("[SERVER] Connection closed - Unloading all\n");
            close(new_fd);
    }
    close(sockfd);

    sem_wait(&sem);
    isRunning = false;
    stopServer = false;
    sem_post(&sem);
}

void wiimoteServer::stop()
{
    sem_wait(&sem);
    stopServer = true;
    sem_post(&sem);
}
