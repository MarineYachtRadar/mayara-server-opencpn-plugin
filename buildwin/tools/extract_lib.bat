@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

echo Creating import library from opencpn.exe...
cd /d "C:\Program Files (x86)\OpenCPN"

REM Generate .def file from exports
dumpbin /exports opencpn.exe > c:\GitHub\mayara-server-opencpn-plugin\opencpn_exports.txt

echo Done - check opencpn_exports.txt
