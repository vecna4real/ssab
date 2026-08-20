# build.ps1 - Build SSAB VST2/VST3 on Windows (PowerShell)
# Usage:
#   .\Scripts\build.ps1                  # default: auto-detect VS, VST2 enabled
#   .\Scripts\build.ps1 -DisableVST2     # build only VST3 + Standalone
#   .\Scripts\build.ps1 -UseNinja        # use Ninja generator (faster, no VS solution)
#   .\Scripts\build.ps1 -Clean            # delete build/ before configuring
#   .\Scripts\build.ps1 -Verbose          # verbose cmake output
#
# The script auto-detects the installed Visual Studio version via
# vswhere.exe and picks the correct CMake generator name
# (e.g. "Visual Studio 17 2022" or "Visual Studio 18 2026").

param(
    [switch]$DisableVST2,
    [switch]$UseNinja,
    [switch]$Clean,
    [switch]$Verbose,
    [string]$VSGenerator     # optional override, e.g. "Visual Studio 17 2022"
)

$ErrorActionPreference = "Stop"
Set-Location -Path $PSScriptRoot
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

Write-Host "=================================================" -ForegroundColor Yellow
Write-Host "  SSAB // EXTREME BASS SYNTH // BUILD SCRIPT" -ForegroundColor Yellow
Write-Host "=================================================" -ForegroundColor Yellow

# --- 1. Sanity check: cmake ---
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    Write-Host "ERROR: cmake not found in PATH." -ForegroundColor Red
    Write-Host "Install CMake 3.22+ from https://cmake.org/download/ and add it to PATH." -ForegroundColor Yellow
    Write-Host "Or: open Visual Studio Installer -> 'Desktop development with C++' -> ensure 'C++ CMake tools for Windows' is checked." -ForegroundColor Yellow
    exit 1
}

Write-Host "CMake found at: $($cmake.Source)" -ForegroundColor Cyan
& cmake --version

# --- 2. VST2 flag ---
$enableVst2Flag = "ON"
if ($DisableVST2) {
    $enableVst2Flag = "OFF"
    Write-Host "VST2 build DISABLED by -DisableVST2 flag." -ForegroundColor Yellow
}

# --- 3. Choose generator ---
$generator = ""
$generatorArgs = @()

if ($UseNinja) {
    Write-Host "Using Ninja generator (requested via -UseNinja)." -ForegroundColor Cyan
    $ninja = Get-Command ninja -ErrorAction SilentlyContinue
    if (-not $ninja) {
        Write-Host "ERROR: Ninja not found in PATH." -ForegroundColor Red
        Write-Host "Install via:" -ForegroundColor Yellow
        Write-Host "  choco install ninja   OR   pip install ninja" -ForegroundColor White
        exit 1
    }
    $generator = "-G"
    $generatorArgs = @("Ninja", "-DCMAKE_BUILD_TYPE=Release")
} else {
    # Auto-detect Visual Studio via vswhere.exe
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

    if ($VSGenerator) {
        # Explicit override
        $generatorName = $VSGenerator
        Write-Host "Using user-specified generator: $generatorName" -ForegroundColor Cyan
    } elseif (Test-Path $vswhere) {
        # Query vswhere for the latest VS installation
        $vsVersion = & $vswhere -latest -property installationVersion
        $vsPath    = & $vswhere -latest -property installationPath

        if ($vsVersion) {
            $vsMajor = [int]::Parse($vsVersion.Split('.')[0])

            # Known VS major -> year mapping.
            # Microsoft breaks the "major + 2005" pattern starting at VS 18
            # (17->2022, 18->2026), so use an explicit map.
            $yearMap = @{
                14 = 2015
                15 = 2017
                16 = 2019
                17 = 2022
                18 = 2026
                19 = 2029
                20 = 2032
            }
            if ($yearMap.ContainsKey($vsMajor)) {
                $vsYear = $yearMap[$vsMajor]
            } else {
                # Fallback: try major + 2005 (might be wrong, but worth a shot)
                $vsYear = $vsMajor + 2005
                Write-Host "WARNING: VS major $vsMajor not in known map. Guessing $vsYear." -ForegroundColor Yellow
            }

            $generatorName = "Visual Studio $vsMajor $vsYear"

            Write-Host "Visual Studio detected:" -ForegroundColor Cyan
            Write-Host "  Version:    $vsVersion" -ForegroundColor Gray
            Write-Host "  Major:      $vsMajor" -ForegroundColor Gray
            Write-Host "  Path:       $vsPath" -ForegroundColor Gray
            Write-Host "  Generator:  $generatorName" -ForegroundColor Gray

            # Sanity check: does CMake actually know this generator?
            $helpOutput = cmake --help | Out-String
            if ($helpOutput -notmatch [regex]::Escape($generatorName)) {
                Write-Host "WARNING: CMake does not list generator '$generatorName'." -ForegroundColor Yellow
                Write-Host "Falling back to 'Visual Studio 17 2022'." -ForegroundColor Yellow
                $generatorName = "Visual Studio 17 2022"
                if ($helpOutput -notmatch [regex]::Escape($generatorName)) {
                    Write-Host "WARNING: VS 2022 generator also missing. Using Ninja." -ForegroundColor Yellow
                    $generatorName = "Ninja"
                }
            }
        } else {
            Write-Host "ERROR: vswhere didn't return a Visual Studio version." -ForegroundColor Red
            Write-Host "Is Visual Studio installed with the C++ workload?" -ForegroundColor Yellow
            exit 1
        }
    } else {
        Write-Host "ERROR: vswhere.exe not found." -ForegroundColor Red
        Write-Host "Visual Studio 2022 or later must be installed." -ForegroundColor Yellow
        Write-Host "Download from: https://visualstudio.microsoft.com/downloads/" -ForegroundColor Yellow
        Write-Host "Workload: 'Desktop development with C++'" -ForegroundColor Yellow
        exit 1
    }

    $generator = "-G"
    $generatorArgs = @($generatorName, "-A", "x64")
}

