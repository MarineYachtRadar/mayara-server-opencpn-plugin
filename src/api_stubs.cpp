/*
 * MaYaRa Server Plugin for OpenCPN
 * Copyright (c) 2025 MarineYachtRadar
 * License: MIT
 *
 * Stub implementations for OpenCPN plugin API virtual functions.
 * These are needed because the OpenCPN import library doesn't provide
 * implementations for all classes declared in ocpn_plugin.h.
 *
 * Note: This is only needed on Windows MSVC builds. On Linux/macOS,
 * the dynamic linker resolves these at runtime from OpenCPN.
 */

#include "pi_common.h"

#ifdef __WXMSW__

// PlugInChartBase stubs
// These classes are declared with DECL_EXP in ocpn_plugin.h but implementations
// are not provided in the import library. Since our plugin doesn't use chart
// functionality, we provide stub implementations to satisfy the linker.

PlugInChartBase::PlugInChartBase() {}
PlugInChartBase::~PlugInChartBase() {}

wxString PlugInChartBase::GetFileSearchMask(void) { return wxEmptyString; }
int PlugInChartBase::Init(const wxString& full_path, int init_flags) { return 0; }
void PlugInChartBase::SetColorScheme(int cs, bool bApplyImmediate) {}
double PlugInChartBase::GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom) { return 0.0; }
double PlugInChartBase::GetNormalScaleMax(double canvas_scale_factor, int canvas_width) { return 0.0; }
double PlugInChartBase::GetNearestPreferredScalePPM(double target_scale_ppm) { return 0.0; }
bool PlugInChartBase::GetChartExtent(ExtentPI* pext) { return false; }
wxBitmap& PlugInChartBase::RenderRegionView(const PlugIn_ViewPort& VPoint, const wxRegion& Region) {
    static wxBitmap empty;
    return empty;
}
bool PlugInChartBase::AdjustVP(PlugIn_ViewPort& vp_last, PlugIn_ViewPort& vp_proposed) { return false; }
void PlugInChartBase::GetValidCanvasRegion(const PlugIn_ViewPort& VPoint, wxRegion* pValidRegion) {}
wxBitmap* PlugInChartBase::GetThumbnail(int tnx, int tny, int cs) { return nullptr; }
void PlugInChartBase::ComputeSourceRectangle(const PlugIn_ViewPort& vp, wxRect* pSourceRect) {}
double PlugInChartBase::GetRasterScaleFactor() { return 1.0; }
bool PlugInChartBase::GetChartBits(wxRect& source, unsigned char* pPix, int sub_samp) { return false; }
int PlugInChartBase::GetSize_X() { return 0; }
int PlugInChartBase::GetSize_Y() { return 0; }
void PlugInChartBase::latlong_to_chartpix(double lat, double lon, double& pixx, double& pixy) {}
void PlugInChartBase::chartpix_to_latlong(double pixx, double pixy, double* plat, double* plon) {}

// PlugInChartBaseGL stubs (inherits from PlugInChartBase)
PlugInChartBaseGL::PlugInChartBaseGL() {}
PlugInChartBaseGL::~PlugInChartBaseGL() {}
int PlugInChartBaseGL::RenderRegionViewOnGL(const wxGLContext& glc,
                                             const PlugIn_ViewPort& VPoint,
                                             const wxRegion& Region,
                                             bool b_use_stencil) { return 0; }
ListOfPI_S57Obj* PlugInChartBaseGL::GetObjRuleListAtLatLon(float lat,
                                                           float lon,
                                                           float select_radius,
                                                           PlugIn_ViewPort* VPoint) { return nullptr; }
wxString PlugInChartBaseGL::CreateObjDescriptions(ListOfPI_S57Obj* obj_list) { return wxEmptyString; }
int PlugInChartBaseGL::GetNoCOVREntries() { return 0; }
int PlugInChartBaseGL::GetNoCOVRTablePoints(int iTable) { return 0; }
int PlugInChartBaseGL::GetNoCOVRTablenPoints(int iTable) { return 0; }
float* PlugInChartBaseGL::GetNoCOVRTableHead(int iTable) { return nullptr; }

