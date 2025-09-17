#!/bin/bash

viewset="${1:-maya}"
subtest="${2:-10}"
profile=$3 

if [[ -d ~/viewperf2020v3 ]]; then 
    pushd ~/viewperf2020v3
elif [[ -d /mnt/linuxqa/wanliz/viewperf2020v3/$(uname -m) ]]; then 
    pushd /mnt/linuxqa/wanliz/viewperf2020v3/$(uname -m)
else
    echo "Error: folder not found ~/viewperf2020v3"
    exit 1
fi 

if [[ $viewset == all ]]; then 
    subtest=
    profile=
    for viewset in catia creo energy maya medical snx sw; do 
        ./viewperf/bin/viewperf viewsets/$viewset/config/$viewset.xml $subtest -resolution 3840x2160 &&
        cat results/${viewset//sw/solidworks}*/results.xml
    done 
else
    if [[ $profile == pi ]]; then 
        sudo rm -rf $HOME/SinglePassCapture/PerfInspector/output/viewperf-$viewset-$subtest-on-$(hostname)
        sudo $HOME/SinglePassCapture/pic-x \
            --api=ogl \
            --check_clocks=0 \
            --sample=4800 \
            --aftbuffersize=2048 \
            --startframe=100 \
            --name=viewperf-$viewset-$subtest-on-$(hostname) \
            --exe=viewperf/bin/viewperf \
            --arg="viewsets/$viewset/config/$viewset.xml $subtest -resolution 3840x2160" \
            --workdir=$HOME/viewperf2020v3 && {
            pushd $HOME/SinglePassCapture/PerfInspector/output/viewperf-$viewset-$subtest-on-$(hostname)
            ./upload_report.sh 
            popd 
        }
    else 
        ./viewperf/bin/viewperf viewsets/$viewset/config/$viewset.xml $subtest -resolution 3840x2160 &&
        cat results/${viewset//sw/solidworks}*/results.xml
    fi 
fi 

popd >/dev/null 