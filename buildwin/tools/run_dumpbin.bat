@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

echo === DASHBOARD_PI DEPENDENCIES === > c:\GitHub\mayara-server-opencpn-plugin\dumpbin_results.txt
dumpbin /dependents "C:\Program Files (x86)\OpenCPN\plugins\dashboard_pi.dll" >> c:\GitHub\mayara-server-opencpn-plugin\dumpbin_results.txt 2>&1

echo. >> c:\GitHub\mayara-server-opencpn-plugin\dumpbin_results.txt
echo === MAYARA_SERVER_PI DEPENDENCIES === >> c:\GitHub\mayara-server-opencpn-plugin\dumpbin_results.txt
dumpbin /dependents "c:\GitHub\mayara-server-opencpn-plugin\build\Release\mayara_server_pi.dll" >> c:\GitHub\mayara-server-opencpn-plugin\dumpbin_results.txt 2>&1

echo. >> c:\GitHub\mayara-server-opencpn-plugin\dumpbin_results.txt
echo === DASHBOARD_PI EXPORTS === >> c:\GitHub\mayara-server-opencpn-plugin\dumpbin_results.txt
dumpbin /exports "C:\Program Files (x86)\OpenCPN\plugins\dashboard_pi.dll" >> c:\GitHub\mayara-server-opencpn-plugin\dumpbin_results.txt 2>&1

echo. >> c:\GitHub\mayara-server-opencpn-plugin\dumpbin_results.txt
echo === MAYARA_SERVER_PI EXPORTS === >> c:\GitHub\mayara-server-opencpn-plugin\dumpbin_results.txt
dumpbin /exports "c:\GitHub\mayara-server-opencpn-plugin\build\Release\mayara_server_pi.dll" >> c:\GitHub\mayara-server-opencpn-plugin\dumpbin_results.txt 2>&1

echo Done - results in dumpbin_results.txt
