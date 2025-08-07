#!/bin/bash

function Unsandbag-NvTest-Driver {
    driver=$1
    if [[ "$1" == "http"* ]]; then 
        driver=/root/nvt/driver/$(basename "$1")
    fi 
    pushd /tmp >/dev/null 
        sudo rm -rf /tmp/tests-Linux-$(uname -m)
        tar -xf $(dirname $driver)/tests-Linux-$(uname -m).tar
        ./tests-Linux-aarch64/sandbag-tool/sandbag-tool -unsandbag
        ./tests-Linux-aarch64/sandbag-tool/sandbag-tool -print 
    popd >/dev/null 
}