// PlugInChartBaseGLPlus2 stubs
PlugInChartBaseGLPlus2::PlugInChartBaseGLPlus2() {}
PlugInChartBaseGLPlus2::~PlugInChartBaseGLPlus2() {}
ListOfPI_S57Obj* PlugInChartBaseGLPlus2::GetLightsObjRuleListVisibleAtLatLon(float lat,
                                                                               float lon,
                                                                               PlugIn_ViewPort* VPoint) { return nullptr; }

// PlugInChartBaseExtended stubs
PlugInChartBaseExtended::PlugInChartBaseExtended() {}
PlugInChartBaseExtended::~PlugInChartBaseExtended() {}
int PlugInChartBaseExtended::RenderRegionViewOnGL(const wxGLContext& glc,
                                                   const PlugIn_ViewPort& VPoint,
                                                   const wxRegion& Region,
                                                   bool b_use_stencil) { return 0; }
wxBitmap& PlugInChartBaseExtended::RenderRegionViewOnDCNoText(const PlugIn_ViewPort& VPoint,
                                                               const wxRegion& Region) {
    static wxBitmap empty;
    return empty;
}
bool PlugInChartBaseExtended::RenderRegionViewOnDCTextOnly(wxMemoryDC& dc,
                                                            const PlugIn_ViewPort& VPoint,
                                                            const wxRegion& Region) { return false; }
int PlugInChartBaseExtended::RenderRegionViewOnGLNoText(const wxGLContext& glc,
                                                         const PlugIn_ViewPort& VPoint,
                                                         const wxRegion& Region,
                                                         bool b_use_stencil) { return 0; }
int PlugInChartBaseExtended::RenderRegionViewOnGLTextOnly(const wxGLContext& glc,
                                                           const PlugIn_ViewPort& VPoint,
                                                           const wxRegion& Region,
                                                           bool b_use_stencil) { return 0; }
ListOfPI_S57Obj* PlugInChartBaseExtended::GetObjRuleListAtLatLon(float lat,
                                                                  float lon,
                                                                  float select_radius,
                                                                  PlugIn_ViewPort* VPoint) { return nullptr; }
wxString PlugInChartBaseExtended::CreateObjDescriptions(ListOfPI_S57Obj* obj_list) { return wxEmptyString; }
int PlugInChartBaseExtended::GetNoCOVREntries() { return 0; }
int PlugInChartBaseExtended::GetNoCOVRTablePoints(int iTable) { return 0; }
int PlugInChartBaseExtended::GetNoCOVRTablenPoints(int iTable) { return 0; }
float* PlugInChartBaseExtended::GetNoCOVRTableHead(int iTable) { return nullptr; }
void PlugInChartBaseExtended::ClearPLIBTextList() {}

// PlugInChartBaseExtendedPlus2 stubs
PlugInChartBaseExtendedPlus2::PlugInChartBaseExtendedPlus2() {}
PlugInChartBaseExtendedPlus2::~PlugInChartBaseExtendedPlus2() {}
ListOfPI_S57Obj* PlugInChartBaseExtendedPlus2::GetLightsObjRuleListVisibleAtLatLon(float lat,
                                                                                     float lon,
                                                                                     PlugIn_ViewPort* VPoint) { return nullptr; }

