#!/bin/bash

if [[ ! -z $P4ROOT ]]; then 
    codeBaseDir=$P4ROOT 
elif [[ $HOSTNAME == wanliz-sc-ubuntu24 ]]; then 
    codeBaseDir=/media/wanliz/data/wanliz-sw-gpu-driver-office
elif [[ $HOSTNAME == Wanliz-WorkMachine ]]; then 
    codeBaseDir=$HOME/wanliz-sw-gpu-driver-home
else
    echo "P4ROOT is not defined, aborting!"
    exit 1
fi 

branchName=rel/gpu_drv/r580/r580_00
moduleName="drivers dist"
moduleDir=""  
targetArch=amd64 
targetConfig=develop
builderThreads=$(nproc)

echo "WorkDir: $codeBaseDir/$branchName/$moduleDir"
echo "   Args: $moduleName linux $buildArch $targetConfig $builderThreads"
read -p "Press [Enter] to continue or [c] to change: " userInput
if [[ $userInput == c || $userInput == C ]]; then 
    read -e -i "$codeBaseDir" -p "Code Base Dir: " codeBaseDir
    read -e -i "$branchName" -p "Branch Name: " branchName
    read -e -i "$moduleName" -p "Module Name: " moduleName
    read -e -i "$moduleDir" -p "Module Dir: " moduleDir
    read -e -i "$targetArch" -p "Target Arch: " targetArch
    read -e -i "$targetConfig" -p "Target Config: " targetConfig
    read -e -i "$builderThreads" -p "Builder Threads: " builderThreads
fi 

commandLine="cd $codeBaseDir/$branchName/$moduleDir && $codeBaseDir/tools/linux/unix-build/unix-build --tools $codeBaseDir/tools --devrel $codeBaseDir/devrel/SDK/inc/GL --unshare-namespaces nvmake NV_COLOR_OUTPUT=1 NV_GUARDWORD= NV_COMPRESS_THREADS=$(nproc) NV_FAST_PACKAGE_COMPRESSION=zstd NV_USE_FRAME_POINTER=1 NV_UNIX_LTO_ENABLED= NV_MANGLE_SYMBOLS= NV_TRACE_CODE=1 $moduleName linux $targetArch $targetConfig $builderThreads -time"
echo -e "\n${commandLine}\n"

if [[ $1 == -dr ]]; then
    exit 
fi 

pushd . >/dev/null || exit 1
eval "$commandLine"
popd >/dev/null   
