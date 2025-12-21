# MaYaRa Server Plugin for OpenCPN

OpenCPN plugin that displays radar data from [mayara-server](https://github.com/MarineYachtRadar/mayara-server).

## Features

- **Multi-brand radar support** via mayara-server (Furuno, Navico, Raymarine, Garmin)
- **Chart overlay** - Radar display integrated with your chart
- **Separate PPI window** - Traditional radar display with range rings
- **ARPA targets** - Click-to-acquire target tracking
- **Full control** - Adjust gain, sea clutter, rain, range
- **Auto-discovery** - Automatically finds radars connected to mayara-server

## Requirements

- OpenCPN 5.8 or later
- [mayara-server](https://github.com/MarineYachtRadar/mayara-server) running
- OpenGL support
- Supported radar connected to your network

## Installation

### Via OpenCPN Plugin Manager

1. Open OpenCPN → Options → Plugins
2. Click "Update Plugin Catalog"
3. Find "MaYaRa Server" and click Install
4. Restart OpenCPN

### Manual Installation

Download the appropriate package from [Releases](https://github.com/MarineYachtRadar/mayara-server-opencpn-plugin/releases) and use OpenCPN's "Import Plugin" feature.

## Quick Start

1. Start mayara-server:
   ```bash
   ./mayara-server --port 6502
   ```

2. In OpenCPN, click the MaYaRa toolbar icon

3. The plugin will automatically discover your radar

## Architecture

```
┌─────────────┐      REST/WebSocket      ┌────────────────┐
│   OpenCPN   │◄────────────────────────►│  mayara-server │
│  (plugin)   │      localhost:6502      │   (backend)    │
└─────────────┘                          └───────┬────────┘
                                                 │
                                          ┌──────▼──────┐
                                          │    Radar    │
                                          │  (network)  │
                                          └─────────────┘
```

The plugin connects to mayara-server via HTTP/WebSocket API. All radar protocol handling is done by the server - the plugin focuses on display and user interface.

## Building from Source

### Prerequisites

- CMake 3.12+
- C++17 compiler
- wxWidgets (via OpenCPN)
- Git

### Build Steps (Linux/macOS)

```bash
# Clone with submodules
git clone --recurse-submodules https://github.com/MarineYachtRadar/mayara-server-opencpn-plugin.git
cd mayara-server-opencpn-plugin

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target tarball
```

### Build Steps (Windows)

```cmd
# Clone with submodules
git clone --recurse-submodules https://github.com/MarineYachtRadar/mayara-server-opencpn-plugin.git
cd mayara-server-opencpn-plugin

# Install dependencies (downloads wxWidgets, etc.)
call buildwin\win_deps.bat

# Configure (from MSVC Developer Command Prompt)
cmake -A Win32 -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17

# Build
cmake --build build --config Release --target tarball-finish

# dirk rebuild
C:\ProgramData\opencpn\opencpn.ini
---> add CatalogExpert=1

rmdir /s /q cache\wxWidgets
rmdir /s /q build
call buildwin\win_deps.bat
"C:\Program Files\CMake\bin\cmake.exe" -A Win32 -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17 -DGETTEXT_MSGFMT_EXECUTABLE="C:/Program Files (x86)/Poedit/GettextTools/bin/msgfmt.exe" -DGETTEXT_MSGMERGE_EXECUTABLE="C:/Program Files (x86)/Poedit/GettextTools/bin/msgmerge.exe"
"C:\Program Files\CMake\bin\cmake.exe" --build build --config Release --target tarball-finish

cd build\app
"C:\Program Files\CMake\bin\cmake.exe" -E tar czf ..\mayara_server_pi.tar.gz files
del "C:\Users\Mastercabin\AppData\Local\opencpn\plugins\mayara_server_pi.dll"

# clear blacklist 
rm -f c:/Users/Mastercabin/AppData/Local/opencpn/plugins/mayara_server_pi.dll && rm -f c:/ProgramData/opencpn/plugins/install_data/mayaraserver.* && echo "Removed old plugin files"
clean also C:\ProgramData\opencpn\plugins\install_data\
Delete any mayaraserver files in 
```

The built plugin tarball will be in `build/`.

## Documentation

Full user documentation is available in the `manual/` directory (AsciiDoc format).

## Configuration

Plugin settings are available via Options → Plugins → MaYaRa Server → Preferences:

- **Server Host/Port** - mayara-server connection (default: localhost:6502)
- **Discovery Interval** - How often to poll for new radars
- **Show Overlay** - Enable chart overlay display
- **Show PPI Window** - Enable separate radar window

## API

The plugin uses the same REST/WebSocket API as the [SignalK plugin](https://github.com/MarineYachtRadar/mayara-server-signalk-plugin):

| Endpoint | Purpose |
|----------|---------|
| `GET /v2/api/radars` | List radars |
| `GET /v2/api/radars/{id}/capabilities` | Radar specs |
| `GET /v2/api/radars/{id}/state` | Current settings |
| `PUT /v2/api/radars/{id}/controls/{ctrl}` | Set control |
| `WS /v2/api/radars/{id}/spokes` | Binary spoke stream |

## License

MIT License - see [LICENSE](LICENSE)

## Contributing

Contributions welcome! Please open an issue or pull request.

## Related Projects

- [mayara-server](https://github.com/MarineYachtRadar/mayara-server) - Radar backend
- [mayara-server-signalk-plugin](https://github.com/MarineYachtRadar/mayara-server-signalk-plugin) - SignalK integration
- [mayara-gui](https://github.com/MarineYachtRadar/mayara-gui) - Web-based radar display
