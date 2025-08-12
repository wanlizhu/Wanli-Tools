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
    sudo su -c "/mnt/linuxqa/nvt.sh sync" || exit 1
fi 

if [[ $1 == driver || $1 == drivers ]]; then 
    sudo su -c " NVTEST_INSTALLER_REUSE_INSTALL=False /mnt/linuxqa/nvt.sh $*" || exit 1

    read -p "ENVVARS (Copy & Paste): " envvars
    for pair in $envvars; do 
        echo "export ${pair%%=*}=${pair#*=}"
        export ${pair%%=*}=${pair#*=}
    done 
    
    read -p "Unsandbag installed driver? (yes/no): " -e -i yes unsandbag
    if [[ $unsandbag == yes ]]; then 
        pushd /tmp >/dev/null 
        sudo rm -rf /tmp/tests-Linux-$(uname -m)
        if [[ "$NVTEST_DRIVER" == "http"* ]]; then 
            /root/nvt/driver/$(basename "$NVTEST_DRIVER")
        else
            tar -xf $(dirname $NVTEST_DRIVER)/tests-Linux-$(uname -m).tar 
        fi 
        echo "(Nothing prints out if there is no GPU workload)"
        ./tests-Linux-$(uname -m)/sandbag-tool/sandbag-tool -unsandbag
        ./tests-Linux-$(uname -m)/sandbag-tool/sandbag-tool -print 
        popd >/dev/null 
    fi 

    read -p "Lock iGPU clocks? (yes/no): " -e -i yes lock
    if [[ $lock == yes ]]; then
        pushd /mnt/linuxqa/wanliz/iGPU_vfmax_scripts >/dev/null 
        echo -e "\n>> sudo ./perfdebug --lock_loose   set pstateId P0"        && sudo ./perfdebug --lock_loose   set pstateId P0 
        echo -e "\n>> sudo ./perfdebug --lock_strict  set dramclkkHz 8000000" && sudo ./perfdebug --lock_strict  set dramclkkHz 8000000 
        echo -e "\n>> sudo ./perfdebug --lock_strict  set gpcclkkHz 1995000"  && sudo ./perfdebug --lock_strict  set gpcclkkHz 1995000 
        echo -e "\n>> sudo ./perfdebug --lock_loose   set xbarclkkHz 2400000" && sudo ./perfdebug --lock_loose   set xbarclkkHz 2400000 
        echo -e "\n>> sudo ./perfdebug --lock_loose   set sysclkkHz 1800000"  && sudo ./perfdebug --lock_loose   set sysclkkHz 1800000 
        echo -e "\n>> sudo ./perfdebug --force_regime ffr"                    && sudo ./perfdebug --force_regime ffr && echo "Force regime successful"
        echo "" && sleep 3
        echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
        echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
        echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
        popd >/dev/null 
    fi 

    # Start a new shell with exported variables 
    /bin/bash 
elif [[ $1 == env ]]; then
    echo "NVTEST_DRIVER           : $NVTEST_DRIVER"
    echo "NVTEST_DRIVER_BRANCH    : $NVTEST_DRIVER_BRANCH"
    echo "NVTEST_DRIVER_CHANGELIST: $NVTEST_DRIVER_CHANGELIST"
    echo "NVTEST_DRIVER_DIR       : $NVTEST_DRIVER_DIR"
elif [[ $1 == startx ]]; then 
    screen -S nvtest-fake-display bash -c 'sudo su -c "NVTEST_NO_SMI=1 NVTEST_NO_RMMOD=1 NVTEST_NO_MODPROBE=1 /mnt/linuxqa/nvt.sh 3840x2160__runcmd --cmd "sleep 2147483647""'
    echo "Xorg PID: $(pidof Xorg)"
    xrandr | grep current
else 
    sudo su -c "/mnt/linuxqa/nvt.sh $*"
fi 