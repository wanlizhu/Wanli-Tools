#!/bin/bash

# Mount /mnt/linuxqa
if [[ ! -z $(ls /mnt/linuxqa 2>/dev/null) ]]; then
    sudo apt install -y nfs-common
    sudo mkdir -p /mnt/linuxqa
    sudo mount linuxqa.nvidia.com:/storage/people /mnt/linuxqa || exit 1
fi 

# Mount /mnt/builds
if [[ ! -z $(ls /mnt/builds 2>/dev/null) ]]; then
    sudo apt install -y nfs-common
    sudo mkdir -p /mnt/builds
    sudo mount linuxqa.nvidia.com:/storage3/builds /mnt/builds  
fi 

# Mount /mnt/dvsbuilds
if [[ ! -z $(ls /mnt/dvsbuilds 2>/dev/null) ]]; then
    sudo apt install -y nfs-common
    sudo mkdir -p /mnt/dvsbuilds
    sudo mount linuxqa.nvidia.com:/storage5/dvsbuilds /mnt/dvsbuilds
fi 

# Mount /mnt/data
if [[ ! -z $(ls /mnt/data 2>/dev/null) ]]; then
    sudo apt install -y nfs-common
    sudo mkdir -p /mnt/data
    sudo mount linuxqa.nvidia.com:/storage/data /mnt/data 
fi 

# Sync /root/nvt
if [[ ! -z $(ls /root/nvt 2>/dev/null) ]]; then
    sudo apt install -y python3 python3-pip libjpeg-dev 
    sudo su -c "/mnt/linuxqa/nvt.sh sync" || exit 1
fi 

if [[ -z $1 ]]; then
    sudo su -c "/mnt/linuxqa/nvt.sh driver --help"
else 
    sudo su -c "/mnt/linuxqa/nvt.sh driver $@" || exit 1
    read -p "ENVVARS: " envvars
    for pair in $envvars; do 
        echo "export ${pair%%=*}=${pair#*=}"
        export ${pair%%=*}=${pair#*=}
    done 
    /bin/bash 
fi 