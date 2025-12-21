@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1
dumpbin /exports "c:\GitHub\mayara-server-opencpn-plugin\build\Release\mayara_server_pi.dll" > c:\GitHub\mayara-server-opencpn-plugin\dll_exports.txt 2>&1
dumpbin /imports "c:\GitHub\mayara-server-opencpn-plugin\build\Release\mayara_server_pi.dll" > c:\GitHub\mayara-server-opencpn-plugin\dll_imports.txt 2>&1
echo Done
