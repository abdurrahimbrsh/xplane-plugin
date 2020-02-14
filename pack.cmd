cd %~dp0
set CONFIG=%1
set PLATFORM=%2

echo "Packing for config %CONFIG% %PLATFORM%..."

REM rmdir /s /q plugin_%CONFIG%

if "%CONFIG%"=="Debug_X_Plane_11" (
	mkdir plugin_%CONFIG%\RealSimGear\win_x64
	copy /y x64\%CONFIG%\RealSimGear-GNSx30.dll plugin_%CONFIG%\RealSimGear\win_x64\RealSimGear.xpl
	copy /y x64\%CONFIG%\RealSimGear-GNSx30.pdb plugin_%CONFIG%\RealSimGear\win_x64\RealSimGear.pdb
	copy /y RealSimGear-GNSx30\CommandMapping.ini plugin_%CONFIG%\RealSimGear\CommandMapping.ini
	copy /y README.txt plugin_%CONFIG%\RealSimGear\README.txt
)

if "%CONFIG%"=="Release_X_Plane_11" (
	mkdir plugin_%CONFIG%\RealSimGear\win_x64
	copy /y x64\%CONFIG%\RealSimGear-GNSx30.dll plugin_%CONFIG%\RealSimGear\win_x64\RealSimGear.xpl
	copy /y RealSimGear-GNSx30\CommandMapping.ini plugin_%CONFIG%\RealSimGear\CommandMapping.ini
	copy /y README.txt plugin_%CONFIG%\RealSimGear\README.txt
)

if "%CONFIG%"=="Debug_X_Plane_10" (
	if "%PLATFORM%"=="x64" (
		mkdir plugin_%CONFIG%\RealSimGear\64
		copy /y x64\%CONFIG%\RealSimGear-GNSx30.dll plugin_%CONFIG%\RealSimGear\64\win.xpl
		copy /y x64\%CONFIG%\RealSimGear-GNSx30.pdb plugin_%CONFIG%\RealSimGear\64\win.pdb
		copy /y RealSimGear-GNSx30\CommandMapping.ini plugin_%CONFIG%\RealSimGear\CommandMapping.ini
		copy /y README.txt plugin_%CONFIG%\RealSimGear\README.txt
	) else (
		mkdir plugin_%CONFIG%\RealSimGear\32
		copy /y %CONFIG%\RealSimGear-GNSx30.dll plugin_%CONFIG%\RealSimGear\32\win.xpl
		copy /y %CONFIG%\RealSimGear-GNSx30.pdb plugin_%CONFIG%\RealSimGear\32\win.pdb
		copy /y RealSimGear-GNSx30\CommandMapping.ini plugin_%CONFIG%\RealSimGear\CommandMapping.ini
		copy /y README.txt plugin_%CONFIG%\RealSimGear\README.txt	
	)
)

if "%CONFIG%"=="Release_X_Plane_10" (
	if "%PLATFORM%"=="x64" (
		mkdir plugin_%CONFIG%\RealSimGear\64
		copy /y x64\%CONFIG%\RealSimGear-GNSx30.dll plugin_%CONFIG%\RealSimGear\64\win.xpl
		copy /y RealSimGear-GNSx30\CommandMapping.ini plugin_%CONFIG%\RealSimGear\CommandMapping.ini
		copy /y README.txt plugin_%CONFIG%\RealSimGear\README.txt
	) else (
		mkdir plugin_%CONFIG%\RealSimGear-GNSx30\32
		copy /y %CONFIG%\RealSimGear-GNSx30.dll plugin_%CONFIG%\RealSimGear\32\win.xpl
		copy /y RealSimGear-GNSx30\CommandMapping.ini plugin_%CONFIG%\RealSimGear\CommandMapping.ini
		copy /y README.txt plugin_%CONFIG%\RealSimGear\README.txt	
	)
)

echo "Packing completed."


