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
    sudo -H bash -lc " NVTEST_INSTALLER_REUSE_INSTALL=False /mnt/linuxqa/nvt.sh $*" || exit 1

    read -p "ENVVARS (Copy & Paste): " envvars
    for pair in $envvars; do 
        echo "export ${pair%%=*}=${pair#*=}"
        export ${pair%%=*}=${pair#*=}
    done 

    # Start a new shell with exported variables 
    /bin/bash 
elif [[ $1 == env ]]; then
    echo "NVTEST_DRIVER           : $NVTEST_DRIVER"
    echo "NVTEST_DRIVER_BRANCH    : $NVTEST_DRIVER_BRANCH"
    echo "NVTEST_DRIVER_CHANGELIST: $NVTEST_DRIVER_CHANGELIST"
    echo "NVTEST_DRIVER_DIR       : $NVTEST_DRIVER_DIR"
    echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
elif [[ $1 == maxclock ]]; then 
    sudo /root/nvt/tests/system/sandbag-tool/sandbag-tool -unsandbag
    sudo /root/nvt/tests/system/sandbag-tool/sandbag-tool -print 

    sudo /mnt/linuxqa/wanliz/iGPU_vfmax_scripts/perfdebug --lock_loose   set pstateId P0 && echo -e "set pstateId -> [OK]\n"
    sudo /mnt/linuxqa/wanliz/iGPU_vfmax_scripts/perfdebug --lock_strict  set dramclkkHz 8000000 && echo -e "set dramclkkHz -> [OK]\n"
    sudo /mnt/linuxqa/wanliz/iGPU_vfmax_scripts/perfdebug --lock_strict  set gpcclkkHz  1995000 && echo -e "set gpcclkkHz  -> [OK]\n"
    sudo /mnt/linuxqa/wanliz/iGPU_vfmax_scripts/perfdebug --lock_loose   set xbarclkkHz 2400000 && echo -e "set xbarclkkHz -> [OK]\n"
    sudo /mnt/linuxqa/wanliz/iGPU_vfmax_scripts/perfdebug --lock_loose   set sysclkkHz  1800000 && echo -e "set sysclkkHz  -> [OK]\n"
    sudo /mnt/linuxqa/wanliz/iGPU_vfmax_scripts/perfdebug --force_regime ffr && echo "Force regime successful"
    echo "" && sleep 3
    echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
    echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
    echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)" 
elif [[ $1 == startx ]]; then 
    sudo -H bash -lc "screen -S nvtest-fake-display bash -c \"NVTEST_NO_SMI=1 NVTEST_NO_RMMOD=1 NVTEST_NO_MODPROBE=1 /mnt/linuxqa/nvt.sh 3840x2160__runcmd --cmd 'sleep 2147483647'\""
    echo "Xorg PID: $(pidof Xorg)"
    xrandr | grep current
elif [[ $1 == viewperf ]]; then 
    if [[ -z $2 ]]; then echo "viewset name is missing"; exit 1; fi 
    if [[ $WZHU_PI == 1 ]]; then 
        commandLine="cd $(pwd) && $HOME/SinglePassCapture/pic-x --api=ogl --check_clocks=0 --sample=4800 --aftbuffersize=2048 --name=viewperf-$2-subtest$3-on-$(hostname) --startframe=100 --exe=./viewperf/bin/viewperf --arg=\"viewsets/$2/config/$2.xml $3 -resolution 3840x2160\" --workdir=/root/nvt/tests/viewperf2020v3/viewperf2020 | grep -v \"won't hook API\"" 
    else
        commandLine="cd /root/nvt/tests/viewperf2020v3/viewperf2020 && ./viewperf/bin/viewperf viewsets/$2/config/$2.xml $3 -resolution 3840x2160 && cat /root/nvt/tests/viewperf2020v3/viewperf2020/results/$2*/results.xml"
    fi 

    echo -e "\n${commandLine}\n"
    read -p "Press [Enter] to continue as root: "
    sudo -H bash -lc "$commandLine"
else 
    sudo -H bash -lc "/mnt/linuxqa/nvt.sh $*"
fi 