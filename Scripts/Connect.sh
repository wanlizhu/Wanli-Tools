#!/bin/bash

declare -A hostsInLab=(
    [horizon5]="172.16.178.123"
    [horizon6]="172.16.177.182"
    [horizon7]="172.16.177.216"
)

if [[ ! -z "${hostsInLab[$1]+set}" ]]; then
    sshpass -p nvidia ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no nvidia@${hostsInLab[$1]}
elif [[ $1 == proxy ]]; then 
    [[ ! -f ~/.ssh/id_ed25519 ]] && echo "~/.ssh/id_ed25519 is missing, fall back to password" 
    ssh nvidia@10.176.11.106 
elif [[ $1 == office ]]; then 
    ssh wanliz@172.16.179.143 
fi 
