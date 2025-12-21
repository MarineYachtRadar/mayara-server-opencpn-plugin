# Windows Build Notes for OpenCPN 5.12

This document captures lessons learned while getting the plugin to build and load correctly on Windows 11 with OpenCPN 5.12.4.

## Status: WORKING (2024-12-22)

Full plugin builds and loads successfully in OpenCPN 5.12.4 on Windows 11:
- All support files: WORKS
- nlohmann/json (header-only): WORKS
- IXWebSocket (static linked): WORKS
- wxJSON: WORKS
- All renderers, dialogs, RadarDisplay, RadarManager: WORKS
- Full mayara_server_pi.cpp: WORKS

**Solution**: Define plugin class inline in the .cpp file, not from a header. See "DLL Static Init Solution" below.

## Working Configuration

- **API Version**: 1.16 (using `opencpn_plugin_116` base class)
- **wxWidgets**: 3.2.8 (must match OpenCPN's version exactly)
- **Architecture**: Win32 (x86) - OpenCPN 5.12 is 32-bit on Windows
- **Compiler**: MSVC (Visual Studio 2022 Build Tools)
- **CMake**: 4.2.1+

## Quick Start

### 1. Configure (first time only)

```powershell
$env:WXWIN = 'c:/GitHub/mayara-server-opencpn-plugin/cache/wxWidgets-3.2.8'

& 'C:\Program Files\CMake\bin\cmake.exe' -B build -S . `
  -DCMAKE_BUILD_TYPE=Release `
  -DwxWidgets_ROOT_DIR="$env:WXWIN" `
  -DwxWidgets_LIB_DIR="$env:WXWIN/lib/vc14x_dll" `
  -DGETTEXT_MSGFMT_EXECUTABLE="C:/Program Files (x86)/Poedit/GettextTools/bin/msgfmt.exe" `
  -DGETTEXT_MSGMERGE_EXECUTABLE="C:/Program Files (x86)/Poedit/GettextTools/bin/msgmerge.exe"
```

### 2. Build DLL

```powershell
& 'C:\Program Files\CMake\bin\cmake.exe' --build build --config Release --target mayara_server_pi
```

### 3. Create Tarball (Manual Method)

The CMake `tarball` target has issues with WXWIN environment. Use this manual process:

```powershell
# Create directory structure
New-Item -ItemType Directory -Path 'build/app/MayaraServer/plugins' -Force

# Copy DLL
Copy-Item 'build/Release/mayara_server_pi.dll' 'build/app/MayaraServer/plugins/'

# Create metadata.xml (see example below)

# Create tarball
cd build/app
& 'C:\Program Files\CMake\bin\cmake.exe' -E tar czf ../MayaraServer-test.tar.gz MayaraServer
```

### 4. metadata.xml Format

```xml
<?xml version="1.0" encoding="UTF-8"?>
<plugin version="1">
  <name> MayaraServer </name>
  <version> 1.0.0-beta </version>
  <release> 1 </release>
  <summary> Displays radar data from mayara-server in OpenCPN </summary>

  <api-version> 1.16 </api-version>
  <open-source> yes </open-source>
  <author> MarineYachtRadar </author>
  <source> https://github.com/MarineYachtRadar/mayara-server-opencpn-plugin </source>
  <info-url> https://github.com/MarineYachtRadar/mayara-server </info-url>

  <description> MaYaRa Server Plugin for OpenCPN </description>

  <target>msvc-wx32</target>
  <target-version>10.0.26200</target-version>
  <target-arch>x86</target-arch>
  <tarball-url> </tarball-url>
  <tarball-checksum> </tarball-checksum>
</plugin>
```

### 5. Install in OpenCPN

1. **Close OpenCPN completely**
2. Delete old plugin files (critical - old DLL may persist!):
   ```powershell
   del C:\Users\<username>\AppData\Local\opencpn\plugins\mayara_server_pi.dll
   del C:\ProgramData\opencpn\plugins\install_data\mayaraserver.*
   ```
3. Open OpenCPN -> Options -> Plugins
4. Click "Import Plugin..."
5. Select the `.tar.gz` file
6. Restart OpenCPN if prompted

**IMPORTANT**: Always verify the tarball timestamp before installing! Check with:
```powershell
Get-Item build\MayaraServer-test.tar.gz | Select-Object Name, Length, LastWriteTime
```

## Incremental Testing Strategy

When debugging DLL load crashes, use binary search to isolate the problem:

1. Start with minimal test plugin (`mayara_server_pi_test.cpp`)
2. Add components incrementally:
   - Libraries first (nlohmann/json, IXWebSocket, wxJSON)
   - Then support files (ColorPalette, SpokeBuffer, icons, gl_funcs)
   - Then network code (MayaraClient, SpokeReceiver)
   - Then renderers (RadarRenderer, RadarOverlayRenderer, etc.)
   - Then dialogs (PreferencesDialog, RadarControlDialog)
   - Then managers (RadarDisplay, RadarManager)
   - Finally, main plugin file (mayara_server_pi.cpp)

3. After each addition:
   - Clean rebuild: `cmake --build build --config Release --target mayara_server_pi --clean-first`
   - Create fresh tarball
   - Delete old plugin files
   - Install and test

## Tested Components (2024-12-22)

| Component | Status | Notes |
|-----------|--------|-------|
| mayara_server_pi_test.cpp | WORKS | Test entry point with wxEvtHandler |
| nlohmann/json | WORKS | Header-only, no static init issues |
| IXWebSocket | WORKS | Static linked, USE_TLS=OFF, USE_ZLIB=OFF |
| wxJSON | WORKS | From opencpn-libs |
| ColorPalette.cpp | WORKS | No static initializers |
| SpokeBuffer.cpp | WORKS | No static initializers |
| icons.cpp | WORKS | Static pointers init to nullptr |
| gl_funcs.cpp | WORKS | Static pointers init to nullptr |
| MayaraClient.cpp | WORKS | Uses IXWebSocket HTTP client |
| SpokeReceiver.cpp | WORKS | Uses IXWebSocket WebSocket |
| RadarRenderer.cpp | WORKS | OpenGL shader management |
| RadarOverlayRenderer.cpp | WORKS | Chart overlay rendering |
| RadarPPIRenderer.cpp | WORKS | PPI window rendering |
| RadarCanvas.cpp | WORKS | wxGLCanvas for PPI |
| RadarControlDialog.cpp | WORKS | Radar control UI |
| PreferencesDialog.cpp | WORKS | Plugin settings UI |
| RadarDisplay.cpp | WORKS | Radar data management |
| RadarManager.cpp | WORKS | Multi-radar coordination |
| mayara_server_pi.h | N/A | Not used - class defined inline in .cpp |
| mayara_server_pi.cpp | WORKS | Plugin class defined inline, see "DLL Static Init Solution" |

## DLL Static Init Solution

Windows MSVC has strict requirements for DLL static initialization. The plugin class MUST be defined inline in the .cpp file, not included from a header.

### The Problem

Including `mayara_server_pi.h` (which includes `pi_common.h` → `<wx/glcanvas.h>`) causes DLL static initialization crashes on Windows/MSVC. The crash occurs before `create_pi()` is even called.

### What Did NOT Work

1. Moving plugin class to global namespace (still in header) - CRASHED
2. Fully qualified event table names - CRASHED
3. Removing `DECLARE_EVENT_TABLE()` entirely - CRASHED
4. Moving `using namespace` after event table - CRASHED

### The Solution

Define the plugin class **inline in the .cpp file** with minimal includes. This matches the pattern used by `radar_pi` and other working plugins.

```cpp
// mayara_server_pi.cpp - THE WORKING PATTERN

// Use same includes as working test version
#include "config.h"
#include <wx/wx.h>
#include <wx/bitmap.h>
#include <wx/fileconf.h>
#include "ocpn_plugin.h"

// Include support files AFTER basic wx includes
#include "RadarManager.h"
#include "RadarDisplay.h"
#include "RadarOverlayRenderer.h"
#include "PreferencesDialog.h"
#include "icons.h"

#include <ixwebsocket/IXNetSystem.h>
#include <memory>
#include <string>

// Plugin class defined INLINE (not from header) to avoid DLL static init crash
class mayara_server_pi : public opencpn_plugin_116, public wxEvtHandler {
public:
    mayara_server_pi(void* ppimgr);
    ~mayara_server_pi() override;

    // ... full class definition here ...

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(mayara_server_pi, wxEvtHandler)
    EVT_TIMER(ID_TIMER, mayara_server_pi::OnTimerNotify)
END_EVENT_TABLE()

// ... implementation ...
```

### Key Points

1. **DO NOT** include `mayara_server_pi.h` - define class inline
2. **DO** use minimal includes: `config.h`, `<wx/wx.h>`, `<wx/bitmap.h>`, `<wx/fileconf.h>`, `ocpn_plugin.h`
3. **DO** include support headers AFTER the basic wx includes
4. **DO** put plugin class in global namespace (not `namespace mayara {}`)
5. Support classes (RadarManager, etc.) can stay in `namespace mayara {}`
6. Forward declare plugin class in support headers: `class mayara_server_pi;`
7. Use `::mayara_server_pi*` in support headers for explicit global namespace reference

## API Versions

### API 1.16 (WORKS - CONFIRMED)
- Uses `opencpn_plugin_116` base class
- This is what `radar_pi` uses successfully
- Compatible with OpenCPN 5.12
- **Tested and working on 2024-12-22**

### API 1.20 (CRASHES)
- Uses `opencpn_plugin_120` base class
- Plugin loads but crashes during initialization
- The crash may be related to vtable layout differences

## Critical Build Requirements

### 1. wxWidgets Version Must Match
OpenCPN 5.12.4 uses wxWidgets 3.2.8. Your plugin MUST be built against the same version.

Download prebuilt binaries from: https://www.wxwidgets.org/downloads/
- `wxMSW-3.2.8_vc14x_Dev.7z` (development files)
- `wxMSW-3.2.8_vc14x_ReleaseDLL.7z` (runtime DLLs)
- `wxWidgets-3.2.8-headers.7z` (headers)

### 2. 32-bit Build Required
OpenCPN 5.12 on Windows is 32-bit. CMake automatically configures for Win32.

### 3. IXWebSocket Configuration
Must be built as static library with TLS and zlib disabled:
```cmake
set(USE_TLS OFF CACHE BOOL "Disable TLS" FORCE)
set(USE_ZLIB OFF CACHE BOOL "Disable zlib compression" FORCE)
set(IXWEBSOCKET_INSTALL OFF CACHE BOOL "Skip install" FORCE)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(libs/IXWebSocket)
target_link_libraries(${PACKAGE_NAME} ixwebsocket)
```

Also need Windows socket libraries:
```cmake
target_link_libraries(${PACKAGE_NAME} ws2_32 wsock32)
```

## Plugin Blacklisting

When a plugin crashes during load, OpenCPN blacklists it. The blacklist message:
```
Refusing to load C:\Users\...\mayara_server_pi.dll failed at last attempt
```

### How to Clear the Blacklist

1. **Close OpenCPN completely**
2. Delete the plugin DLL:
   ```
   del C:\Users\<username>\AppData\Local\opencpn\plugins\mayara_server_pi.dll
   ```
3. Delete the install data:
   ```
   del C:\ProgramData\opencpn\plugins\install_data\mayaraserver.*
   ```
4. Start OpenCPN fresh
5. Import the new tarball

## DLL Dependencies

Check dependencies with:
```powershell
dumpbin /dependents build\Release\mayara_server_pi.dll
```

Expected dependencies:
- `wxbase32u_vc14x.dll` - from OpenCPN directory
- `wxmsw32u_core_vc14x.dll` - from OpenCPN directory
- `opencpn.exe` - the host application
- `VCRUNTIME140.dll` - Visual C++ runtime
- `KERNEL32.dll`, `WS2_32.dll` - Windows

## Useful Paths

- OpenCPN install: `C:\Program Files (x86)\OpenCPN\`
- OpenCPN config: `C:\ProgramData\opencpn\opencpn.ini`
- OpenCPN log: `C:\ProgramData\opencpn\opencpn.log`
- Plugin install dir: `C:\Users\<username>\AppData\Local\opencpn\plugins\`
- Plugin install data: `C:\ProgramData\opencpn\plugins\install_data\`

## Troubleshooting

### "Incompatible plugin detected"
- API version mismatch between reported version and base class
- Ensure `GetAPIVersionMinor()` returns 16 for `opencpn_plugin_116`
- Check metadata.xml uses `<api-version>` (hyphen) not `<api_version>` (underscore)

### Crash on load (no log after "Loading PlugIn:")
- wxWidgets version mismatch
- Missing DLL dependencies
- Static initialization issue in plugin code

### Old plugin still loading after update
- OpenCPN may cache the DLL path
- Always delete old DLL and install_data before importing new tarball
- Verify tarball timestamp is fresh

### Plugin loads but doesn't appear in list
- `Init()` returning 0 means no capabilities
- Add `WANTS_PREFERENCES` or `INSTALLS_TOOLBAR_TOOL` to `Init()` return value

## Debugging Tools

The `buildwin/tools/` directory contains batch scripts and PowerShell utilities for Windows build debugging:

| Script | Purpose |
|--------|---------|
| `check_deps.bat` | Shows DLL dependencies using `dumpbin /dependents` |
| `check_exports.bat` | Lists exported and imported symbols from the DLL |
| `check_imports.bat` | Analyzes what the DLL imports from other libraries |
| `analyze_symbols.bat` | Symbol analysis for debugging link errors |
| `compare_plugins.bat` | Compares your plugin DLL against a working reference |
| `generate_lib.bat/.ps1` | Generates import libraries from DLLs |
| `make_opencpn_lib.bat` | Creates `opencpn.lib` import library from `opencpn.exe` exports |
| `get_crash.ps1` | Queries Windows Event Log for crash events (Event ID 1000) |
| `run_dumpbin.bat` | Runs dumpbin with proper VS environment setup |

### Most Useful Tools

**Check DLL dependencies:**
```powershell
.\buildwin\tools\check_deps.bat
```

**Find crash details when plugin fails to load:**
```powershell
powershell -ExecutionPolicy Bypass -File .\buildwin\tools\get_crash.ps1
```

**Regenerate OpenCPN import library (if needed):**
```powershell
.\buildwin\tools\make_opencpn_lib.bat
```
