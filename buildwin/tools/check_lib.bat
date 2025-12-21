@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
dumpbin /exports "c:\GitHub\mayara-server-opencpn-plugin\opencpn-libs\api-18\msvc-wx32\opencpn.lib" > lib_exports.txt 2>&1
echo Done - see lib_exports.txt
