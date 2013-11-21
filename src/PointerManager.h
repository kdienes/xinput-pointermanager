#include <unordered_map>
#include <string>

#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xrandr.h>
#include <X11/X.h>

class PointerManager
{
 public:

  struct DeviceTriplet {
    XIDeviceInfo *master;
    XIDeviceInfo *slave;
    XIDeviceInfo *floating;
    DeviceTriplet () : master (NULL), slave (NULL), floating (NULL) { }
  };

 protected:

  int m_nsizes;
  XRRScreenSize *m_sizes;

  std::unordered_map <std::string, DeviceTriplet> m_devices;
  Display *m_display;

 public:

  PointerManager (Display *display);
  void QueryDevices (std::string prefix, bool calibrateAll);
  void RemoveMaster (XIDeviceInfo *d);
  void Calibrate (int deviceid);
  void Attach (int slave, int master);
};
