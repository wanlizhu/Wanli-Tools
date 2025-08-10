#!/usr/bin/env bash
set -uo pipefail

rootDir=/media/wanliz/data/wanliz-sw-gpu-driver-home
outputDir=$rootDir/rel/gpu_drv/r580/r580_00/_out/Linux_amd64_develop
buildScript=$HOME/WZhu/Scripts/NvMake.sh

Cleanup-and-Restore() {
    echo "Cleanup and restore *-hide-and-checking dirs"
    find "$rootDir" -type d -name '*-hide-and-checking' \
        -exec bash -c 'for d; do mv "$d" "${d%-hide-and-checking}"; done' _ {} +
    exit 
}
trap Cleanup-and-Restore EXIT INT TERM

Hide-Dir-and-Test-Build() {
    dirPath="$1"
    if [[ "$dirPath" == *"_out"* || 
          "$dirPath" == *"-hide-and-checked"* ||
          "$dirPath" == *"-hide-and-checking"* ]]; then
        return 0
    fi

    export P4ROOT=$rootDir
    sudo rm -rf "$outputDir"
    mv "$dirPath" "$dirPath-hide-and-checking"
    bash $buildScript sweep <<< "" 
    bash $buildScript <<< "" 

    echo "[$(date)] Testing $dirPath" >> prune.log
    if [[ ! -z $(ls "$outputDir" | grep 'internal.run') ]]; then
        echo "[$(date)] >> This folder is NOT needed" >> prune.log 
        mv "$dirPath-hide-and-checking" "$dirPath-hide-and-checked"
    else
        echo "[$(date)] >> This folder is needed" >> prune.log 
        mv "$dirPath-hide-and-checking" "$dirPath"
        Process-Children-Dirs "$dirPath"
    fi
    echo >> prune.log 
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
echo "Output Dir   : $outputDir"
echo "Build Script : $buildScript"
read -p "Press [Enter] to continue: "

rm -rf prune.log 
Process-Children-Dirs "$rootDir"

