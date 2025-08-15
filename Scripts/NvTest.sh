#!/bin/bash

if [[ -z $(ls /mnt/linuxqa 2>/dev/null) ]]; then
    sudo mkdir -p /mnt/linuxqa
    sudo mount linuxqa.nvidia.com:/storage/people /mnt/linuxqa || exit 1
fi 
if [[ -z $(ls /mnt/builds 2>/dev/null) ]]; then
    sudo mkdir -p /mnt/builds
    sudo mount linuxqa.nvidia.com:/storage3/builds /mnt/builds  
fi 
if [[ -z $(ls /mnt/dvsbuilds 2>/dev/null) ]]; then
    sudo mkdir -p /mnt/dvsbuilds
    sudo mount linuxqa.nvidia.com:/storage5/dvsbuilds /mnt/dvsbuilds
fi 
if [[ -z $(ls /mnt/data 2>/dev/null) ]]; then
    sudo mkdir -p /mnt/data
    sudo mount linuxqa.nvidia.com:/storage/data /mnt/data 
fi 

if [[ ! -z $(ls /root/nvt 2>/dev/null) ]]; then
    sudo apt install -y python3 python3-pip libjpeg-dev 
    sudo -H bash -lc "/mnt/linuxqa/nvt.sh sync" || exit 1
fi 

if [[ $1 == driver || $1 == drivers ]]; then 
    if [[ $2 == rsync ]]; then 
        config=$3
        rsync -ah --progress wanliz@office:/media/wanliz/data/wanliz-sw-gpu-driver-office/rel/gpu_drv/r580/r580_00/_out/Linux_$(uname -m)_${config:-develop}/NVIDIA-Linux-$(uname -m)-*-internal.run wanliz@office:/media/wanliz/data/wanliz-sw-gpu-driver-office/rel/gpu_drv/r580/r580_00/_out/Linux_$(uname -m)_${config:-develop}/tests-Linux-$(uname -m).tar $HOME || exit 1

        ls $HOME/NVIDIA-Linux-$(uname -m)-*-internal.run | awk -F/ '{print $NF}'  | sort -V 
        read -p "Enter [version] to continue: " version
        sudo pkill Xorg 
        sudo env IGNORE_CC_MISMATCH=1 IGNORE_MISSING_MODULE_SYMVERS=1 $HOME/NVIDIA-Linux-$(uname -m)-$version-internal.run -s --no-kernel-module-source --skip-module-load || exit 1

        unset NVTEST_DRIVER NVTEST_DRIVER_BRANCH NVTEST_DRIVER_CHANGELIST NVTEST_DRIVER_DIR && echo "Unset NVTEST_* envvars -> OK"
        tar -xf $HOME/tests-Linux-$(uname -m).tar -C $HOME 
        sudo ln -sf $HOME/tests-Linux-$(uname -m)/sandbag-tool/sandbag-tool $HOME/sandbag-tool && echo "Updated: $HOME/sandbag-tool"
    else
        sudo -H bash -lc " NVTEST_INSTALLER_REUSE_INSTALL=False /mnt/linuxqa/nvt.sh $*" || exit 1
        read -p "ENVVARS (Copy & Paste): " envvars
        for pair in $envvars; do 
            echo "export ${pair%%=*}=${pair#*=}"
            export ${pair%%=*}=${pair#*=}
        done 
        sudo ln -sf /root/nvt/tests/system/sandbag-tool/sandbag-tool $HOME/sandbag-tool && echo "Updated: $HOME/sandbag-tool"
    fi 
    /bin/bash 
elif [[ $1 == env ]]; then
    echo "NVTEST_DRIVER           : $NVTEST_DRIVER"
    echo "NVTEST_DRIVER_BRANCH    : $NVTEST_DRIVER_BRANCH"
    echo "NVTEST_DRIVER_CHANGELIST: $NVTEST_DRIVER_CHANGELIST"
    echo "NVTEST_DRIVER_DIR       : $NVTEST_DRIVER_DIR"
    echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
elif [[ $1 == maxclock ]]; then 
    sudo $HOME/sandbag-tool -unsandbag
    sudo $HOME/sandbag-tool -print 

    perfdebug=/mnt/linuxqa/wanliz/iGPU_vfmax_scripts/perfdebug
    sudo $perfdebug --lock_loose   set pstateId P0 && echo -e "set pstateId -> [OK]\n"
    sudo $perfdebug --lock_strict  set dramclkkHz 8000000 && echo -e "set dramclkkHz -> [OK]\n"
    sudo $perfdebug --lock_strict  set gpcclkkHz  1995000 && echo -e "set gpcclkkHz  -> [OK]\n"
    sudo $perfdebug --lock_loose   set xbarclkkHz 2400000 && echo -e "set xbarclkkHz -> [OK]\n"
    sudo $perfdebug --lock_loose   set sysclkkHz  1800000 && echo -e "set sysclkkHz  -> [OK]\n"
    sudo $perfdebug --force_regime ffr && echo "Force regime successful"
    echo "" && sleep 3
    echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
    echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
    echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)" 
elif [[ $1 == startx ]]; then 
    if [[ -z $NVTEST_DRIVER ]]; then
        screen -S bare-xorg sudo X :0 +iglx
        xrandr --fb 3840x2160  
    else
        sudo -H bash -lc "screen -S nvtest-fake-display bash -c \"NVTEST_NO_SMI=1 NVTEST_NO_RMMOD=1 NVTEST_NO_MODPROBE=1 /mnt/linuxqa/nvt.sh 3840x2160__runcmd --cmd 'sleep 2147483647'\""
    fi 
    echo "Xorg PID: $(pidof Xorg)"
    xrandr | grep current
elif [[ $1 == viewperf ]]; then 
    GL_ENV=$(env | grep -E '^(__GL_|WZHU_)' | while IFS='=' read -r k v; do printf 'export %s=%q; ' $k $v; done)
    if [[ $WZHU_PI == 1 ]]; then 
        commandLine="$GL_ENV cd $(pwd) && $HOME/SinglePassCapture/pic-x --api=ogl --check_clocks=0 --sample=24000 --aftbuffersize=2048 --name=viewperf-$2-subtest$3-on-$(hostname) --startframe=100 --exe=./viewperf/bin/viewperf --arg=\"viewsets/$2/config/$2.xml $3 -resolution 3840x2160\" --workdir=/root/nvt/tests/viewperf2020v3/viewperf2020 | grep -v \"won't hook API\"" 
    else
        commandLine="$GL_ENV cd /root/nvt/tests/viewperf2020v3/viewperf2020 && ./viewperf/bin/viewperf viewsets/$2/config/$2.xml $3 -resolution 3840x2160 && cat /root/nvt/tests/viewperf2020v3/viewperf2020/results/$2*/results.xml"
    fi 

    echo "${commandLine}"
    read -p "Press [Enter] to continue as root: "
    sudo -H bash -lc "$commandLine" 
else 
    GL_ENV=$(env | grep -E '^(__GL_|WZHU_)' | while IFS='=' read -r k v; do printf 'export %s=%q; ' $k $v; done)
    sudo -H bash -lc "$GL_ENV /mnt/linuxqa/nvt.sh $*" 
fi 