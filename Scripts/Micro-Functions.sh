#!/bin/bash

function Unsandbag-NvTest-Driver {
    driver=$1
    if [[ -z $driver ]]; then 
        echo "Driver URL/Path is required, aborting!"
        return 1
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