#!/bin/bash

if [[ $UID -ne 0 ]]; then
    echo -n "Please run as root"
    [[ $EUID -ne 0 ]] && echo "" || echo " (instead of sudo)" 
    echo "Aborting!"
    exit 1
fi 

if [[ ! -z $(ls /mnt/linuxqa 2>/dev/null) ]]; then
    apt install -y nfs-common
    mkdir -p /mnt/linuxqa
    mount linuxqa.nvidia.com:/storage/people /mnt/linuxqa || exit 1
fi 

if [[ ! -z $(ls /mnt/builds 2>/dev/null) ]]; then
    apt install -y nfs-common
    mkdir -p /mnt/builds
    mount linuxqa.nvidia.com:/storage3/builds /mnt/builds  
fi 

if [[ ! -z $(ls /mnt/dvsbuilds 2>/dev/null) ]]; then
    apt install -y nfs-common
    mkdir -p /mnt/dvsbuilds
    mount linuxqa.nvidia.com:/storage5/dvsbuilds /mnt/dvsbuilds
fi 

if [[ ! -z $(ls /mnt/data 2>/dev/null) ]]; then
    apt install -y nfs-common
    mkdir -p /mnt/data
    mount linuxqa.nvidia.com:/storage/data /mnt/data 
fi 

if [[ ! -z $(ls /root/nvt 2>/dev/null) ]]; then
    apt install -y python3 python3-pip libjpeg-dev 
    /mnt/linuxqa/nvt.sh sync || exit 1
fi 

if [[ -z $1 ]]; then
    /mnt/linuxqa/nvt.sh driver --help 
else 
    /mnt/linuxqa/nvt.sh driver $@
fi 