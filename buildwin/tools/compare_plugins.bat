@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

echo === Working plugin (dashboard_pi.dll) dependencies ===
dumpbin /dependents "C:\Program Files (x86)\OpenCPN\plugins\dashboard_pi.dll"

echo.
echo === Our plugin (mayara_server_pi.dll) dependencies ===
dumpbin /dependents "c:\GitHub\mayara-server-opencpn-plugin\build\Release\mayara_server_pi.dll"
