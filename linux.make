ifeq ($(DEBUG),1)
	CXX_FLAGS = -g 
else
	CXX_FLAGS = -O3	
endif

all: plugin11/RealSimGear-GNSx30/CommandMapping.ini plugin11/RealSimGear-GNSx30/lin_x64/RealSimGear-GNSx30.xpl plugin10/RealSimGear-GNSx30/CommandMapping.ini plugin10/RealSimGear-GNSx30/64/lin.xpl

plugin11/RealSimGear-GNSx30/lin_x64/RealSimGear-GNSx30.xpl: RealSimGear-GNSx30/plugin.cpp RealSimGear-GNSx30/all_serial_watch.h RealSimGear-GNSx30/command_mapping.h RealSimGear-GNSx30/resource.h RealSimGear-GNSx30/serial_watch.h RealSimGear-GNSx30/serial.h RealSimGear-GNSx30/stdafx.h RealSimGear-GNSx30/util.h RealSimGear-GNSx30/gui/box.h RealSimGear-GNSx30/gui/button.h RealSimGear-GNSx30/gui/color.h RealSimGear-GNSx30/gui/gui.h RealSimGear-GNSx30/gui/label.h RealSimGear-GNSx30/gui/line.h RealSimGear-GNSx30/gui/tools.h RealSimGear-GNSx30/gui/widget.h RealSimGear-GNSx30/gui/window.h RealSimGear-GNSx30/gui/circle_button.h linux.make
	mkdir -p plugin11/RealSimGear-GNSx30/lin_x64
	g++ $(CXX_FLAGS) -o plugin11/RealSimGear-GNSx30/lin_x64/RealSimGear-GNSx30.xpl -shared -rdynamic -undefined_warning -lGL -fPIC -ISDK/CHeaders/XPLM -IRealSimGear-GNSx30 -DLIN -DTRACE -DXPLM200 -DXPLM210 -DXPLM300 -DXPLM301 -Wfatal-errors -std=c++17 -pthread RealSimGear-GNSx30/plugin.cpp

plugin10/RealSimGear-GNSx30/64/lin.xpl: RealSimGear-GNSx30/plugin.cpp RealSimGear-GNSx30/all_serial_watch.h RealSimGear-GNSx30/command_mapping.h RealSimGear-GNSx30/resource.h RealSimGear-GNSx30/serial_watch.h RealSimGear-GNSx30/serial.h RealSimGear-GNSx30/stdafx.h RealSimGear-GNSx30/util.h RealSimGear-GNSx30/gui/box.h RealSimGear-GNSx30/gui/button.h RealSimGear-GNSx30/gui/color.h RealSimGear-GNSx30/gui/gui.h RealSimGear-GNSx30/gui/label.h RealSimGear-GNSx30/gui/line.h RealSimGear-GNSx30/gui/tools.h RealSimGear-GNSx30/gui/widget.h RealSimGear-GNSx30/gui/window.h RealSimGear-GNSx30/gui/circle_button.h linux.make
	mkdir -p plugin10/RealSimGear-GNSx30/64
	g++ $(CXX_FLAGS) -o plugin10/RealSimGear-GNSx30/64/lin.xpl -shared -rdynamic -undefined_warning -lGL -fPIC -ISDK/CHeaders/XPLM -IRealSimGear-GNSx30 -DLIN -DTRACE -DXPLM200 -DXPLM210 -Wfatal-errors -std=c++17 -pthread RealSimGear-GNSx30/plugin.cpp

plugin11/RealSimGear-GNSx30/CommandMapping.ini: RealSimGear-GNSx30/CommandMapping.ini
	mkdir -p plugin11/RealSimGear-GNSx30
	cp -f RealSimGear-GNSx30/CommandMapping.ini plugin11/RealSimGear-GNSx30/CommandMapping.ini

plugin10/RealSimGear-GNSx30/CommandMapping.ini: RealSimGear-GNSx30/CommandMapping.ini
	mkdir -p plugin10/RealSimGear-GNSx30
	cp -f RealSimGear-GNSx30/CommandMapping.ini plugin10/RealSimGear-GNSx30/CommandMapping.ini



ConsoleTest: plugin/RealSimGear-GNSx30/lin_x64/ConsoleTest
plugin/RealSimGear-GNSx30/lin_x64/ConsoleTest: ConsoleTest/ConsoleTest.cpp RealSimGear-GNSx30/plugin.cpp RealSimGear-GNSx30/all_serial_watch.h RealSimGear-GNSx30/command_mapping.h RealSimGear-GNSx30/resource.h RealSimGear-GNSx30/serial_watch.h RealSimGear-GNSx30/serial.h RealSimGear-GNSx30/stdafx.h RealSimGear-GNSx30/util.h RealSimGear-GNSx30/gui/box.h RealSimGear-GNSx30/gui/button.h RealSimGear-GNSx30/gui/color.h RealSimGear-GNSx30/gui/gui.h RealSimGear-GNSx30/gui/label.h RealSimGear-GNSx30/gui/line.h RealSimGear-GNSx30/gui/tools.h RealSimGear-GNSx30/gui/widget.h RealSimGear-GNSx30/gui/window.h linux.make
	mkdir -p plugin/RealSimGear-GNSx30/lin_x64
	g++ $(CXX_FLAGS) -o plugin/RealSimGear-GNSx30/lin_x64/ConsoleTest -ISDK/CHeaders/XPLM -IRealSimGear-GNSx30 -DLIN -DTRACE -Wfatal-errors -std=c++17 -pthread ConsoleTest/ConsoleTest.cpp
