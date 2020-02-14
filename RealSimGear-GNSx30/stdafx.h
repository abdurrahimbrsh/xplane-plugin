#pragma once

#if defined(IBM)

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>

#elif defined(__linux__) || defined(__APPLE__)

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <dlfcn.h>

#endif

#if defined(__linux__)
#include <GL/gl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>
#endif

#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <memory>
#include <thread>
#include <chrono>
#include <functional>
#include <vector>
#include <list>
#include <map>
#include <atomic>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <cmath>
#include <tuple>

#if defined(__APPLE__)
#include <experimental/optional>
namespace std {
    using std::experimental::optional;
}
#else
#include <optional>
#endif


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>

#include "XPLMDisplay.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPLMProcessing.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPLMPlanes.h"

