#!/bin/bash

if ! dpkg --print-foreign-architectures | grep -q '^i386$'; then
    echo "Enabling i386 architecture"
    sudo dpkg --add-architecture i386
    sudo apt-get update
    sudo apt-get install -y libc6:i386 libncurses6:i386 libstdc++6:i386
fi

# Install package dependencies
for pkg in libelf-dev elfutils; do 
    dpkg -s $pkg &>/dev/null || sudo apt install -y $pkg 
done 

# Install certificate for code signing (optional)
if ! ls /etc/ssl/certs/certnew*.pem &>/dev/null; then
    if [[ ! -z $(ls /mnt/linuxqa 2>/dev/null) ]]; then
        apt install -y nfs-common
        mkdir -p /mnt/linuxqa
        mount linuxqa.nvidia.com:/storage/people /mnt/linuxqa 
    fi 
    if mountpoint -q /mnt/linuxqa; then
        echo "Installing root certificate"
        sudo cp /mnt/linuxqa/wanliz/certnew.crt /usr/local/share/ca-certificates/
        sudo update-ca-certificates
    fi 
fi


