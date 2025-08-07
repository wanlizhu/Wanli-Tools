#!/bin/bash

function Mount-NvTest-Dirs {
    # Mount /mnt/linuxqa
    if [[ -z $(ls /mnt/linuxqa 2>/dev/null) ]]; then
        sudo apt install -y nfs-common
        sudo mkdir -p /mnt/linuxqa
        sudo mount linuxqa.nvidia.com:/storage/people /mnt/linuxqa || exit 1
    fi 

    # Mount /mnt/builds
    if [[ -z $(ls /mnt/builds 2>/dev/null) ]]; then
        sudo apt install -y nfs-common
        sudo mkdir -p /mnt/builds
        sudo mount linuxqa.nvidia.com:/storage3/builds /mnt/builds  
    fi 

    # Mount /mnt/dvsbuilds
    if [[ -z $(ls /mnt/dvsbuilds 2>/dev/null) ]]; then
        sudo apt install -y nfs-common
        sudo mkdir -p /mnt/dvsbuilds
        sudo mount linuxqa.nvidia.com:/storage5/dvsbuilds /mnt/dvsbuilds
    fi 

    # Mount /mnt/data
    if [[ -z $(ls /mnt/data 2>/dev/null) ]]; then
        sudo apt install -y nfs-common
        sudo mkdir -p /mnt/data
        sudo mount linuxqa.nvidia.com:/storage/data /mnt/data 
    fi 
}

function Unsandbag-NvTest-Driver {
    driver=$1
    if [[ -z $driver ]]; then 
        if [[ ! -z $NVTEST_DRIVER ]]; then 
            driver=$NVTEST_DRIVER
        else 
            echo "Driver URL/Path is required, aborting!"
            return 1
        fi 
    fi 
    
    if [[ "$driver" == "http"* ]]; then 
        driver=/root/nvt/driver/$(basename "$driver")
    fi 

    pushd /tmp >/dev/null 
        sudo rm -rf /tmp/tests-Linux-$(uname -m)
        tar -xf $(dirname $driver)/tests-Linux-$(uname -m).tar 
        echo "(Nothing prints out if there is no GPU workload)"
        ./tests-Linux-aarch64/sandbag-tool/sandbag-tool -unsandbag
        ./tests-Linux-aarch64/sandbag-tool/sandbag-tool -print 
    popd >/dev/null 
}