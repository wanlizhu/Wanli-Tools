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
        branch=rel/gpu_drv/r580/r580_00
        module=drivers 
        config=develop
        while [[ $# -gt 2 ]]; do 
            case $3 in 
                bfm) branch=dev/gpu_drv/bugfix_main ;;
                opengl) module=$3 ;;
                debug|release|develop) config=$3 ;;
            esac
            shift 
        done 
        if [[ $module == drivers ]]; then 
            rsync -ah --progress wanliz@office:/media/wanliz/data/wanliz-sw-gpu-driver-office/$branch/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/NVIDIA-Linux-$(uname -m)-*-internal.run wanliz@office:/media/wanliz/data/wanliz-sw-gpu-driver-office/$branch/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/tests-Linux-$(uname -m).tar $HOME && echo || exit 1

            ls $HOME/NVIDIA-Linux-$(uname -m)-*-internal.run | awk -F/ '{print $NF}'  | sort -V 
            read -p "Enter [version] to continue: " version
            $0 driver local $HOME/NVIDIA-Linux-$(uname -m)-$version-internal.run
        elif [[ $module == opengl ]]; then 
            rsync -ah --progress wanliz@office:/media/wanliz/data/wanliz-sw-gpu-driver-office/$branch/drivers/OpenGL/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/libnvidia-glcore.so $HOME || exit 1
            $0 driver local $HOME/libnvidia-glcore.so
        fi 
    elif [[ $2 == local ]]; then
        if [[ $3 == *".run" ]]; then 
            for nvpid in $(sudo fuser -v /dev/nvidia* 2>/dev/null | grep -v 'COMMAND' | awk '{print $3}' | sort  | uniq); do 
                sudo kill -9 $nvpid && echo "Killed $nvpid"
            done
            sudo env IGNORE_CC_MISMATCH=1 IGNORE_MISSING_MODULE_SYMVERS=1 $3 -s --no-kernel-module-source --skip-module-load || { cat /var/log/nvidia-installer.log; exit 1; }
            unset NVTEST_DRIVER NVTEST_DRIVER_BRANCH NVTEST_DRIVER_CHANGELIST NVTEST_DRIVER_DIR && echo "Unset NVTEST_* envvars -> OK"
            if [[ -f $(dirname $3)/tests-Linux-$(uname -m).tar ]]; then 
                tar -xf $(dirname $3)/tests-Linux-$(uname -m).tar -C $HOME 
                sudo ln -sf $HOME/tests-Linux-$(uname -m)/sandbag-tool/sandbag-tool $HOME/sandbag-tool && echo "Updated: $HOME/sandbag-tool"
            fi 
        elif [[ $3 == *".so" ]]; then 
            version=$(ls /usr/lib/*-linux-gnu/$(basename $3).*  | awk -F '.so.' '{print $2}' | head -1)
            sudo cp -vf --remove-destination $3 /usr/lib/$(uname -m)-linux-gnu/$(basename $3).$version 
        fi 
    else
        sudo -H bash -lc "NVTEST_INSTALLER_REUSE_INSTALL=False /mnt/linuxqa/nvt.sh $*" || exit 1
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
    if [[ $(uname -m) == aarch64 ]]; then 
        perfdebug=/mnt/linuxqa/wanliz/iGPU_vfmax_scripts/perfdebug
        sudo $perfdebug --lock_loose   set pstateId P0 && echo -e "set pstateId -> [OK]\n"
        #sudo $perfdebug --lock_strict  set dramclkkHz 8000000 && echo -e "set dramclkkHz -> [OK]\n"
        sudo $perfdebug --lock_strict  set gpcclkkHz  2000000 && echo -e "set gpcclkkHz  2000MHz -> [OK]\n"
        sudo $perfdebug --lock_loose   set xbarclkkHz 1800000 && echo -e "set xbarclkkHz 1800MHz -> [OK]\n"
        #sudo $perfdebug --lock_loose   set sysclkkHz  1800000 && echo -e "set sysclkkHz  -> [OK]\n"
        sudo $perfdebug --force_regime ffr && echo "Force regime successful"
        echo "" && sleep 3
        echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
        echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
        echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)" 
    fi 
elif [[ $1 == startx ]]; then 
    [[ -z $DISPLAY ]] && export DISPLAY=:0
    if [[ -z $NVTEST_DRIVER ]]; then
        screen -S bare-xorg bash -c "sudo X $DISPLAY +iglx || read -p 'Press [Enter] to exit: '"
        xrandr --fb 3840x2160  
    else
        sudo -H bash -lc "screen -S nvtest-fake-display bash -c \"NVTEST_NO_SMI=1 NVTEST_NO_RMMOD=1 NVTEST_NO_MODPROBE=1 /mnt/linuxqa/nvt.sh 3840x2160__runcmd --cmd 'sleep 2147483647'  || read -p 'Press [Enter] to exit: '\""
    fi 
    echo "Xorg PID: $(pidof Xorg)"
    xrandr | grep current || ls -al /tmp/.X11-unix/
elif [[ $1 == viewperf ]]; then 
    # $2: viewset, $3: subtest, [$4: optional pic-x args]
    GL_ENV=$(env | grep -E '^(__GL_|WZHU_)' | while IFS='=' read -r k v; do printf 'export %s=%q; ' $k $v; done)
    if [[ $WZHU_PI == 1 ]]; then 
        commandLine="$GL_ENV cd $(pwd) && rm -rf $HOME/SinglePassCapture/PerfInspector/output/viewperf-$2-subtest$3-on-$(hostname)$WZHU_PI_SUFFIX && $HOME/SinglePassCapture/pic-x $4 --api=ogl --check_clocks=0 --sample=24000 --aftbuffersize=2048 --name=viewperf-$2-subtest$3-on-$(hostname)$WZHU_PI_SUFFIX --startframe=100 --exe=./viewperf/bin/viewperf --arg=\"viewsets/$2/config/$2.xml $3 -resolution 3840x2160\" --workdir=/root/nvt/tests/viewperf2020v3/viewperf2020 | grep -v \"won't hook API\" && sudo -u $USER -H bash -lc \"source $HOME/SinglePassCapture/PerfInspector/Python-venv/bin/activate && NVM_GTLAPI_USER=wanliz $HOME/SinglePassCapture/PerfInspector/output/viewperf-$2-subtest$3-on-$(hostname)$WZHU_PI_SUFFIX/upload_report.sh\"" 
    else
        if [[ ! -z $2 ]]; then 
            commandLine="$GL_ENV cd /root/nvt/tests/viewperf2020v3/viewperf2020 && ./viewperf/bin/viewperf viewsets/$2/config/$2.xml $3 -resolution 3840x2160 && cat /root/nvt/tests/viewperf2020v3/viewperf2020/results/$2*/results.xml"
        else
            commandLine="$GL_ENV cd /root/nvt/tests/viewperf2020v3/viewperf2020"
            for viewset in catia creo energy maya medical snx sw; do 
                commandLine+="  ; ./viewperf/bin/viewperf viewsets/$viewset/config/$viewset.xml -resolution 3840x2160 && cat /root/nvt/tests/viewperf2020v3/viewperf2020/results/$viewset*/results.xml"
            done 
        fi 
    fi 

    echo "${commandLine}"
    read -p "Press [Enter] to continue as root: "
    sudo -H bash -lc "$commandLine" 
else 
    GL_ENV=$(env | grep -E '^(__GL_|WZHU_)' | while IFS='=' read -r k v; do printf 'export %s=%q; ' $k $v; done)
    sudo -H bash -lc "$GL_ENV /mnt/linuxqa/nvt.sh $*" 
fi 