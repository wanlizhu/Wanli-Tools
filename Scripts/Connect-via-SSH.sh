#!/bin/bash

if [[ -z $1 ]]; then 
    echo "Host name is required"
    exit 1
fi 

declare -A hostsInLab=(
    [proxy]="10.176.11.106"
    [horizon5]="172.16.178.123"
    [horizon6]="172.16.177.182"
    [horizon7]="172.16.177.216"
)

if [[ ! -z "${hostsInLab[$1]+set}" ]]; then
    dpkg -s sshpass &>/dev/null || sudo apt install -y sshpass 
    sshpass -p nvidia ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no nvidia@${hostsInLab[$1]}
    exit 
fi 

echo "Host not found: $1"