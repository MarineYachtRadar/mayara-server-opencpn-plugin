@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

echo Analyzing symbols... > c:\GitHub\mayara-server-opencpn-plugin\symbol_analysis.txt
echo. >> c:\GitHub\mayara-server-opencpn-plugin\symbol_analysis.txt

echo === OUR PLUGIN IMPORTS FROM OPENCPN.EXE === >> c:\GitHub\mayara-server-opencpn-plugin\symbol_analysis.txt
dumpbin /imports "c:\GitHub\mayara-server-opencpn-plugin\build\Release\mayara_server_pi.dll" 2>&1 | findstr /i "opencpn" >> c:\GitHub\mayara-server-opencpn-plugin\symbol_analysis.txt

echo. >> c:\GitHub\mayara-server-opencpn-plugin\symbol_analysis.txt
echo === OPENCPN.EXE EXPORTS === >> c:\GitHub\mayara-server-opencpn-plugin\symbol_analysis.txt
dumpbin /exports "C:\Program Files (x86)\OpenCPN\opencpn.exe" >> c:\GitHub\mayara-server-opencpn-plugin\symbol_analysis.txt 2>&1

echo Done - check symbol_analysis.txt
