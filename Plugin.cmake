# ~~~
# Summary:      Plugin-specific build setup
# Copyright (c) 2025 MarineYachtRadar
# License:      MIT
# ~~~

# -------- Options ----------

set(OCPN_TEST_REPO
    "marineyachtradar/mayara-server-opencpn-unstable"
    CACHE STRING "Default repository for untagged builds"
)
set(OCPN_BETA_REPO
    "marineyachtradar/mayara-server-opencpn-beta"
    CACHE STRING
    "Default repository for tagged builds matching 'beta'"
)
set(OCPN_RELEASE_REPO
    "marineyachtradar/mayara-server-opencpn"
    CACHE STRING
    "Default repository for tagged builds not matching 'beta'"
)

#
# -------  Plugin setup --------
#
set(PKG_NAME mayara_server_pi)
set(PKG_VERSION 1.0.0)
set(PKG_PRERELEASE "beta")  # Empty, or a tag like 'beta'

set(DISPLAY_NAME mayara-server)    # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME MayaraServer)  # As of GetCommonName() in plugin API
set(CPACK_PACKAGE_CONTACT "info@marineyachtradar.com")
set(PKG_SUMMARY "Displays radar data from mayara-server in OpenCPN")
set(PKG_DESCRIPTION [=[
MaYaRa Server Plugin for OpenCPN

Connects to a running mayara-server instance to display radar data from
Furuno, Navico, Raymarine, and Garmin radars.

Features:
- Chart overlay rendering
- Separate PPI (Plan Position Indicator) window
- ARPA target tracking with CPA/TCPA
- Full radar control (gain, sea, rain, range)
- Multi-radar support

WARNING: OPENGL MODE IS REQUIRED!

Requires mayara-server running on localhost or network.
]=])

set(PKG_AUTHOR "MarineYachtRadar")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/MarineYachtRadar/mayara-server-opencpn-plugin)
set(PKG_INFO_URL https://github.com/MarineYachtRadar/mayara-server)


# FULL PLUGIN - plugin class in global namespace (not mayara::) to avoid DLL static init issues
set(SRC
  include/mayara_server_pi.h
  include/pi_common.h
  include/MayaraClient.h
  include/SpokeReceiver.h
  include/SpokeBuffer.h
  include/ColorPalette.h
  include/icons.h
  include/gl_funcs.h
  include/RadarRenderer.h
  include/RadarOverlayRenderer.h
  include/RadarPPIRenderer.h
  include/RadarCanvas.h
  include/RadarControlDialog.h
  include/DynamicControlPanel.h
  include/PreferencesDialog.h
  include/RadarDisplay.h
  include/RadarManager.h

  src/mayara_server_pi.cpp
  src/MayaraClient.cpp
  src/SpokeReceiver.cpp
  src/SpokeBuffer.cpp
  src/ColorPalette.cpp
  src/icons.cpp
  src/gl_funcs.cpp
  src/RadarRenderer.cpp
  src/RadarOverlayRenderer.cpp
  src/RadarPPIRenderer.cpp
  src/RadarCanvas.cpp
  src/RadarControlDialog.cpp
  src/DynamicControlPanel.cpp
  src/PreferencesDialog.cpp
  src/RadarDisplay.cpp
  src/RadarManager.cpp
)

set(PKG_API_LIB api-16)  #  API 1.16 like radar_pi uses - works with OpenCPN 5.12

macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers
  # and ocpn::api is available.

  # Fix OpenGL deprecated warnings in Xcode
  target_compile_definitions(${PACKAGE_NAME} PRIVATE GL_SILENCE_DEPRECATION)

  target_include_directories(${PACKAGE_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
  )

  # IXWebSocket for HTTP/WebSocket - build as static library to avoid DLL export issues
  # Use FORCE to override IXWebSocket's default option() values
  set(USE_TLS OFF CACHE BOOL "Disable TLS" FORCE)
  set(USE_ZLIB OFF CACHE BOOL "Disable zlib compression" FORCE)
  set(IXWEBSOCKET_INSTALL OFF CACHE BOOL "Skip install" FORCE)
  set(IXWEBSOCKET_BUILD_SHARED_LIBS_SAVED ${BUILD_SHARED_LIBS})
  set(BUILD_SHARED_LIBS OFF)
  add_subdirectory(libs/IXWebSocket)
  set(BUILD_SHARED_LIBS ${IXWEBSOCKET_BUILD_SHARED_LIBS_SAVED})
  target_link_libraries(${PACKAGE_NAME} ixwebsocket)

  # nlohmann/json for REST API parsing (header-only)
  target_include_directories(${PACKAGE_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/libs/json/single_include
  )

  # Platform-specific
  if(WIN32)
    # Windows socket libraries required by IXWebSocket
    target_link_libraries(${PACKAGE_NAME} ws2_32 wsock32)
  elseif(UNIX AND NOT APPLE)
    # Linux: Define GL_GLEXT_PROTOTYPES before any GL headers are included
    # This enables shader function declarations in glext.h
    target_compile_definitions(${PACKAGE_NAME} PRIVATE GL_GLEXT_PROTOTYPES)
  endif()
endmacro ()

macro(add_plugin_libraries)
  # Add libraries required by this plugin
  add_subdirectory("opencpn-libs/wxJSON")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)
endmacro ()
