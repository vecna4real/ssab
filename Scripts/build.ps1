# build.ps1 - Build SSAB VST2/VST3 on Windows (PowerShell)
# Usage: powershell -ExecutionPolicy Bypass -File build.ps1
# Optional args: -EnableVST2 -VST2SdkPath "C:\path\to\VST2_SDK"

param(
    [switch]$EnableVST2,
    [switch]$DisableVST2,
    [string]$VST2SdkPath = $env:JUCE_VST2_SDK_DIR,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
Set-Location -Path $PSScriptRoot

Write-Host "=================================================" -ForegroundColor Yellow
Write-Host "  SSAB // EXTREME BASS SYNTH // BUILD SCRIPT" -ForegroundColor Yellow
Write-Host "=================================================" -ForegroundColor Yellow

# --- 1. Sanity check: cmake + VS ---
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    Write-Host "ERROR: cmake not found in PATH." -ForegroundColor Red
    Write-Host "Install CMake 3.22+ from https://cmake.org/download/ and add it to PATH."
    exit 1
}

# --- 2. Configure VST2 ---
$enableVst2Flag = "ON"
if ($DisableVST2) {
    $enableVst2Flag = "OFF"
    Write-Host "VST2 build DISABLED by -DisableVST2 flag." -ForegroundColor Yellow
} elseif ($EnableVST2) {
    if (-not $VST2SdkPath -or -not (Test-Path $VST2SdkPath)) {
        Write-Host "Note: -EnableVST2 ignored (no VST2 SDK path given)." -ForegroundColor Yellow
        Write-Host "SSAB CMakeLists.txt already downloads VST2 stub headers via FetchContent." -ForegroundColor Yellow
        Write-Host "Just run build.ps1 without -EnableVST2 to get VST2 built automatically." -ForegroundColor Yellow
    }
}
# (VST2 stub headers are auto-downloaded by CMake FetchContent —
# no manual SDK needed anymore. The -EnableVST2 flag is kept for
# backwards compatibility but is essentially a no-op now.)

# --- 3. Configure ---
if ($Clean -and (Test-Path build)) {
    Write-Host "Cleaning build/ ..." -ForegroundColor Cyan
    Remove-Item -Recurse -Force build
}

New-Item -ItemType Directory -Force -Path build | Out-Null
Set-Location -Path build

Write-Host "Configuring (CMake)..." -ForegroundColor Cyan
& cmake .. -G "Visual Studio 17 2022" -A x64 -DSSAB_ENABLE_VST2=$enableVst2Flag
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configure failed." -ForegroundColor Red
    exit 1
}

# --- 4. Build ---
Write-Host "Building (Release)..." -ForegroundColor Cyan
& cmake --build . --config Release -j
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed." -ForegroundColor Red
    exit 1
}

# --- 5. Show output location ---
Set-Location -Path $PSScriptRoot
$artefacts = Join-Path "build" "SSAB_artefacts\Release"
Write-Host ""
Write-Host "=================================================" -ForegroundColor Green
Write-Host "  BUILD COMPLETE!" -ForegroundColor Green
Write-Host "=================================================" -ForegroundColor Green
Write-Host "Artefacts in: $artefacts" -ForegroundColor White
Get-ChildItem -Path $artefacts -Recurse -Include *.dll,*.vst3,*.exe -ErrorAction SilentlyContinue |
    ForEach-Object { Write-Host "  - $($_.FullName)" -ForegroundColor Gray }

Write-Host ""
Write-Host "Install locations:" -ForegroundColor Yellow
Write-Host "  VST3 (64):   $env:COMMONPROGRAMFILES\VST3\"
if ($enableVst2Flag -eq "ON") {
    Write-Host "  VST2 (64):   C:\Program Files\VstPlugins\" -ForegroundColor White
    Write-Host "  (LMMS scans this by default — just drop SSAB.dll there)"
}
