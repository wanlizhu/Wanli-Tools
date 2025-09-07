#!/bin/bash

viewset=$1
subtest=$2

if [[ -d ~/viewperf2020v3 ]]; then 
    pushd ~/viewperf2020v3
elif [[ -d /mnt/linuxqa/wanliz/viewperf2020v3/$(uname -m) ]]; then 
    pushd /mnt/linuxqa/wanliz/viewperf2020v3/$(uname -m)
else
    echo "Error: folder not found ~/viewperf2020v3"
    exit 1
fi 

if [[ -z $viewset ]]; then 
    for viewset in catia creo energy maya medical snx sw; do 
        ./viewperf/bin/viewperf viewsets/$viewset/config/$viewset.xml $subtest -resolution 3840x2160 &&
        cat results/${viewset//sw/solidworks}*/results.xml
    fi 
else
    ./viewperf/bin/viewperf viewsets/$viewset/config/$viewset.xml $subtest -resolution 3840x2160 &&
    cat results/${viewset//sw/solidworks}*/results.xml
fi 

popd >/dev/null 