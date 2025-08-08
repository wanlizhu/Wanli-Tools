#!/usr/bin/env bash
set -uo pipefail

rootDir=/media/wanliz/data/wanliz-sw-gpu-driver-home
outputFile=/media/wanliz/data/wanliz-sw-gpu-driver-home/rel/gpu_drv/r580/r580_00/_out/Linux_amd64_develop/NVIDIA-Linux-x86_64-*-internal.run
buildScript=$HOME/WZhu/Scripts/NvMake.sh

Run-Build-Script() {
    export P4ROOT=$rootDir
    bash $buildScript sweep <<< "" &>/dev/null 
    bash $buildScript <<< "" &>/dev/null 
    if (( $? != 0 )) || [[ -z $(ls $outputFile 2>/dev/null) ]]; then return 1; fi
    return 0
}

Hide-Dir-And-Test-Build() {
    dirPath=$1
    if [[ $dirPath == *"_out"* || 
        $dirPath == *"-tobe-removed"* ]]; then
        return 0
    fi

    mv $dirPath $dirPath-tobe-removed 

    if Run-Build-Script; then
        echo "[Found] $dirPath"
    else
        mv $dirPath-tobe-removed $dirPath
        Process-Children-Dirs $dirPath
    fi
}

Process-Children-Dirs() {
    shopt -s nullglob
    for child in $1/*/; do
        child="${child%/}"
        [[ ! -d "$child" || 
            $child == *"_out"* ||
            $child == *"-tobe-removed"* ]] && continue
        Hide-Dir-And-Test-Build "$child"
    done
}

echo "=== Starting Reduction ==="
echo "Root Dir     : $rootDir"
echo "Output File  : $outputFile"
echo "Build Script : $buildScript"
read -p "Press [Enter] to continue: "

Process-Children-Dirs $rootDir
