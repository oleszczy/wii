#ifndef _SERVER_STRUCTS_H
#define	_SERVER_STRUCTS_H

#include "wiimote_structs.h"

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
};

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

#endif	/* _SERVER_STRUCTS_H_ */

