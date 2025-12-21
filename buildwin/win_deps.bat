::
:: Install build dependencies for Windows.
:: Requires a working choco installation:
:: https://docs.chocolatey.org/en-us/choco/setup
::

:: Install the pathman tool: https://github.com/therootcompany/pathman
::
if not exist "%HomeDrive%%HomePath%\.local\bin\pathman.exe" (
    pushd "%HomeDrive%%HomePath%"
    curl.exe https://webi.ms/pathman | powershell
    popd
)
pathman list > nul 2>&1
if errorlevel 1 set PATH=%PATH%;%HomeDrive%\%HomePath%\.local\bin
pathman add %HomeDrive%%HomePath%\.local\bin >nul

:: Install choco cmake and add it's persistent user path element
::
set CMAKE_HOME=C:\Program Files\CMake
if not exist "%CMAKE_HOME%\bin\cmake.exe" choco install --no-progress -y cmake
pathman add "%CMAKE_HOME%\bin" > nul

:: Install choco poedit (for gettext tools)
::
set POEDIT_HOME=C:\Program Files (x86)\Poedit\Gettexttools
if not exist "%POEDIT_HOME%" choco install --no-progress -y poedit
pathman add "%POEDIT_HOME%\bin" > nul

:: Update required python stuff
::
python --version > nul 2>&1 && python -m ensurepip > nul 2>&1
if errorlevel 1 choco install --no-progress -y python
python --version
python -m ensurepip
python -m pip install --upgrade pip
python -m pip install -q setuptools wheel
python -m pip install -q cloudsmith-cli

:: Install pre-compiled wxWidgets; add required paths.
::
set SCRIPTDIR=%~dp0
set WXWIN=%SCRIPTDIR%..\cache\wxWidgets
set wxWidgets_ROOT_DIR=%WXWIN%
set wxWidgets_LIB_DIR=%WXWIN%\lib\vc_dll
if not exist "%WXWIN%" (
  wget --version > nul 2>&1 || choco install --no-progress -y wget
  wget -q https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.4/wxWidgets-3.2.4-headers.7z ^
      -O wxWidgetsHeaders.7z
  wget -q https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.4/wxMSW-3.2.4_vc14x_ReleaseDLL.7z ^
      -O wxWidgetsDLL.7z
  wget -q https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.4/wxMSW-3.2.4_vc14x_Dev.7z ^
      -O wxWidgetsDev.7z
  7z i > nul 2>&1 || choco install -y 7zip
  7z x -aoa wxWidgetsHeaders.7z -o%WXWIN%
  7z x -aoa wxWidgetsDLL.7z -o%WXWIN%
  7z x -aoa wxWidgetsDev.7z -o%WXWIN%
  ren "%WXWIN%\lib\vc14x_dll" vc_dll
)
pathman add "%WXWIN%" > nul
pathman add "%wxWidgets_LIB_DIR%" > nul

:: Note: Do not call refreshenv here as it clears the session variables we just set
:: The wxWidgets_ROOT_DIR and wxWidgets_LIB_DIR are needed for cmake
