#ifndef _WIIMOTE_H
#define	_WIIMOTE_H

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
#include <semaphore.h>
#include <math.h>

#include <bluetooth/bluetooth.h>
#include "cwiid.h"

#include "wiimote_structs.h"


/**
 * Controller reporting mode. 
 * Data reported by the controller depend on combination fo the following flags:
 *
 * CWIID_RPT_STATUS - report device status
 * CWIID_RPT_BTN    - report button status
 * CWIID_RPT_ACC    - report accelerometer readings
 * CWIID_RPT_IR     - report IR sensor readings
 *
 * Button status and accelerometer readings is what we need
 */
#define REP_MODE (CWIID_RPT_ACC | CWIID_RPT_BTN)

#define AGGREGATE_COUNT 5
#define IGNORE_LIMIT 1e-3


/**
 * Represents wiimote device and provides device readings
 */
class wiimote
{
public:
    /**
     * Initializes wiimote object
     */
    wiimote();

    /**
     * Connects to the device at given address
     *
     * @param char* address device address
     *
     * @return bool true if connected, false otherwise
     */
    bool connect(char* address);

    /**
     * Disconnect from the device
     */
    void disconnect();

    /**
     * Perform device calibration
     *
     * @return float** array of floats that could be passed to precalibrate()
     *                 function in order to skip calibration process next time     *
     */
    float** calibrate();

    /**
     * Skip device calibration, using calibrated values from previous
     * runs of calibrate()
     *
     * @param float[3][4] array of floats returned from previous runs of calibrate()
     */
    void precalibrate(float calibration[3][4]);

    /**
     * Turn the rumble on/off
     *
     * @param bool on true to turn the rumble on, false otherwise
     */
    void setRumble(bool on);

    /**
     * Turn leds on/off
     *
     * @param int leds desired led status - if any of the lowest 4 bits are set,
     *                 the corresponding leds will be turned on, with the least
     *                 significant bit corresponding to the first (left) led
     */
    void setLeds(int leds);

    /**
     * Get connection status
     *
     * @return bool true, if device is connected, false otherwise
     */
    bool getIsConnected();

    /**
     * Get calibration status
     *
     * @return bool true, if device is calibrated, false otherwise
     */
    bool getIsCalibrated();

    /**
     * Get last received button status
     *
     * @return buttonStatus button status
     */
    buttonStatus getButtons();

    /**
     * Get last read device orientation
     *
     * @return orientationStatus device orientation
     */
    orientationStatus getOrientation();

    /**
     * Callbacks functions. Must be public so that external callback can call it.
     * msg_callback is called every time new reading is available.
     * fill_acc_readings is called during calibration process.
     *
     * @param cwiid_wiimote_t* device       device object
     * @param int              cnt          number of messages
     * @param union cwiid_mesg messages     messages
     * @param struct timespec* timestamp    timestamps
     */
    void msg_callback(cwiid_wiimote_t* device,int cnt,union cwiid_mesg messages[], struct timespec * timestamp);
    void fill_acc_readings(cwiid_wiimote_t* device,int cnt,union cwiid_mesg messages[], struct timespec * timestamp);

protected:
    /**
     * Fills given array with each axis acceleration determined from given accelerometer readings
     *
     * @param uint16_t x            x-axis reading
     * @param uint16_t y            y-axis reading
     * @param uint16_t z            z-axis reading
     * @param float*   acceleration each axis acceleration will be stored here
     */
    void get_acceleration(uint16_t x,uint16_t y,uint16_t z,float* acceleration);

    /**
     * Fills given array with device orientation determined from each axis acceleration
     *
     * @param uint16_t x           x-axis acceleration
     * @param uint16_t y           y-axis acceleration
     * @param uint16_t z           z-axis acceleration
     * @param float*   orientation device orientation (angle)
     */
    void get_orientation(uint16_t y,uint16_t x,uint16_t z,float* orientation);

private:
    /**
     * Copy constructor to prevent object copying to prevent segmentation
     * faults when two object try to communicate with the same device
     *
     * @param wiimote orig original wiimote object
     */
    wiimote(const wiimote& orig);

    /**
     * Helper function to limit given value
     *
     * @param float value value to be bound
     * @param float min   the minimum value function will return
     * @param float max   the maximum value function will return
     */
    float limit(float value,float min,float max);    

    /**
     * Device last read button status
     */
    buttonStatus buttons;

    /**
     * Device last read orientation
     */
    orientationStatus orientation;
    
    sem_t sem;
    sem_t readings_sem;
    uint16_t reading_count;
    uint16_t readings[3];
    float calibration[3][4];
    cwiid_wiimote_t* device;
    bool isConnected;
    bool isCalibrated;
};

void global_msg_callback(cwiid_wiimote_t* device,int cnt,union cwiid_mesg messages[], struct timespec * timestamp);
void global_fill_acc_readings(cwiid_wiimote_t* device,int cnt,union cwiid_mesg messages[], struct timespec * timestamp);

#endif	/* _WIIMOTE_H */

