#!/usr/bin/env bash
set -uo pipefail

rootDir=/media/wanliz/data/wanliz-sw-gpu-driver-home
outputDir=$rootDir/rel/gpu_drv/r580/r580_00/_out/Linux_amd64_develop
buildScript=$HOME/WZhu/Scripts/NvMake.sh

if [[ "$1" == print ]]; then 
    find "$rootDir" -type f -name ".nvmake.deps"
    find "$rootDir" -type d -name *-hide-and-checked | while read -r line; do  du -sh $line; done | sort -h
    exit 
fi 

Cleanup-and-Restore() {
    echo "Please restore *-hide-and-checking dirs:"
    find "$rootDir" -type d -name '*-hide-and-checking' | tee /tmp/log
    exit 
}
trap Cleanup-and-Restore EXIT INT TERM

Hide-Dir-and-Test-Build() {
    dirPath="$1"
    if [[ "$dirPath" == *"_out"* || 
          "$dirPath" == *"-hide-and-checked"* ||
          "$dirPath" == *"-hide-and-checking"* ]]; then
        echo "[$(date)] Ignoring $dirPath"
    elif [[ -f "$dirPath/.nvmake.deps" ]]; then 
        echo "[$(date)] $dirPath is a known dependency" >> prune.log 
        Process-Children-Dirs "$dirPath"
    elif (( $(du -sb "$dirPath" | cut -f1) < 1024 * 1024 * 100 )); then 
        echo "[$(date)] $dirPath is smaller than threshold, ignoring"
    else 
        export P4ROOT=$rootDir
        sudo rm -rf "$outputDir"

        echo "[$(date)] Testing $dirPath" >> prune.log
        mv "$dirPath" "$dirPath-hide-and-checking"
        bash $buildScript sweep <<< "" 
        bash $buildScript <<< "" 

        if [[ ! -z $(ls "$outputDir" | grep 'internal.run') ]]; then
            echo "[$(date)] >> This folder is NOT needed" >> prune.log 
            mv "$dirPath-hide-and-checking" "$dirPath-hide-and-checked"
        else
            echo "[$(date)] >> This folder is needed" >> prune.log 
            mv "$dirPath-hide-and-checking" "$dirPath"
            echo 1 > "$dirPath/.nvmake.deps"
            Process-Children-Dirs "$dirPath"
        fi

        echo "[$(date)] Testing Finished" >> prune.log 
    fi 
}

Process-Children-Dirs() {
    shopt -s nullglob
    for child in $(du -sb $1/* 2>/dev/null | sort -nr | cut -f2); do
        child="${child%/}"
        [[ ! -d "$child" ]] && continue
        Hide-Dir-and-Test-Build "$child"
    done
}

echo "=== Starting Reduction ==="
echo "Root Dir     : $rootDir"
echo "Output Dir   : $outputDir"
echo "Build Script : $buildScript"
read -p "Press [Enter] to continue: "

rm -rf prune.log 
Process-Children-Dirs "$rootDir/tools"
Process-Children-Dirs "$rootDir/sdk"
Process-Children-Dirs "$rootDir/common"
Process-Children-Dirs "$rootDir/compiler"
Process-Children-Dirs "$rootDir/gpgpu"
Process-Children-Dirs "$rootDir/resman"
Process-Children-Dirs "$rootDir/rtcore"
Process-Children-Dirs "$rootDir/xfree86"
Process-Children-Dirs "$rootDir"

