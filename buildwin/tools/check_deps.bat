@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
dumpbin /dependents "c:\GitHub\mayara-server-opencpn-plugin\build\Release\mayara_server_pi.dll"
