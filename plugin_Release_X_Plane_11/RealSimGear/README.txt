RealSimGear Hardware Connector
Version 1.3.5.0
==============================


System requirements
-------------------

For all operating systems:
    
	- X-Plane 10 (32-bit or 64-bit) or X-Plane 11

For specific operating systems:

    - Microsoft Windows
        - Microsoft Visual C++ Redistributable for Visual Studio 2017 x86
          https://aka.ms/vs/15/release/vc_redist.x64.exe
        - Microsoft Visual C++ Redistributable for Visual Studio 2017 x64
          https://aka.ms/vs/15/release/vc_redist.x64.exe

    - Mac OS 10.14
	    - no additional software required

    - Linux x64
	    - no additional software required
		- tested on Ubuntu 18.04 LTS


Installation
------------

Copy the appropriate subfolder of this folder to the "Resources\plugins" 
subdirectory of your X-Plane 10 or 11 installation directory.


Serial port
-----------

The plugin automatically monitors all (virtual) serial ports available on
the system and detects the hardware when it is connected. No configuration
is required. Up to 5 hardware units can be connected simultaneously.

The hardware can be safely connected and disconnected while X-Plane is running.


Command mapping
---------------

Command mapping is performed using the CommandMapping.ini file in the 
plugin directory OR the root of the aircraft folder. It contains a section for eac
hardware model and id.  Each section header is of the form [<hardware model>#<id>].

The RealSimGear plugin will first search the root of your loaded aircraft folder for
CommandMapping.ini that may be specific for your aircraft.  If it does not locate an
aircraft specific file, it will load the CommandMapping.ini file from the root of the
RealSimGear plugin folder.  This allows you to customize the hardware mappings for
specific aircraft.

There are four variations of command/dataref mappings that can be used with the plugin
as defined below.  This allows for flexibility in manipulating the RealSimGear hardware
in conjuntion with various aircraft.  For buttons and knobs, the RealSimGear hardware 
transmits a "command" when a button is pushed or knob turned, those command are then acted
on in one of the following manners:
1) Run a basic X-Plane command.  The format of this entry in the CommandMapping.ini file is:
     sim/GPS/g1000n3_nav=sim/autopilot/NAV
  - The effect of this entry, when the command "sim/GPS/g1000n3_nav" is received from 
     the RSG hardware, the plugin will execute the X-Plane command "sim/autopilot/NAV"

2) Run a command after evaluating the state of a dataref.  The format of this entry
     in the CommandMapping.ini file is:
     sim/GPS/g1000n3_nose_up@sim/cockpit2/autopilot/altitude_mode#4=sim/autopilot/vertical_speed_up
  - The effect of this entry, when the command "sim/GPS/g1000n3_nose_up" is received from
     the RSG hardware, the plugin will first evalaute the dataref "sim/cockpit2/autopilot/altitude_mode"
     and compare the current value to the value provided in the entry, "4".  If they match,
     the plugin will then execute the X-Plane command "sim/autopilot/vertical_speed_up"

3) Only set a dataref.  The format of this entry in the CommandMapping.ini file is:
     BTN_FMS=sim/panel/FMS_Panel_Mode|4
  - The effect of this entry, when the command "BTN_FMS" is received from the RSG hardware,
     the plugin will set the dataref "sim/panel/FMS_Panel_Mode" to the value of "4".

4) Set a dataref after evaluating the state of a dataref.  The format of this entry in
     the CommandMapping.ini file is:
     BTN_FPL@sim/panel/MFD_Status#0=sim/panel/MFD_Status|30
  - The effect of this entry, when the command "BTN_FPL" is received from the RSG hardware,
     the plugin will first evaluate the dataref "sim/panel/MFD_Status" and compare the current
     value to the value provided in the entry, "0".  If they match, the plugin will then set the
     dataref "sim/panel/MFD_Status" to the value of "30".

Note that there are delimiter used in the following manner:
     @ - Separates the inbound RSG command from the conditional dataref
     # - Separates the conditional dataref between the dataref name and the value to check
     = - Separates the inbound command and/or conditional dataref paring from the "action" to be taken
     | - Denotes that the "action" is to set a dataref, and separates the dataref name from
         the value to be set

Of note, the plugin currently only allow mapping button "press" therefore it is not possible to
act on button release (or turning a switch off).  Support coming in a future release.

A LED mapping is of the form <"n"="x-plane dataref"#"value" where
"N" is the index of the LED as sent to the hardware, "x-plane dataref"
is the X-Plane DataRef to query and "value" is the integer value of the X-Plane
DataRef that will cause the LED to become active.
Note that a LED mapping line starts with a "smaller than" (<) character.

The plugin must be disabled an re-enabled using the X-Plane plugin manager
for the command mapping to be reloaded from the configuration file.


Logging
-------

Important message are written to the X-Plane log file Log.txt in the
X-Plane installation directory.

Detailed trace information can be captured using the Microsoft DebugView 
utility from https://docs.microsoft.com/en-us/sysinternals/downloads/debugview.

