#!/bin/bash

codeBaseDir=$P4ROOT 
branchName=rel/gpu_drv/r580/r580_00
moduleName="drivers dist"
moduleDir=""  
targetArch=amd64 
targetConfig=develop
builderThreads=$(nproc)

while [[ $# -gt 0 ]]; do 
    case $1 in 
        bfm)  branchName=dev/gpu_drv/bugfix_main ;;
        r580) branchName=rel/gpu_drv/r580/r580_00 ;;
        sweep)   moduleName="sweep"; moduleDir="" ;;
        drivers) moduleName="drivers dist"; moduleDir="" ;;
        opengl)  moduleName="opengl"; moduleDir="drivers/OpenGL" ;;
        amd64|aarch64) targetArch=$1 ;;
        debug|release|develop) targetConfig=$1 ;;
        -r|-root) shift; codeBaseDir="$1" ;;
        -j|-jobs) shift; builderThreads=$1 ;;
    esac
    shift 
done 

if [[ ! -d $codeBaseDir/$branchName/$moduleDir ]]; then 
    echo "Error: source folder not found: $codeBaseDir/$branchName/$moduleDir"
    exit 1
fi 

commandLine="cd $codeBaseDir/$branchName/$moduleDir && $codeBaseDir/tools/linux/unix-build/unix-build --tools $codeBaseDir/tools --devrel $codeBaseDir/devrel/SDK/inc/GL --unshare-namespaces nvmake NV_COLOR_OUTPUT=1 NV_GUARDWORD= NV_COMPRESS_THREADS=$(nproc) NV_FAST_PACKAGE_COMPRESSION=zstd NV_USE_FRAME_POINTER=1 NV_UNIX_LTO_ENABLED= NV_MANGLE_SYMBOLS= NV_TRACE_CODE=1 -time $moduleName linux $targetArch $targetConfig -j$builderThreads"

echo "${commandLine}"
echo -n "" >&1
read -p "Press [Enter] to continue: "

pushd . >/dev/null || exit 1
eval "$commandLine"
popd >/dev/null   