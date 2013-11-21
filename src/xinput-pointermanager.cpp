#include "PointerManager.h"

#include "xinput-pointermanager-options.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <X11/extensions/XInput2.h>
#include <X11/extensions/Xrandr.h>
#include <X11/X.h>

int main (int argc, char *argv[])
{
  gengetopt_args_info args_info;

  if (cmdline_parser (argc, argv, &args_info) != 0)
    exit (EXIT_FAILURE);

  Display *display = XOpenDisplay (NULL);
  if (display == NULL) {
    printf ("unable to connect");
    return -1;
  }

  int xi_opcode, event, error;
  if (! XQueryExtension (display, "XInputExtension", &xi_opcode, &event, &error)) {
    printf ("X Input extension not available.\n");
    return -1;
  }

  /* Which version of XI2? We support 2.0 */
  int major = 2, minor = 0;
  if (XIQueryVersion (display, &major, &minor) == BadRequest) {
    printf("XI2 not available. Server supports %d.%d\n", major, minor);
    return -1;
  }

  if (! XRRQueryExtension (display, &event, &error)) {
    printf ("X randr extension not available.\n");
  }

  PointerManager m (display);

  m.QueryDevices (args_info.prefix_arg, true);

  XIEventMask evmask;
  unsigned char mask[2] = { 0, 0 };

  XISetMask (mask, XI_HierarchyChanged);
  evmask.deviceid = XIAllDevices;
  evmask.mask_len = sizeof (mask);
  evmask.mask = mask;

  XISelectEvents (display, DefaultRootWindow (display), &evmask, 1);

  for (;;) {

    XEvent ev;
    XNextEvent (display, &ev);
  
    if (ev.xcookie.type != GenericEvent) { continue; }
    if (ev.xcookie.extension != xi_opcode) { continue; }
    
    XGetEventData (display, &ev.xcookie);
    
    switch (ev.xcookie.type) {
    case XI_HierarchyChanged:
      m.QueryDevices (args_info.prefix_arg, false);
      break;
    default:
      /* I can't get the opcode right. */
      m.QueryDevices (args_info.prefix_arg, false);
      break;
    }

    XFreeEventData (display, &ev.xcookie);
  }
}
