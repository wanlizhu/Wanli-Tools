#!/bin/bash

# This script is expected to run as a startup script, no user interaction available
if [[ $UID -ne 0 ]] && ! sudo grep -q "$USER ALL=(ALL) NOPASSWD:ALL" /etc/sudoers; then
    echo "Run Setup-NoPasswd.sh first, skipping!"
    exit
fi 

mountPoint="/mnt/wzhu-work-linux"
idSerial="SNVMe_Samsung_SSD_990_0025_3847_4140_5866"

if ! mountpoint -q $mountPoint; then 
    for dev in /dev/sd?; do 
        serial=$(sudo udevadm info --query=all --name=$dev | grep 'ID_SERIAL=')
        if [[ "$serial" == *"$idSerial"* ]]; then 
            read -p "Press [Enter] to mount $isSerial inside WSL2: " _
            echo "Mounting $device to $mountPoint"
            sudo mkdir -p $mountPoint
            sudo mount $device $mountPoint
            exit 
        fi
    done
    echo "Disk not found: $idSerial" 
fi 