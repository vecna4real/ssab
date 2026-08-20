#!/usr/bin/env bash
# build.sh - Build SSAB VST3 + Standalone on macOS / Linux
# Usage: ./Scripts/build.sh          (VST3 + Standalone)
#        ./Scripts/build.sh --vst2 /path/to/VST2_SDK   (also builds VST2)
#        ./Scripts/build.sh --clean
set -e

cd "$(dirname "$0")/.."

ENABLE_VST2=0
VST2_SDK=""
CLEAN=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --vst2) ENABLE_VST2=1; VST2_SDK="$2"; shift 2;;
        --clean) CLEAN=1; shift;;
        *) echo "Unknown arg: $1"; exit 1;;
    esac
done

if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: cmake not found. Install CMake 3.22+ first."
    exit 1
fi

if [[ "$ENABLE_VST2" == "1" ]]; then
    if [[ ! -d "$VST2_SDK" ]]; then
        echo "ERROR: VST2 SDK path not found: $VST2_SDK"
        exit 1
    fi
    # patch CMakeLists.txt
    sed -i.bak 's/FORMATS  VST3 Standalone/FORMATS  VST VST3 Standalone/' CMakeLists.txt
    sed -i.bak "s|juce_add_plugin(SSAB|juce_set_vst2_sdk_path(\"$VST2_SDK\")\njuce_add_plugin(SSAB|" CMakeLists.txt
fi

if [[ "$CLEAN" == "1" ]] && [[ -d build ]]; then
    rm -rf build
fi

mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j

echo ""
echo "================================================="
echo "  BUILD COMPLETE"
echo "================================================="
find . -name "*.vst3" -o -name "*.dll" -o -name "SSAB" | grep -v build/CMakeFiles
