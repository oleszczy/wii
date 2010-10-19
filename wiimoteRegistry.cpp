#include "wiimoteRegistry.h"

void wiimoteRegistry::addWiimote(cwiid_wiimote_t* device,wiimote* wii)
{
    wiimoteRegistry::registry[device] = wii;
}

wiimote* wiimoteRegistry::getWiimote(cwiid_wiimote_t* device)
{
    return wiimoteRegistry::registry[device];
}

void wiimoteRegistry::removeWiimote(cwiid_wiimote_t* device)
{
    wiimoteRegistry::registry.erase(device);
}

std::map<cwiid_wiimote_t*,wiimote*> wiimoteRegistry::registry;