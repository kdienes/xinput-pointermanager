#include "PointerManager.h"

#include <string.h>
#include <stdint.h>

const bool remaster = true;

PointerManager::PointerManager (Display *display) : m_display (display)
{
  m_sizes = XRRSizes (display, DefaultScreen (display), &m_nsizes);
}

void PointerManager::RemoveMaster (XIDeviceInfo *d)
{
  XIRemoveMasterInfo remove;
  remove.type = XIRemoveMaster;
  remove.deviceid = d->deviceid;
  remove.return_mode = XIAttachToMaster;
  remove.return_pointer = 2;
  remove.return_keyboard = 3;
  XIChangeHierarchy (m_display, (XIAnyHierarchyChangeInfo *) &remove, 1);
}

void PointerManager::Attach (int slave, int master)
{
  XIAttachSlaveInfo attach;
  attach.type = XIAttachSlave;
  attach.deviceid = slave;
  attach.new_master = master;
  XIChangeHierarchy (m_display, (XIAnyHierarchyChangeInfo *) &attach, 1);
}

void PointerManager::QueryDevices (std::string prefix, bool calibrateAll)
{
  int ndevices;
  XIDeviceInfo *devices, *device;

  devices = XIQueryDevice (m_display, XIAllDevices, &ndevices);

  for (int i = 0; i < ndevices; i++) {

    device = &devices[i];

    if (strncmp (device->name, prefix.c_str(), strlen (prefix.c_str ())) != 0) {
      continue;
    }
    if (strstr (device->name, "XTEST") != NULL) {
      continue;
    }
    if (device->use == XIMasterPointer) {
      char *s = rindex (device->name, ' ');
      if (s == NULL) { abort (); }
      if (strncmp (s, " pointer", strlen (" pointer")) != 0) { abort (); }
      *s = '\0';
    }

    if ((device->use == XIMasterKeyboard) || (device->use == XISlaveKeyboard)) { continue; }
    std::string devname (device->name);
    
    auto deviter = m_devices.find (devname);
    if (deviter == m_devices.end ()) { deviter = m_devices.insert (std::make_pair (devname, DeviceTriplet ())).first; }
    auto &dev = deviter->second;

    switch (device->use) {
    case XIMasterPointer: if (dev.master != NULL) { abort (); } else { dev.master = device; } break;
    case XISlavePointer: if (dev.slave != NULL) { abort (); } dev.slave = device; break;
    case XIFloatingSlave: if (dev.floating != NULL) { abort (); } dev.floating = device; break;
    default: abort ();
    }
  }

  for (auto de : m_devices) {

    auto &d = de.second;

    if (calibrateAll) {
      if (d.slave) { Calibrate (d.slave->deviceid); }
      if (d.floating) { Calibrate (d.floating->deviceid); }
    }

    /* Should never happen, but is possible. */
    if ((d.slave != NULL) && (d.floating != NULL)) { abort (); }

    if (remaster && (d.master == NULL)) {

      if ((d.slave == NULL) && (d.floating == NULL)) {
	/* Why do we even have an entry? */
	abort ();
      }
      if ((d.slave != NULL) || (d.floating != NULL)) {
	/* Need to create a master to attach to. */
	if (remaster) {
	  printf ("creating master (\"%s\")\n", de.first.c_str ());
	  XIAddMasterInfo add;
	  add.type = XIAddMaster;
	  add.name = (char *) de.first.c_str ();
	  add.send_core = 1;
	  add.enable = 1;
	  XIChangeHierarchy (m_display, (XIAnyHierarchyChangeInfo *) &add, 1);
	}
      }
    } else {
      
      int master = remaster ? d.master->deviceid : 2;

      if (remaster && (d.slave == NULL) && (d.floating == NULL)) {
	/* Need to delete orphaned master. */
	printf ("removing master %d (\"%s\")\n", master, de.first.c_str ());
	RemoveMaster (d.master);
      }
      if (d.slave != NULL) {
	/* Need to verify ID. */
	if (d.slave->attachment != master) {
#if 0
	  abort ();
#else
	  /* Need to attach to master. */
	  printf ("reattaching %d (\"%s\") from %d to %d\n", d.slave->deviceid, de.first.c_str (), d.slave->attachment, master);
	  Calibrate (d.slave->deviceid);
	  Attach (d.slave->deviceid, master);
#endif
	}
      }
      if (d.floating != NULL) {
	printf ("calibrating and attaching %d (\"%s\") to %d\n", d.floating->deviceid, de.first.c_str (), master);
	/* Need to attach to master. */
	Calibrate (d.floating->deviceid);
	Attach (d.floating->deviceid, master);
      }
    }

  }

  m_devices.clear ();
  XIFreeDeviceInfo (devices);
  XSync (m_display, false);
}

void PointerManager::Calibrate (int deviceid)
{
  printf ("calibrating %d\n", deviceid);

  Atom property = XInternAtom (m_display, "Evdev Axis Calibration", False);
  Atom type = XInternAtom (m_display, "INTEGER", False);

  Window root_window;
  int x, y;
  unsigned int width, height, border_width, depth;
  
  XGetGeometry (m_display, DefaultRootWindow (m_display), &root_window,
		&x, &y, &width, &height, &border_width, &depth);
		
  uint32_t data[4] = { (uint32_t) x, x + width, (uint32_t) y, y + height };

  XIChangeProperty (m_display, deviceid, property, type, 32,
		    PropModeReplace, (unsigned char *) &data, 4);
}
