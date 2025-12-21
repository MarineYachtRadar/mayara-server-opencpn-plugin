@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"

echo Step 1: Extracting exports from OpenCPN 5.12.4...
dumpbin /exports "C:\Program Files (x86)\OpenCPN\opencpn.exe" > opencpn_exports_raw.txt

echo Step 2: Creating .def file...
echo LIBRARY opencpn.exe > opencpn_512.def
echo EXPORTS >> opencpn_512.def

REM Parse the exports and add them to the def file
REM Format: ordinal hint RVA name
for /f "skip=19 tokens=4" %%a in (opencpn_exports_raw.txt) do (
    if not "%%a"=="" (
        if not "%%a"=="Summary" (
            echo     %%a >> opencpn_512.def
        )
    )
)

echo Step 3: Generating import library...
lib /def:opencpn_512.def /out:opencpn_512.lib /machine:x86

echo.
echo Done! New library: opencpn_512.lib
echo Copy this to opencpn-libs\api-20\msvc-wx32\opencpn.lib
pause
