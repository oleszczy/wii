#include <stdlib.h>
#include "wiimote.h"
#include "wiimoteServer.h"

//variables needed for counting the device's orientation
//their values is counted during the calibration phase
//float x_0,x_1,x_2,x_3,y_0,y_1,y_2,y_3,z_0,z_1,z_2,z_3;
float x_0 = 124.500000; float x_1 = 125.000000; float x_2 = 124.000000; float x_3 = 149.000000;
float y_0 = 124.000000; float y_1 = 125.000000; float y_2 = 149.000000; float y_3 = 123.000000;
float z_0 = 127.000000; float z_1 = 152.000000; float z_2 = 126.000000; float z_3 = 128.000000;

#define PORT 40666

int main(int argc, char** argv)
{
    if(argc < 2)
    {
            fprintf(stderr,"USAGE: %s adres\n",argv[0]);
            return EXIT_FAILURE;
    }

    

    float calibration[3][4] = {{x_0,x_1,x_2,x_3},{y_0,y_1,y_2,y_3},{z_0,z_1,z_2,z_3}};

    wiimote* wii = new wiimote();

    if(!(wii->connect(argv[1])))
    {
        return EXIT_FAILURE;
    }

    wii->precalibrate(calibration);
    //wii->calibrate();

    wiimoteServer* server = new wiimoteServer(wii);
    server->run(PORT);
    
    return (EXIT_SUCCESS);
}

