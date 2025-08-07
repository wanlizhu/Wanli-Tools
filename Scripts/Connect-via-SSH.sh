#!/bin/bash

if [[ -z $1 ]]; then 
    echo "Host name is required"
    exit 1
fi 

if [[ ! -f ~/.ssh/id_ed25519 ]]; then 
    echo '-----BEGIN OPENSSH PRIVATE KEY-----
b3BlbnNzaC1rZXktdjEAAAAABG5vbmUAAAAEbm9uZQAAAAAAAAABAAAAMwAAAAtzc2gtZW
QyNTUxOQAAACB8e4c/PmyYwYqGt0Zb5mom/KTEndF05kcF8Gsa094RSwAAAJhfAHP9XwBz
/QAAAAtzc2gtZWQyNTUxOQAAACB8e4c/PmyYwYqGt0Zb5mom/KTEndF05kcF8Gsa094RSw
AAAECa55qWiuh60rKkJLljELR5X1FhzceY/beegVBrDPv6yXx7hz8+bJjBioa3Rlvmaib8
pMSd0XTmRwXwaxrT3hFLAAAAE3dhbmxpekBFbnpvLU1hY0Jvb2sBAg==
-----END OPENSSH PRIVATE KEY-----' > ~/.ssh/id_ed25519
    chmod 600 ~/.ssh/id_ed25519

    echo 'ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIHx7hz8+bJjBioa3Rlvmaib8pMSd0XTmRwXwaxrT3hFL wanliz@Enzo-MacBook' > ~/.ssh/id_ed25519.pub
    chmod 644 ~/.ssh/id_ed25519.pub
fi 

declare -A hostsInLab=(
    [horizon5]="172.16.178.123"
    [horizon6]="172.16.177.182"
    [horizon7]="172.16.177.216"
)

if [[ ! -z "${hostsInLab[$1]+set}" ]]; then
    dpkg -s sshpass &>/dev/null || sudo apt install -y sshpass 
    sshpass -p nvidia ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no nvidia@${hostsInLab[$1]}
    exit 
elif [[ $1 == proxy ]]; then 
    ssh nvidia@10.176.11.106 # Use public key
fi 

echo "Host not found: $1"