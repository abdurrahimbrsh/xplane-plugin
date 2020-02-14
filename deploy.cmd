set CONFIG=%1

if "%CONFIG%"=="Debug_X_Plane_11" (
	SET XPLANEPLUGINS=D:\X-Plane 11\Resources\plugins
)
if "%CONFIG%"=="Release_X_Plane_11" (
	SET XPLANEPLUGINS=D:\X-Plane 11\Resources\plugins
)
if "%CONFIG%"=="Debug_X_Plane_10" (
	SET XPLANEPLUGINS=D:\X-Plane 10\Resources\plugins
)
if "%CONFIG%"=="Release_X_Plane_10" (
	SET XPLANEPLUGINS=D:\X-Plane 10\Resources\plugins
)

echo "Deploying to %XPLANEPLUGINS%..."

cd %~dp0
rmdir /s /q "%XPLANEPLUGINS%\RealSimGear-GNSx30"
xcopy /e "plugin_%CONFIG%\*.*" "%XPLANEPLUGINS%\"

echo "Deployment complete."

