#!/bin/bash

sudo apt autoremove &>/dev/null 
sudo apt update &>/dev/null 

if [[ $UID -ne 0 ]] && ! sudo grep -q "$USER ALL=(ALL) NOPASSWD:ALL" /etc/sudoers; then
    echo "Setting up NoPasswd sudo for $USER"
    echo "$USER ALL=(ALL) NOPASSWD:ALL" | sudo tee -a /etc/sudoers
fi 

if ! grep -q "WZhu Env Setup" ~/.bashrc; then
    echo "Setting up ~/.bashrc"
    echo ' 
# === WZhu Env Setup ===
export PATH="$PATH:$HOME/WZhu/Scripts"
export __GL_SYNC_TO_VBLANK=0
export vblank_mode=0
export __GL_DEBUG_BYPASS_ASSERT=c 
export PIP_BREAK_SYSTEM_PACKAGES=1
[[ -z $SSL_CERT_DIR ]] && export SSL_CERT_DIR=/etc/ssl/certs
[[ -z $DISPLAY ]] && export DISPLAY=:0

export P4PORT=p4proxy-sc.nvidia.com:2006
export P4USER=wanliz
export P4CLIENT=wanliz-sw-gpu-driver-home
export P4IGNORE=$HOME/.p4ignore
' >> ~/.bashrc
fi 

if [ ! -f ~/.p4ignore ]; then 
    echo "Setting up ~/.p4ignore"
    echo "_out
.git
.vscode
.cursorignore
.clangd
*.code-workspace" > ~/.p4ignore
fi 

# Install package dependencies
for pkg in libopengl0 libnss3 libxkbfile1 \
    libxcb-cursor0 libxcb-icccm4 libxcb-image0 \
    libxcb-keysyms1 libxcb-randr0 libxcb-render-util0 \
    libxcb-xinerama0 libxcb-xkb1 libxkbcommon-x11-0 \
    libxcb-shape0 libxcb1-dev \
    screen x11-xserver-utils sshpass nfs-common \
    xfce4 xfce4-goodies tigervnc-standalone-server x11vnc; do 
    dpkg -s $pkg &>/dev/null || sudo apt install -y $pkg 
done 

if ! dpkg --print-foreign-architectures | grep -q '^i386$'; then
    echo "Enabling i386 architecture"
    sudo dpkg --add-architecture i386
    sudo apt update 
    sudo apt install -y libc6:i386 libncurses6:i386 libstdc++6:i386
fi

# Install package dependencies
for pkg in libelf-dev elfutils; do 
    dpkg -s $pkg &>/dev/null || sudo apt install -y $pkg 
done 

# Install certificate for code signing (optional)
if ! ls /etc/ssl/certs/certnew*.pem &>/dev/null; then
    if [[ ! -z $(ls /mnt/linuxqa 2>/dev/null) ]]; then
        sudo apt install -y nfs-common
        sudo mkdir -p /mnt/linuxqa
        sudo mount linuxqa.nvidia.com:/storage/people /mnt/linuxqa 
    fi 
    if mountpoint -q /mnt/linuxqa; then
        echo "Installing root certificate"
        sudo cp /mnt/linuxqa/wanliz/certnew.crt /usr/local/share/ca-certificates/
        sudo update-ca-certificates
    fi 
fi

source ~/.bashrc
/bin/bash