// opencpn_plugin base class stubs
// The base class virtual methods need implementations since they're not in the import library
opencpn_plugin::~opencpn_plugin() {}
int opencpn_plugin::Init(void) { return 0; }
bool opencpn_plugin::DeInit(void) { return true; }
int opencpn_plugin::GetAPIVersionMajor() { return 1; }
int opencpn_plugin::GetAPIVersionMinor() { return 18; }
int opencpn_plugin::GetPlugInVersionMajor() { return 1; }
int opencpn_plugin::GetPlugInVersionMinor() { return 0; }
wxBitmap* opencpn_plugin::GetPlugInBitmap() { return nullptr; }
wxString opencpn_plugin::GetCommonName() { return wxEmptyString; }
wxString opencpn_plugin::GetShortDescription() { return wxEmptyString; }
wxString opencpn_plugin::GetLongDescription() { return wxEmptyString; }
void opencpn_plugin::SetDefaults(void) {}
int opencpn_plugin::GetToolbarToolCount(void) { return 0; }
int opencpn_plugin::GetToolboxPanelCount(void) { return 0; }
void opencpn_plugin::SetupToolboxPanel(int page_sel, wxNotebook* pnotebook) {}
void opencpn_plugin::OnCloseToolboxPanel(int page_sel, int ok_apply_cancel) {}
void opencpn_plugin::ShowPreferencesDialog(wxWindow* parent) {}
bool opencpn_plugin::RenderOverlay(wxMemoryDC* pmdc, PlugIn_ViewPort* vp) { return false; }
void opencpn_plugin::SetCursorLatLon(double lat, double lon) {}
void opencpn_plugin::SetCurrentViewPort(PlugIn_ViewPort& vp) {}
void opencpn_plugin::SetPositionFix(PlugIn_Position_Fix& pfix) {}
void opencpn_plugin::SetNMEASentence(wxString& sentence) {}
void opencpn_plugin::SetAISSentence(wxString& sentence) {}
void opencpn_plugin::ProcessParentResize(int x, int y) {}
void opencpn_plugin::SetColorScheme(PI_ColorScheme cs) {}
void opencpn_plugin::OnToolbarToolCallback(int id) {}
void opencpn_plugin::OnContextMenuItemCallback(int id) {}
void opencpn_plugin::UpdateAuiStatus(void) {}
wxArrayString opencpn_plugin::GetDynamicChartClassNameArray(void) { return wxArrayString(); }

// opencpn_plugin_16 stubs
opencpn_plugin_16::opencpn_plugin_16(void* pmgr) : opencpn_plugin(pmgr) {}
opencpn_plugin_16::~opencpn_plugin_16() {}
bool opencpn_plugin_16::RenderOverlay(wxDC& dc, PlugIn_ViewPort* vp) { return false; }
void opencpn_plugin_16::SetPluginMessage(wxString& message_id, wxString& message_body) {}

// opencpn_plugin_17 stubs
opencpn_plugin_17::opencpn_plugin_17(void* pmgr) : opencpn_plugin(pmgr) {}
opencpn_plugin_17::~opencpn_plugin_17() {}
bool opencpn_plugin_17::RenderOverlay(wxDC& dc, PlugIn_ViewPort* vp) { return false; }
bool opencpn_plugin_17::RenderGLOverlay(wxGLContext* pcontext, PlugIn_ViewPort* vp) { return false; }
void opencpn_plugin_17::SetPluginMessage(wxString& message_id, wxString& message_body) {}

// opencpn_plugin_18 stubs
opencpn_plugin_18::opencpn_plugin_18(void* pmgr) : opencpn_plugin(pmgr) {}
opencpn_plugin_18::~opencpn_plugin_18() {}
bool opencpn_plugin_18::RenderOverlay(wxDC& dc, PlugIn_ViewPort* vp) { return false; }
bool opencpn_plugin_18::RenderGLOverlay(wxGLContext* pcontext, PlugIn_ViewPort* vp) { return false; }
void opencpn_plugin_18::SetPluginMessage(wxString& message_id, wxString& message_body) {}
void opencpn_plugin_18::SetPositionFixEx(PlugIn_Position_Fix_Ex& pfix) {}

// opencpn_plugin_19 stubs
opencpn_plugin_19::opencpn_plugin_19(void* pmgr) : opencpn_plugin_18(pmgr) {}
opencpn_plugin_19::~opencpn_plugin_19() {}
void opencpn_plugin_19::OnSetupOptions(void) {}

// opencpn_plugin_110 stubs
opencpn_plugin_110::opencpn_plugin_110(void* pmgr) : opencpn_plugin_19(pmgr) {}
opencpn_plugin_110::~opencpn_plugin_110() {}
void opencpn_plugin_110::LateInit(void) {}

// opencpn_plugin_111 stubs
opencpn_plugin_111::opencpn_plugin_111(void* pmgr) : opencpn_plugin_110(pmgr) {}
opencpn_plugin_111::~opencpn_plugin_111() {}

// opencpn_plugin_112 stubs
opencpn_plugin_112::opencpn_plugin_112(void* pmgr) : opencpn_plugin_111(pmgr) {}
opencpn_plugin_112::~opencpn_plugin_112() {}
bool opencpn_plugin_112::MouseEventHook(wxMouseEvent& event) { return false; }
void opencpn_plugin_112::SendVectorChartObjectInfo(wxString& chart, wxString& feature,
                                                    wxString& objname, double lat, double lon,
                                                    double scale, int nativescale) {}

