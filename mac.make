ifeq ($(DEBUG),1)
	CXX_FLAGS = -g 
else
	CXX_FLAGS = -O3	
endif

all: plugin11/RealSimGear-GNSx30/CommandMapping.ini plugin11/RealSimGear-GNSx30/mac_x64/RealSimGear-GNSx30.xpl plugin10/RealSimGear-GNSx30/CommandMapping.ini plugin10/RealSimGear-GNSx30/mac.xpl


plugin11/RealSimGear-GNSx30/mac_x64/RealSimGear-GNSx30.xpl: RealSimGear-GNSx30/plugin.cpp RealSimGear-GNSx30/all_serial_watch.h RealSimGear-GNSx30/command_mapping.h RealSimGear-GNSx30/resource.h RealSimGear-GNSx30/serial_watch.h RealSimGear-GNSx30/serial.h RealSimGear-GNSx30/stdafx.h RealSimGear-GNSx30/util.h RealSimGear-GNSx30/gui/box.h RealSimGear-GNSx30/gui/button.h RealSimGear-GNSx30/gui/color.h RealSimGear-GNSx30/gui/gui.h RealSimGear-GNSx30/gui/label.h RealSimGear-GNSx30/gui/line.h RealSimGear-GNSx30/gui/tools.h RealSimGear-GNSx30/gui/widget.h RealSimGear-GNSx30/gui/window.h RealSimGear-GNSx30/gui/circle_button.h mac.make
	mkdir -p plugin11/RealSimGear-GNSx30/mac_x64
	clang++ $(CXX_FLAGS) -shared -o plugin11/RealSimGear-GNSx30/mac_x64/RealSimGear-GNSx30.xpl -F SDK/Libraries/Mac -framework XPLM -framework IOKit -framework CoreFoundation -framework OpenGL -ISDK/CHeaders/XPLM -IRealSimGear-GNSx30 -DXPLM200 -DXPLM210 -DXPLM300 -DXPLM301 -DAPL -DTRACE -Wfatal-errors -std=c++17 -pthread RealSimGear-GNSx30/plugin.cpp

plugin10/RealSimGear-GNSx30/mac.xpl: RealSimGear-GNSx30/plugin.cpp RealSimGear-GNSx30/all_serial_watch.h RealSimGear-GNSx30/command_mapping.h RealSimGear-GNSx30/resource.h RealSimGear-GNSx30/serial_watch.h RealSimGear-GNSx30/serial.h RealSimGear-GNSx30/stdafx.h RealSimGear-GNSx30/util.h RealSimGear-GNSx30/gui/box.h RealSimGear-GNSx30/gui/button.h RealSimGear-GNSx30/gui/color.h RealSimGear-GNSx30/gui/gui.h RealSimGear-GNSx30/gui/label.h RealSimGear-GNSx30/gui/line.h RealSimGear-GNSx30/gui/tools.h RealSimGear-GNSx30/gui/widget.h RealSimGear-GNSx30/gui/window.h RealSimGear-GNSx30/gui/circle_button.h mac.make
	mkdir -p plugin10/RealSimGear-GNSx30
	clang++ $(CXX_FLAGS) -shared -o plugin10/RealSimGear-GNSx30/mac.xpl -F SDK/Libraries/Mac -framework XPLM -framework IOKit -framework CoreFoundation -framework OpenGL -ISDK/CHeaders/XPLM -IRealSimGear-GNSx30 -DXPLM200 -DXPLM210 -DXPLM300 -DXPLM301 -DAPL -DTRACE -Wfatal-errors -std=c++17 -pthread RealSimGear-GNSx30/plugin.cpp

plugin11/RealSimGear-GNSx30/CommandMapping.ini: RealSimGear-GNSx30/CommandMapping.ini
	mkdir -p plugin11/RealSimGear-GNSx30
	cp -f RealSimGear-GNSx30/CommandMapping.ini plugin11/RealSimGear-GNSx30/CommandMapping.ini

plugin10/RealSimGear-GNSx30/CommandMapping.ini: RealSimGear-GNSx30/CommandMapping.ini
	mkdir -p plugin10/RealSimGear-GNSx30
	cp -f RealSimGear-GNSx30/CommandMapping.ini plugin10/RealSimGear-GNSx30/CommandMapping.ini


ConsoleTest: plugin/RealSimGear-GNSx30/mac_x64/ConsoleTest
plugin/RealSimGear-GNSx30/mac_x64/ConsoleTest: ConsoleTest/ConsoleTest.cpp RealSimGear-GNSx30/plugin.cpp RealSimGear-GNSx30/all_serial_watch.h RealSimGear-GNSx30/command_mapping.h RealSimGear-GNSx30/resource.h RealSimGear-GNSx30/serial_watch.h RealSimGear-GNSx30/serial.h RealSimGear-GNSx30/stdafx.h RealSimGear-GNSx30/util.h RealSimGear-GNSx30/gui/box.h RealSimGear-GNSx30/gui/button.h RealSimGear-GNSx30/gui/color.h RealSimGear-GNSx30/gui/gui.h RealSimGear-GNSx30/gui/label.h RealSimGear-GNSx30/gui/line.h RealSimGear-GNSx30/gui/tools.h RealSimGear-GNSx30/gui/widget.h RealSimGear-GNSx30/gui/window.h
	mkdir -p plugin/RealSimGear-GNSx30/mac_x64
	clang++ -g -o plugin/RealSimGear-GNSx30/mac_x64/ConsoleTest -framework IOKit -framework CoreFoundation -ISDK/CHeaders/XPLM -IRealSimGear-GNSx30 -DXPLM200 -DXPLM210 -DXPLM300 -DXPLM301 -DAPL -DTRACE -Wfatal-errors -std=c++17 -pthread ConsoleTest/ConsoleTest.cpp

