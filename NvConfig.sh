#!/bin/bash

export PATH="$PATH:$HOME/Wanli_Tools:/mnt/linuxqa/wanliz/Wanli_Tools"
export __GL_SYNC_TO_VBLANK=0
export vblank_mode=0
export __GL_DEBUG_BYPASS_ASSERT=c 
export PIP_BREAK_SYSTEM_PACKAGES=1
export NVM_GTLAPI_USER=wanliz
export QT_QPA_PLATFORM_PLUGIN_PATH="/usr/lib/$(uname -m)-linux-gnu/qt5/plugins/platforms" # For qapitrace
[[ -z $SSL_CERT_DIR ]] && export SSL_CERT_DIR=/etc/ssl/certs
[[ -z $DISPLAY ]] && export DISPLAY=:0
# Export Perforce variables 
export P4PORT=p4proxy-sc.nvidia.com:2006
export P4USER=wanliz
export P4CLIENT=wanliz_sw_linux
export P4ROOT=/wanliz_sw_linux
export P4IGNORE=$HOME/.p4ignore
[[ ! -f ~/.p4ignore ]] \
    && echo "_out
    .git
    .vscode
    .cursorignore
    .clangd
    compile-commands.json
    *.code-workspace" | sed 's/^[[:space:]]*//' > ~/.p4ignore
# Mount /mnt/linuxqa
ping -c1 -W1 linuxqa.nvidia.com &>/dev/null \
    && ! mountpoint -q /mnt/linuxqa &>/dev/null \
    && sudo mkdir -p /mnt/linuxqa &>/dev/null \
    && sudo mount linuxqa.nvidia.com:/storage/people /mnt/linuxqa &>/dev/null
# Enable no-password sudo
[[ $EUID -ne 0 ]] \
    && ! sudo grep -qxF "$USER ALL=(ALL) NOPASSWD:ALL" /etc/sudoers \
    && echo "$USER ALL=(ALL) NOPASSWD:ALL" | sudo tee -a /etc/sudoers &>/dev/null
# Add known host names to /etc/hosts
declare -A _address_=(
    [office]="172.16.179.143"
    [proxy]="10.176.11.106"
    [horizon5]="172.16.178.123"
    [horizon6]="172.16.177.182"
    [horizon7]="172.16.177.216"
    [n1x6]="10.31.40.241"
)
for host in "${!_address_[@]}"; do 
    ip=${_address_[$host]}
    if sudo grep -Eq "^[[:space:]]*[^#].*\b${host}\b" /etc/hosts; then
        sudo sed -Ei "/^[[:space:]]*#/! s/^([[:space:]]*)[0-9A-Fa-f:.]+([[:space:]][^#]*\<${host}\>[^#]*)/\1${ip}\2/" /etc/hosts 
    else
        printf '%s %s\n' "$ip" "$host" | sudo tee -a /etc/hosts &>/dev/null
    fi
done 