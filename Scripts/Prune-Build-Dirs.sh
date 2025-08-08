#!/usr/bin/env bash
set -uo pipefail

rootDir=/media/wanliz/data/wanliz-sw-gpu-driver-home
outputFile=/media/wanliz/data/wanliz-sw-gpu-driver-home/rel/gpu_drv/r580/r580_00/_out/Linux_amd64_develop/NVIDIA-Linux-x86_64-*-internal.run
buildScript=$HOME/WZhu/Scripts/NvMake.sh

Cleanup-and-Restore() {
    echo "Cleanup and restore *-hide-and-checking dirs"
    find "$rootDir" -type d -name '*-hide-and-checking' \
        -exec bash -c 'for d; do mv "$d" "${d%-hide-and-checking}"; done' _ {} +
}
trap Cleanup-and-Restore EXIT INT TERM

Run-Build-Script() {
    export P4ROOT=$rootDir
    bash $buildScript sweep <<< "" 
    bash $buildScript <<< "" 
    if (( $? != 0 )) || [[ -z $(ls $outputFile 2>/dev/null) ]]; then return 1; fi
    return 0
}

Hide-Dir-and-Test-Build() {
    dirPath="$1"
    if [[ "$dirPath" == *"_out"* || 
          "$dirPath" == *"-hide-and-checked"* ||
          "$dirPath" == *"-hide-and-checking"* ]]; then
        return 0
    fi

    mv "$dirPath" "$dirPath-hide-and-checking"

    if Run-Build-Script; then
        mv "$dirPath-hide-and-checking" "$dirPath-hide-and-checked"
        echo "[Found] $dirPath" | tee -a hide-and-checked.txt
    else
        mv "$dirPath-hide-and-checking" "$dirPath"
        Process-Children-Dirs "$dirPath"
    fi
}

Process-Children-Dirs() {
    shopt -s nullglob
    for child in $1/*/; do
        child="${child%/}"
        [[ ! -d "$child" || 
            $child == *"_out"* || 
            $child == *"-hide-and-checked"* ||
            $child == *"-hide-and-checking"* ]] && continue
        Hide-Dir-and-Test-Build "$child"
    done
}

echo "=== Starting Reduction ==="
echo "Root Dir     : $rootDir"
echo "Output File  : $outputFile"
echo "Build Script : $buildScript"
read -p "Press [Enter] to continue: "

rm -rf hide-and-checked.txt
Process-Children-Dirs "$rootDir"
