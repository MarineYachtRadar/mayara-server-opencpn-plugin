# Generate opencpn.lib from OpenCPN 5.12.4 exports
# Run from Developer Command Prompt or after calling vcvars32.bat

$opencpnExe = "C:\Program Files (x86)\OpenCPN\opencpn.exe"
$outputDir = "c:\GitHub\mayara-server-opencpn-plugin"

Write-Host "Extracting exports from OpenCPN..."

# Run dumpbin to get exports
$dumpbinPath = "dumpbin"
$rawExports = & $dumpbinPath /exports $opencpnExe 2>&1

# Parse the exports
$exports = @()
$inExports = $false
foreach ($line in $rawExports) {
    if ($line -match "^\s+ordinal\s+hint\s+RVA\s+name") {
        $inExports = $true
        continue
    }
    if ($inExports) {
        if ($line -match "^\s+Summary") {
            break
        }
        # Parse: ordinal hint RVA name
        if ($line -match "^\s+\d+\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+(\S+)") {
            $exports += $Matches[1]
        }
    }
}

Write-Host "Found $($exports.Count) exports"

# Create .def file
$defContent = @"
LIBRARY opencpn.exe
EXPORTS
"@

foreach ($exp in $exports) {
    $defContent += "`n    $exp"
}

$defFile = Join-Path $outputDir "opencpn_512.def"
$defContent | Out-File -FilePath $defFile -Encoding ASCII

Write-Host "Created $defFile"
Write-Host ""
Write-Host "Now run in Developer Command Prompt:"
Write-Host "  lib /def:opencpn_512.def /out:opencpn_512.lib /machine:x86"