# --- 4. Clean if requested ---
if ($Clean -and (Test-Path build)) {
    Write-Host "Cleaning build/ ..." -ForegroundColor Cyan
    Remove-Item -Recurse -Force build
}

# --- 5. Configure ---
New-Item -ItemType Directory -Force -Path build | Out-Null

Write-Host ""
Write-Host "Configuring (CMake)..." -ForegroundColor Cyan
$configureArgs = @("-S", ".", "-B", "build", $generator) + $generatorArgs + @("-DSSAB_ENABLE_VST2=$enableVst2Flag")
if ($Verbose) {
    $configureArgs += "--debug-output"
}
Write-Host "cmake $($configureArgs -join ' ')" -ForegroundColor DarkGray
Write-Host ""
& cmake @configureArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "ERROR: CMake configure failed (exit code $LASTEXITCODE)." -ForegroundColor Red
    Write-Host ""
    Write-Host "Common causes:" -ForegroundColor Yellow
    Write-Host "  1. Visual Studio with C++ workload is not installed." -ForegroundColor Yellow
    Write-Host "     -> Install: https://visualstudio.microsoft.com/downloads/" -ForegroundColor Yellow
    Write-Host "     -> Workload: 'Desktop development with C++'" -ForegroundColor Yellow
    Write-Host "  2. CMake is too old (need 3.22+)." -ForegroundColor Yellow
    Write-Host "     -> Update: https://cmake.org/download/" -ForegroundColor Yellow
    Write-Host "  3. No internet (JUCE download fails)." -ForegroundColor Yellow
    Write-Host "     -> Check connectivity, rerun script." -ForegroundColor Yellow
    Write-Host "  4. Wrong VS generator for your VS version." -ForegroundColor Yellow
    Write-Host "     -> Try: .\Scripts\build.ps1 -UseNinja" -ForegroundColor White
    Write-Host "     -> Or override: .\Scripts\build.ps1 -VSGenerator 'Visual Studio 17 2022'" -ForegroundColor White
    exit 1
}

# --- 6. Build ---
Write-Host ""
Write-Host "Building (Release)..." -ForegroundColor Cyan
& cmake --build build --config Release --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "ERROR: Build failed (exit code $LASTEXITCODE)." -ForegroundColor Red
    Write-Host "See the compiler output above for details." -ForegroundColor Yellow
    exit 1
}

# --- 7. Show output location ---
$artefacts = Join-Path "build" "SSAB_artefacts\Release"
Write-Host ""
Write-Host "=================================================" -ForegroundColor Green
Write-Host "  BUILD COMPLETE!" -ForegroundColor Green
Write-Host "=================================================" -ForegroundColor Green
Write-Host "Artefacts in: $artefacts" -ForegroundColor White
Get-ChildItem -Path $artefacts -Recurse -Include *.dll,*.vst3,*.exe -ErrorAction SilentlyContinue |
    ForEach-Object {
        $rel = $_.FullName.Replace((Get-Location).Path + "\", "")
        Write-Host "  - $rel ($($_.Length) bytes)" -ForegroundColor Gray
    }

Write-Host ""
Write-Host "Install locations:" -ForegroundColor Yellow
Write-Host "  VST3 (64):   $env:COMMONPROGRAMFILES\VST3\" -ForegroundColor White
if ($enableVst2Flag -eq "ON") {
    Write-Host "  VST2 (64):   C:\Program Files\VstPlugins\" -ForegroundColor White
    Write-Host "  (LMMS scans this by default — just drop SSAB.dll there)" -ForegroundColor DarkGray
}

Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Copy SSAB.dll to your LMMS VST folder" -ForegroundColor White
Write-Host "  2. Open LMMS -> Vestige -> select SSAB.dll" -ForegroundColor White
Write-Host "  3. The SSAB GUI should appear with all 7 sections." -ForegroundColor White
