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

### Build Steps

```bash
# Clone with submodules
git clone --recurse-submodules https://github.com/MarineYachtRadar/mayara-server-opencpn-plugin.git
cd mayara-server-opencpn-plugin

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build . --target tarball
```

### Platform-specific builds

See the `ci/` directory for platform-specific build scripts.

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
