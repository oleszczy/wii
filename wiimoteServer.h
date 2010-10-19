#ifndef _WIIMOTESERVER_H
#define	_WIIMOTESERVER_H

#include "wiimote.h"
#include "server_structs.h"

/**
 * Listens for connections and provides device readings over tcp/ip
 */
class wiimoteServer
{
public:

    /**
     * Initializes server
     */
    wiimoteServer();

    /**
     * Initializes server and associates it with given wiimote
     *
     * @param wiimote* wii wiimote object
     */
    wiimoteServer(wiimote* wii);

    /**
     * Starts server
     *
     * @param int port port number to listen for connections
     */
    void run(int port);

    /**
     * Stops server
     */
    void stop();

    /**
     * Returns server status
     *
     * @return bool true if the server is running, false otherwise
     */
    bool getIsRunning();

    /**
     * Associates server with given wiimote device
     *
     * @param wiimote* wii wiimote object
     */
    void setWiimote(wiimote* wii);

private:
    /**
     * Private copy constructor to prevent copying
     */
    wiimoteServer(const wiimoteServer& orig);

    /**
     * Wiimote instance
     */
    wiimote* wii;

    /**
     * Reply struct
     */
    struct VSP_ECP_MSG reply;
    sem_t sem;
    bool isRunning;
    bool stopServer;
};

#endif	/* _WIIMOTESERVER_H */

