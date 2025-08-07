#!/bin/bash

if [[ $EUID -ne 0 ]]; then
    echo "Please run as root"
    echo "Aborting!"
    exit 1
fi 

# Mount /mnt/linuxqa
if [[ ! -z $(ls /mnt/linuxqa 2>/dev/null) ]]; then
    apt install -y nfs-common
    mkdir -p /mnt/linuxqa
    mount linuxqa.nvidia.com:/storage/people /mnt/linuxqa || exit 1
fi 

# Mount /mnt/builds
if [[ ! -z $(ls /mnt/builds 2>/dev/null) ]]; then
    apt install -y nfs-common
    mkdir -p /mnt/builds
    mount linuxqa.nvidia.com:/storage3/builds /mnt/builds  
fi 

# Mount /mnt/dvsbuilds
if [[ ! -z $(ls /mnt/dvsbuilds 2>/dev/null) ]]; then
    apt install -y nfs-common
    mkdir -p /mnt/dvsbuilds
    mount linuxqa.nvidia.com:/storage5/dvsbuilds /mnt/dvsbuilds
fi 

# Mount /mnt/data
if [[ ! -z $(ls /mnt/data 2>/dev/null) ]]; then
    apt install -y nfs-common
    mkdir -p /mnt/data
    mount linuxqa.nvidia.com:/storage/data /mnt/data 
fi 

# Sync /root/nvt
if [[ ! -z $(ls /root/nvt 2>/dev/null) ]]; then
    apt install -y python3 python3-pip libjpeg-dev 
    /mnt/linuxqa/nvt.sh sync || exit 1
fi 

if [[ -z $1 ]]; then
    /mnt/linuxqa/nvt.sh driver --help 
else 
    /mnt/linuxqa/nvt.sh driver $@ 
    read -p "ENVVARS: " envvars
    for pair in $envvars; do 
        echo "export ${pair%%=*}=${pair#*=}"
        export ${pair%%=*}=${pair#*=}
    done 
    /bin/bash 
fi 