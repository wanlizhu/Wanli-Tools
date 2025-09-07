#!/bin/bash

host=$([[ -d /wanliz_sw_linux ]] && echo localhost || echo "wanliz@office")
root=$([[ -z $P4ROOT ]] && echo /wanliz_sw_linux || echo "$P4ROOT")
branch=rel/gpu_drv/r580/r580_00
subdir=
config=develop 
module=drivers
while [[ $# -gt 0 ]]; do 
    case $1 in 
        office) host="wanliz@office" ;;
        /*) root=$1 ;;
        bfm)  branch=dev/gpu_drv/bugfix_main ;;
        r580) branch=rel/gpu_drv/r580/r580_00 ;;
        debug|release|develop) config=$1 ;;
        opengl) module=opengl; subdir="drivers/OpenGL" ;;
    esac
    shift 
done 

case $module in 
    drivers) driver="$root/$branch/$subdir/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/NVIDIA-Linux-$(uname -m)-*.run" ;;
    opengl)  driver="$root/$branch/$subdir/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/libnvidia-glcore.so" ;;
    *) echo "Error: unknown module: \"$module\""; exit 1 ;;
esac 
if [[ $host != localhost ]]; then 
    sudo rm -rf /tmp/drivers  
    mkdir -p /tmp/drivers 
    source $(dirname $0)/NvConfig.sh
    type -a NoPasswd-SSH
    exit 1 
    NoPasswd-SSH "$host" 
    rsync -ah --progress "$host:$driver" /tmp/drivers  
    if [[ $driver == *".run" ]]; then 
        rsync -ah --progress "$host:$(dirname $driver)/tests-Linux-$(uname -m).tar" /tmp/drivers  
    fi 
    driver="/tmp/drivers/$(basename $driver)"
fi 

if [[ $(ls $driver | wc -l) -gt 1 ]]; then 
    ls $(dirname $driver)/NVIDIA-Linux-$(uname -m)-*.run 2>/dev/null | awk -F/ '{print $NF}' | sort -V | sed -E 's/([0-9]+\.[0-9]+)/\x1b[31m\1\x1b[0m/'
    read -p "Install version: " version
    driver="$(dirname $driver)/NVIDIA-Linux-$(uname -m)-$version-internal.run"
fi 

if [[ ! -f $driver ]]; then 
    echo "Error: file not found: $driver"
    exit 1
fi 

if [[ $driver == *".run" ]]; then 
    echo "Kill all graphics apps and install $driver"
    read -p "Press [Enter] to continue: "
    sudo fuser -v /dev/nvidia* 2>/dev/null | grep -v 'COMMAND' | awk '{print $3}' | sort | uniq | tee > /tmp/nvidia
    for nvpid in $(cat /tmp/nvidia); do 
        echo -n "Killing $nvpid "
        sudo kill -9 $nvpid && echo "-> OK" || echo "-> Failed"
        sleep 1
    done

    while :; do
        removed=0
        for m in $(lsmod | awk '/^nvidia/ {print $1}'); do
            if [ ! -d "/sys/module/$m/holders" ] || [ -z "$(ls -A /sys/module/$m/holders 2>/dev/null)" ]; then
                sudo rmmod -f "$m" && removed=1
                echo "Remove kernel module $m -> OK"
            fi
        done
        [ "$removed" -eq 0 ] && break
    done

    sudo env IGNORE_CC_MISMATCH=1 IGNORE_MISSING_MODULE_SYMVERS=1 $driver -s --no-kernel-module-source --skip-module-load || { 
        cat /var/log/nvidia-installer.log
        echo "Aborting..."
        exit 1
    }

    if [[ -f $(dirname $driver)/tests-Linux-$(uname -m).tar ]]; then 
        tar -xf $(dirname $driver)/tests-Linux-$(uname -m).tar -C /tmp 
        sudo cp -vf /tmp/tests-Linux-$(uname -m)/sandbag-tool/sandbag-tool $HOME/sandbag-tool  
    fi 
elif [[ $driver == *".so" ]]; then 
    version=$(ls /usr/lib/*-linux-gnu/$(basename $driver).*  | awk -F '.so.' '{print $2}' | head -1)
    sudo cp -vf --remove-destination $driver /usr/lib/$(uname -m)-linux-gnu/$(basename $driver).$version 
else
    echo "Error: unknown driver format: \"$driver\""
    exit 1
fi 