// opencpn_plugin_113 stubs
opencpn_plugin_113::opencpn_plugin_113(void* pmgr) : opencpn_plugin_112(pmgr) {}
opencpn_plugin_113::~opencpn_plugin_113() {}
bool opencpn_plugin_113::KeyboardEventHook(wxKeyEvent& event) { return false; }
void opencpn_plugin_113::OnToolbarToolDownCallback(int id) {}
void opencpn_plugin_113::OnToolbarToolUpCallback(int id) {}

// opencpn_plugin_114 stubs
opencpn_plugin_114::opencpn_plugin_114(void* pmgr) : opencpn_plugin_113(pmgr) {}
opencpn_plugin_114::~opencpn_plugin_114() {}

// opencpn_plugin_115 stubs
opencpn_plugin_115::opencpn_plugin_115(void* pmgr) : opencpn_plugin_114(pmgr) {}
opencpn_plugin_115::~opencpn_plugin_115() {}

// opencpn_plugin_116 stubs
opencpn_plugin_116::opencpn_plugin_116(void* pmgr) : opencpn_plugin_115(pmgr) {}
opencpn_plugin_116::~opencpn_plugin_116() {}
bool opencpn_plugin_116::RenderGLOverlayMultiCanvas(wxGLContext* pcontext, PlugIn_ViewPort* vp,
                                                     int canvasIndex) { return false; }
bool opencpn_plugin_116::RenderOverlayMultiCanvas(wxDC& dc, PlugIn_ViewPort* vp,
                                                   int canvasIndex) { return false; }
void opencpn_plugin_116::PrepareContextMenu(int canvasIndex) {}

// opencpn_plugin_117 stubs
opencpn_plugin_117::opencpn_plugin_117(void* pmgr) : opencpn_plugin_116(pmgr) {}
int opencpn_plugin_117::GetPlugInVersionPatch() { return 0; }
int opencpn_plugin_117::GetPlugInVersionPost() { return 0; }
const char* opencpn_plugin_117::GetPlugInVersionPre() { return ""; }
const char* opencpn_plugin_117::GetPlugInVersionBuild() { return ""; }
void opencpn_plugin_117::SetActiveLegInfo(Plugin_Active_Leg_Info& leg_info) {}

// opencpn_plugin_118 stubs
opencpn_plugin_118::opencpn_plugin_118(void* pmgr) : opencpn_plugin_117(pmgr) {}
bool opencpn_plugin_118::RenderGLOverlayMultiCanvas(wxGLContext* pcontext, PlugIn_ViewPort* vp,
                                                     int canvasIndex, int priority) { return false; }
bool opencpn_plugin_118::RenderOverlayMultiCanvas(wxDC& dc, PlugIn_ViewPort* vp,
                                                   int canvasIndex, int priority) { return false; }

// OCPN_downloadEvent stubs
OCPN_downloadEvent::OCPN_downloadEvent(wxEventType commandType, int id)
    : wxEvent(id, commandType), m_stat(OCPN_DL_UNKNOWN), m_condition(OCPN_DL_EVENT_TYPE_UNKNOWN),
      m_totalBytes(0), m_sofarBytes(0), m_b_complete(false) {}
OCPN_downloadEvent::~OCPN_downloadEvent() {}
wxEvent* OCPN_downloadEvent::Clone() const { return new OCPN_downloadEvent(*this); }

// wxEVT_DOWNLOAD_EVENT
wxEventType wxEVT_DOWNLOAD_EVENT = wxNewEventType();

