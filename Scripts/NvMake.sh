#!/bin/bash

if ! dpkg --print-foreign-architectures | grep -q '^i386$'; then
    echo "Enabling i386 architecture"
    sudo dpkg --add-architecture i386
    sudo apt update 
    sudo apt install -y libc6:i386 libncurses6:i386 libstdc++6:i386
fi
for pkg in libelf-dev elfutils; do 
    dpkg -s $pkg &>/dev/null || sudo apt install -y $pkg 
done 

branch=rel/gpu_drv/r580/r580_00
subdir=
module="drivers dist"
arch=amd64 
config=develop
jobs=$(nproc)
others=
excludeModules=(
    vgpu # GPU virtualization
    gpgpu # CUDA driver
    gpgpucomp # CUDA compiler (used by CUDA and raytracing)
    compiler # OpenCL 
    gpgpudbg # CUDA debugger
    uvm # Unified Virtual Memory (used by CUDA) 
    raytracing # Vulkan raytracing (depends on gpgpu, gpgpucomp and uvm)
    optix # Optix raytracing API (depends on gpgpu, gpgpucomp and uvm)
    #nvapi # Linux re-impl of NVAPI (used by iGPU_vfmax_scripts/perfdebug)
    nvtopps # Notebook power management 
    testutils # UVM tests, lock-to-rated-tdp
    vdpau # VDPAU video acceleration driver
    ngx # Neural Graphics Experience
    nvfbc # Nvidia framebuffer capture
    nvcuvid # CUDA based video driver
    encodeapi # Video encode API
    opticalflow # Opticalflow video driver 
    fabricmanager # Fabric manager 
    nvlibpkcs11 # PKCS11 cryptograph (used in confidential compute)
    vulkansc # VulkanSC driver 
    pcc # VulkanSC PCC
)

while [[ $# -gt 0 ]]; do 
    case $1 in 
        bfm)  branch=dev/gpu_drv/bugfix_main ;;
        r580) branch=rel/gpu_drv/r580/r580_00 ;;
        sweep|drivers|opengl) module=$1; if [[ $1 == drivers ]]; then 
            module+=" dist"
        fi ;;
        d3dreg|nvreg) module= ; subdir="drivers/ddraw/tools/D3DRegKeys/d3dreg" ;;  
        amd64|aarch64) arch=$1 ;;
        debug|release|develop) config=$1 ;;
        j|jobs) shift; jobs=$1 ;; 
        cc|comcmd) if [[ ! -z $(grep compilecommands $P4ROOT/$branch/drivers/common/build/build.cfg) ]]; then 
            others+=" compilecommands"
        fi ;;
        full|fullbuild) excludeModules=() ;;
        *) others+=" $1" ;;
    esac
    shift 
done 

workdir=$P4ROOT/$branch/$subdir
if [[ ! -d $workdir ]]; then 
    echo "Error: source folder not found: $workdir"
    exit 1
fi 

version=$(grep '^#define NV_VERSION_STRING' $P4ROOT/$branch/drivers/common/inc/nvUnixVersion.h | awk '{print $3}' | sed 's/"//g')
echo "Driver code version: $version"

unixBuildArgs=(
    --unshare-namespaces
    --tools  $P4ROOT/tools 
    --devrel $P4ROOT/devrel/SDK/inc/GL
)

nvmakeArgs=(
    NV_COLOR_OUTPUT=1 
    NV_GUARDWORD= 
    NV_COMPRESS_THREADS=$(nproc)
    NV_FAST_PACKAGE_COMPRESSION=zstd 
    NV_USE_FRAME_POINTER=1
    NV_UNIX_LTO_ENABLED= 
    NV_LTCG=
    NV_UNIX_CHECK_DEBUG_INFO=0
    NV_MANGLE_SYMBOLS= 
    NV_TRACE_CODE=1
    NV_EXCLUDE_BUILD_MODULES="\"${excludeModules[@]}\""
    "$module" 
    linux 
    "$arch" 
    "$config" 
    "-j$jobs" 
    "$others"
)

commandLine="cd $workdir && $P4ROOT/tools/linux/unix-build/unix-build ${unixBuildArgs[@]} nvmake ${nvmakeArgs[@]} > >(tee /tmp/nvmake.stdout) 2> >(tee /tmp/nvmake.stderr >&2)"

echo "${commandLine}"
if [[ $WZHU_YES != 1 ]]; then 
    read -p "Press [Enter] to continue: "
fi 

pushd . >/dev/null || exit 1
eval "$commandLine" 
popd >/dev/null   