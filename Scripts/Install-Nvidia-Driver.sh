#!/bin/bash

source $(dirname $0)/Micro-Functions.sh
Mount-NvTest-Dirs

if [[ ! -z $(ls /root/nvt 2>/dev/null) ]]; then
    sudo apt install -y python3 python3-pip libjpeg-dev 
    sudo su -c "/mnt/linuxqa/nvt.sh sync" || exit 1
fi 

if [[ -z $1 ]]; then
    sudo su -c "/mnt/linuxqa/nvt.sh driver --help"
else 
    sudo su -c " NVTEST_INSTALLER_REUSE_INSTALL=False /mnt/linuxqa/nvt.sh driver $*" || exit 1
    read -p "ENVVARS: " envvars
    for pair in $envvars; do 
        echo "export ${pair%%=*}=${pair#*=}"
        export ${pair%%=*}=${pair#*=}
    done 
    /bin/bash 
fi 