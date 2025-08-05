#!/bin/bash

if ! dpkg --print-foreign-architectures | grep -q '^i386$'; then
    echo "Enabling i386 architecture"
    sudo dpkg --add-architecture i386
    sudo apt-get update
    sudo apt-get install -y libc6:i386 libncurses5:i386 libstdc++6:i386
fi

if grep -qE "(microsoft|WSL2)" /proc/version; then
    if ! mountpoint -q /mnt/wzhu-linux; then
        echo "Mount /mnt/wzhu-linux from Windows host first, aborting!"
        exit 1
    fi 
fi 

