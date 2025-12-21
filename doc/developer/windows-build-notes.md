# Windows Build Notes for OpenCPN 5.12

This document captures lessons learned while getting the plugin to build and load correctly on Windows 11 with OpenCPN 5.12.4.

## Working Configuration

- **API Version**: 1.16 (using `opencpn_plugin_116` base class)
- **wxWidgets**: 3.2.8 (must match OpenCPN's version exactly)
- **Architecture**: Win32 (x86) - OpenCPN 5.12 is 32-bit on Windows
- **Compiler**: MSVC (Visual Studio 2022)

## API Versions Tested

### API 1.16 (WORKS)
- Uses `opencpn_plugin_116` base class
- This is what `radar_pi` uses successfully
- Compatible with OpenCPN 5.12

### API 1.20 (CRASHES)
- Uses `opencpn_plugin_120` base class
- Plugin loads but crashes during initialization
- OpenCPN uses `dynamic_cast` to verify the plugin matches its reported API version
- The crash may be related to vtable layout differences or missing virtual method implementations

### API 1.18, 1.19 (NOT TESTED INDIVIDUALLY)
- Should work but API 1.16 is known-good

## Critical Build Requirements

### 1. wxWidgets Version Must Match
OpenCPN 5.12.4 uses wxWidgets 3.2.8. Your plugin MUST be built against the same version.

Download prebuilt binaries from: https://www.wxwidgets.org/downloads/
- `wxMSW-3.2.8_vc14x_Dev.7z` (development files)
- `wxMSW-3.2.8_vc14x_ReleaseDLL.7z` (runtime DLLs)
- `wxWidgets-3.2.8-headers.7z` (headers)

### 2. opencpn.lib Must Match Installed OpenCPN
The `opencpn.lib` import library in `opencpn-libs/api-XX/msvc-wx32/` must be generated from YOUR installed OpenCPN version.

To regenerate `opencpn.lib` from OpenCPN 5.12.4:

```batch
@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"

REM Extract exports from installed OpenCPN
dumpbin /exports "C:\Program Files (x86)\OpenCPN\opencpn.exe" > opencpn_exports.txt

REM Create .def file
echo LIBRARY opencpn.exe > opencpn.def
echo EXPORTS >> opencpn.def
REM (parse exports from opencpn_exports.txt and add to .def file)

REM Generate import library
lib /def:opencpn.def /out:opencpn.lib /machine:x86
```

See `buildwin/tools/make_opencpn_lib.bat` for the full script.

### 3. 32-bit Build Required
OpenCPN 5.12 on Windows is 32-bit. Configure CMake with:
```
-G "Visual Studio 17 2022" -A Win32
```

## Plugin Blacklisting

When a plugin crashes during load, OpenCPN blacklists it and refuses to load it again. The blacklist message in the log looks like:

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

The blacklist is stored in memory during the OpenCPN session, keyed by the DLL path. Removing the DLL and restarting clears it.

## CMake Configuration

```bash
cmake -B build -S . \
  -DCMAKE_BUILD_TYPE=Release \
  -DwxWidgets_ROOT_DIR=path/to/wxWidgets-3.2.8 \
  -DwxWidgets_LIB_DIR=path/to/wxWidgets-3.2.8/lib/vc14x_dll \
  -DGETTEXT_MSGFMT_EXECUTABLE="C:/Program Files (x86)/Poedit/GettextTools/bin/msgfmt.exe" \
  -DGETTEXT_MSGMERGE_EXECUTABLE="C:/Program Files (x86)/Poedit/GettextTools/bin/msgmerge.exe" \
  -G "Visual Studio 17 2022" -A Win32
```

## Minimal Test Plugin

When debugging load issues, use a minimal plugin that:
- Inherits from `opencpn_plugin_116`
- Returns `0` from `Init()` (no capabilities)
- Has no external dependencies
- Only includes essential headers

See `src/mayara_server_pi_minimal.cpp` for the test implementation.

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
- `KERNEL32.dll` - Windows

## Useful Paths

- OpenCPN install: `C:\Program Files (x86)\OpenCPN\`
- OpenCPN config: `C:\ProgramData\opencpn\opencpn.ini`
- OpenCPN log: `C:\ProgramData\opencpn\opencpn.log`
- Plugin install dir: `C:\Users\<username>\AppData\Local\opencpn\plugins\`
- Plugin install data: `C:\ProgramData\opencpn\plugins\install_data\`
- Crash reports: `C:\ProgramData\opencpn\CrashReports\`

## Troubleshooting

### "Incompatible plugin detected"
- API version mismatch between reported version and base class
- Ensure `GetAPIVersionMinor()` returns the same version as your base class (e.g., 16 for `opencpn_plugin_116`)

### Crash on load (no log after "Loading PlugIn:")
- wxWidgets version mismatch
- Missing DLL dependencies
- Corrupted `opencpn.lib`

### Plugin loads but doesn't appear in list
- `Init()` returning 0 means no capabilities - plugin won't show toolbar/menu items
- Try adding `WANTS_PREFERENCES` or `INSTALLS_TOOLBAR_TOOL` to `Init()` return value

### Crash when adding capabilities
- Start minimal and add one capability at a time
- Ensure all required virtual methods are implemented for the capability
