@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

echo === MAYARA_SERVER_PI IMPORTS === > c:\GitHub\mayara-server-opencpn-plugin\imports.txt
dumpbin /imports "c:\GitHub\mayara-server-opencpn-plugin\build\Release\mayara_server_pi.dll" >> c:\GitHub\mayara-server-opencpn-plugin\imports.txt 2>&1

echo === DASHBOARD_PI IMPORTS === >> c:\GitHub\mayara-server-opencpn-plugin\imports.txt
dumpbin /imports "C:\Program Files (x86)\OpenCPN\plugins\dashboard_pi.dll" >> c:\GitHub\mayara-server-opencpn-plugin\imports.txt 2>&1

echo Done - check imports.txt
