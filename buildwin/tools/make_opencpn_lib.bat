@echo off
setlocal enabledelayedexpansion

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

cd /d c:\GitHub\mayara-server-opencpn-plugin

echo Step 1: Extracting exports from OpenCPN 5.12.4...
dumpbin /exports "C:\Program Files (x86)\OpenCPN\opencpn.exe" > opencpn_exports_raw.txt

echo Step 2: Creating .def file...
echo LIBRARY opencpn.exe > opencpn_512.def
echo EXPORTS >> opencpn_512.def

set "started="
for /f "usebackq tokens=1,2,3,4*" %%a in ("opencpn_exports_raw.txt") do (
    if defined started (
        if "%%a"=="Summary" goto :done_parsing
        if not "%%d"=="" (
            echo     %%d >> opencpn_512.def
        )
    )
    if "%%a"=="ordinal" if "%%b"=="hint" set "started=1"
)
:done_parsing

echo Step 3: Generating import library...
lib /def:opencpn_512.def /out:opencpn_512.lib /machine:x86

if exist opencpn_512.lib (
    echo.
    echo SUCCESS! Created opencpn_512.lib
    echo.
    echo Copying to opencpn-libs\api-20\msvc-wx32\opencpn.lib...
    copy /y opencpn_512.lib opencpn-libs\api-20\msvc-wx32\opencpn.lib
    echo Done!
) else (
    echo ERROR: Failed to create lib file
)

echo.
echo Check opencpn_exports_raw.txt for raw dumpbin output
pause
