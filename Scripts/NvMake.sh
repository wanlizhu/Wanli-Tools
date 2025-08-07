#!/bin/bash

buildModule="drivers dist" 
buildConfig=develop
buildArch=amd64 
buildJobs="-j$(nproc)"
buildMisc="-time"
buildRoot=/mnt/g/wanliz-sw-gpu-driver-office
branchDir=rel/gpu_drv/r580/r580_00
moduleDir=

while [[ "$#" -gt 0 ]]; do 
    case $1 in 
        bugfix_main) branchDir=dev/gpu_drv/bugfix_main ;;
        r580)    branchDir=rel/gpu_drv/r580/r580_00 ;;
        drivers) buildModule="drivers dist"; moduleDir= ;;
        opengl)  buildModule=; moduleDir=drivers/OpenGL || exit 1;;
        libsass) buildModule=; moduleDir=drivers/common/HW/sass3lib; buildMisc+=" SASS3LIB_BUILD_DLL=0 BLACKWELLSASS=1 EXTERNAL_SASSLIB=0" || exit 1 ;;
        debug|release|develop) buildConfig=$1 ;;
        -j*) buildJobs=$1 ;;
        *) buildMisc+=" $1" ;;
    esac
    shift 
done 

echo "WorkDir: $buildRoot/$branchDir/$moduleDir"
echo "   Args: $buildModule linux $buildArch $buildConfig $buildJobs $buildMisc"
read -p "Press [Enter] to continue: " _

pushd $buildRoot/$branchDir/$moduleDir >/dev/null || exit 1
$buildRoot/tools/linux/unix-build/2.00/unix-build \
    --tools  $buildRoot/tools \
    --devrel $buildRoot/devrel/SDK/inc/GL \
    --unshare-namespaces \
    nvmake-2.09 \
    NV_COLOR_OUTPUT=1 \
    NV_GUARDWORD= \
    NV_COMPRESS_THREADS=$(nproc) \
    NV_FAST_PACKAGE_COMPRESSION=zstd \
    NV_USE_FRAME_POINTER=1 \
    NV_UNIX_LTO_ENABLED= \
    NV_MANGLE_SYMBOLS= \
    NV_TRACE_CODE=1 \
    $buildModule linux $buildArch $buildConfig $buildJobs $buildMisc || exit 1
popd >/dev/null   
