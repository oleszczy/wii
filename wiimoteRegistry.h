#ifndef _WIIMOTEREGISTRY_H
#define	_WIIMOTEREGISTRY_H

#include <map>

#include "cwiid.h"
#include "wiimote.h"

/**
 * Registry storing mapping between libcwiid devices and wiimote objects
 */
class wiimoteRegistry
{
public:
    /**
     * Map given libcwiid device with given wiimote object
     *
     * @param cwiid_wiimote_t* device libcwiid wiimote device
     * @param wiimote*         wii    wiimote object
     */
    static void addWiimote(cwiid_wiimote_t* device,wiimote* wii);

    /**
     * Return wiimote object associated with given libcwiid device
     *
     * @param cwiid_wiimote_t* device libcwiid wiimote device
     *
     * @return wiimote* wiimote object
     */
    static wiimote* getWiimote(cwiid_wiimote_t* device);

    /**
     * Remove wiimote object associated with given libcwiid device from the registry
     *
     * @param cwiid_wiimote_t* device libcwiid wiimote device
     */
    static void removeWiimote(cwiid_wiimote_t* device);

private:
    static std::map<cwiid_wiimote_t*,wiimote*> registry;
};

#endif	/* _WIIMOTEREGISTRY_H */

