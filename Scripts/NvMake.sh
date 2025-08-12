#!/bin/bash

if [[ ! -z $P4ROOT ]]; then 
    codeBaseDir=$P4ROOT 
elif [[ $HOSTNAME == wanliz-sc-ubuntu24 ]]; then 
    codeBaseDir=/media/wanliz/data/wanliz-sw-gpu-driver-office
elif [[ $HOSTNAME == Wanlis-WorkMachine ]]; then 
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

[[ $1 == sweep ]] && { moduleName="sweep"; moduleDir=""; }
[[ $1 == opengl ]] && { moduleName="opengl"; moduleDir="drivers/OpenGL"; }

echo "WorkDir: $codeBaseDir/$branchName/$moduleDir"
echo "   Args: $moduleName linux $targetArch $targetConfig $builderThreads"
read -p "Press [Enter] to continue or [c] to change: " userInput
if [[ $userInput == c || $userInput == C ]]; then 
    read -p "Code Base Dir  : " -e -i "$codeBaseDir" codeBaseDir
    read -p "Branch Name    : " -e -i "$branchName" branchName
    read -p "Module Name    : " -e -i "$moduleName" moduleName
    read -p "Module Dir     : " -e -i "$moduleDir" moduleDir
    read -p "Target Arch    : " -e -i "$targetArch" targetArch
    read -p "Target Config  : " -e -i "$targetConfig" targetConfig
    read -p "Builder Threads: " -e -i "$builderThreads" builderThreads
fi 

commandLine="cd $codeBaseDir/$branchName/$moduleDir && $codeBaseDir/tools/linux/unix-build/unix-build --tools $codeBaseDir/tools --devrel $codeBaseDir/devrel/SDK/inc/GL --unshare-namespaces nvmake NV_COLOR_OUTPUT=1 NV_GUARDWORD= NV_COMPRESS_THREADS=$(nproc) NV_FAST_PACKAGE_COMPRESSION=zstd NV_USE_FRAME_POINTER=1 NV_UNIX_LTO_ENABLED= NV_MANGLE_SYMBOLS= NV_TRACE_CODE=1 -time $moduleName linux $targetArch $targetConfig -j$builderThreads"
echo -e "\n${commandLine}\n"

if [[ $1 == -dr ]]; then
    exit 
fi 

pushd . >/dev/null || exit 1
eval "$commandLine"
popd >/dev/null   