// OpenCPN API callback functions - these are resolved at runtime when loaded by OpenCPN
// On Windows we need stubs to satisfy the linker
extern "C" {

int InsertPlugInTool(wxString label, wxBitmap* bitmap, wxBitmap* bmpRollover,
                     wxItemKind kind, wxString shortHelp, wxString longHelp,
                     wxObject* clientData, int position, int tool_sel,
                     opencpn_plugin* pplugin) { return -1; }

void RemovePlugInTool(int tool_id) {}
void SetToolbarToolViz(int item, bool viz) {}
void SetToolbarItemState(int item, bool toggle) {}
void SetToolbarToolBitmaps(int item, wxBitmap* bitmap, wxBitmap* bmpRollover) {}

int InsertPlugInToolSVG(wxString label, wxString SVGfile, wxString SVGfileRollover,
                        wxString SVGfileToggled, wxItemKind kind, wxString shortHelp,
                        wxString longHelp, wxObject* clientData, int position,
                        int tool_sel, opencpn_plugin* pplugin) { return -1; }

void SetToolbarToolBitmapsSVG(int item, wxString SVGfile,
                               wxString SVGfileRollover, wxString SVGfileToggled) {}

int AddCanvasContextMenuItem(wxMenuItem* pitem, opencpn_plugin* pplugin) { return -1; }
void RemoveCanvasContextMenuItem(int item) {}
void SetCanvasContextMenuItemViz(int item, bool viz) {}
void SetCanvasContextMenuItemGrey(int item, bool grey) {}

wxFileConfig* GetOCPNConfigObject(void) { return nullptr; }
void RequestRefresh(wxWindow*) {}
bool GetGlobalColor(wxString colorName, wxColour* pcolour) { return false; }

void GetCanvasPixLL(PlugIn_ViewPort* vp, wxPoint* pp, double lat, double lon) {
    if (pp) { pp->x = 0; pp->y = 0; }
}
void GetCanvasLLPix(PlugIn_ViewPort* vp, wxPoint p, double* plat, double* plon) {
    if (plat) *plat = 0;
    if (plon) *plon = 0;
}

wxWindow* GetOCPNCanvasWindow() { return nullptr; }
wxFont* OCPNGetFont(wxString TextElement, int default_size) { return nullptr; }
wxString* GetpSharedDataLocation() { return nullptr; }
ArrayOfPlugIn_AIS_Targets* GetAISTargetArray(void) { return nullptr; }
wxAuiManager* GetFrameAuiManager(void) { return nullptr; }
bool AddLocaleCatalog(wxString catalog) { return false; }
void PushNMEABuffer(wxString str) {}
void SendPluginMessage(wxString message_id, wxString message_body) {}
void DimeWindow(wxWindow*) {}
void JumpToPosition(double lat, double lon, double scale) {}

void PositionBearingDistanceMercator_Plugin(double lat, double lon, double brg,
                                             double dist, double* dlat, double* dlon) {
    if (dlat) *dlat = lat;
    if (dlon) *dlon = lon;
}
void DistanceBearingMercator_Plugin(double lat0, double lon0, double lat1,
                                     double lon1, double* brg, double* dist) {
    if (brg) *brg = 0;
    if (dist) *dist = 0;
}
double DistGreatCircle_Plugin(double slat, double slon, double dlat, double dlon) { return 0; }

void toTM_Plugin(float lat, float lon, float lat0, float lon0, double* x, double* y) {
    if (x) *x = 0;
    if (y) *y = 0;
}
void fromTM_Plugin(double x, double y, double lat0, double lon0, double* lat, double* lon) {
    if (lat) *lat = lat0;
    if (lon) *lon = lon0;
}
void toSM_Plugin(double lat, double lon, double lat0, double lon0, double* x, double* y) {
    if (x) *x = 0;
    if (y) *y = 0;
}
void fromSM_Plugin(double x, double y, double lat0, double lon0, double* lat, double* lon) {
    if (lat) *lat = lat0;
    if (lon) *lon = lon0;
}
void toSM_ECC_Plugin(double lat, double lon, double lat0, double lon0, double* x, double* y) {
    if (x) *x = 0;
    if (y) *y = 0;
}
void fromSM_ECC_Plugin(double x, double y, double lat0, double lon0, double* lat, double* lon) {
    if (lat) *lat = lat0;
    if (lon) *lon = lon0;
}

bool DecodeSingleVDOMessage(const wxString& str, PlugIn_Position_Fix_Ex* pos, wxString* acc) { return false; }
int GetChartbarHeight(void) { return 0; }
bool GetActiveRoutepointGPX(char* buffer, unsigned int buffer_length) { return false; }

}  // extern "C"

// C++ linkage API functions
wxString GetPluginDataDir(const char* plugin_name) { return wxEmptyString; }
wxScrolledWindow* AddOptionsPage(OptionsParentPI parent, wxString title) { return nullptr; }
bool DeleteOptionsPage(wxScrolledWindow* page) { return false; }

#endif  // __WXMSW__
