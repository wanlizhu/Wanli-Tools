#!/bin/bash

if [[ -z $1 ]]; then 
    echo "Host name is required"
    exit 1
fi 

if [[ ! -f ~/.ssh/id_ed25519 ]]; then 
    echo "Run commands in \"SSH Key to access NvTest.txt\" to generate ~/.ssh/id_ed25519"
fi 

declare -A hostsInLab=(
    [horizon5]="172.16.178.123"
    [horizon6]="172.16.177.182"
    [horizon7]="172.16.177.216"
)

if [[ ! -z "${hostsInLab[$1]+set}" ]]; then
    sshpass -p nvidia ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no nvidia@${hostsInLab[$1]}
    exit 
elif [[ $1 == proxy ]]; then 
    ssh nvidia@10.176.11.106 # Use public key
fi 

echo "Host not found: $1"