#ifndef _WIIMOTE_STRUCTS_H
#define	_WIIMOTE_STRUCTS_H

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
};

struct orientationStatus
{
	float orientation_x;
	float orientation_y;
	float orientation_z;
};

#endif	/* _WIIMOTE_STRUCTS_H */

