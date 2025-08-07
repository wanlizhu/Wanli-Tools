#!/bin/bash

if [[ $UID -ne 0 ]] && ! sudo grep -q "$USER ALL=(ALL) NOPASSWD:ALL" /etc/sudoers; then
    echo "Setting up NoPasswd sudo for $USER"
    echo "$USER ALL=(ALL) NOPASSWD:ALL" | sudo tee -a /etc/sudoers
fi 

if [[ -z $(sudo ls /root/WZhu 2>/dev/null) ]]; then
    echo "Create symbolic link /root/WZhu"
    sudo ln -s $HOME/WZhu /root/WZhu
fi 

if ! grep -q "WZhu Env PATH" ~/.bashrc; then
    echo ' 
# === WZhu Env PATH ===
export PATH="$PATH:$HOME/WZhu/Scripts"' >> ~/.bashrc
fi 

if ! sudo grep -q "WZhu Env PATH" /root/.bashrc; then
    echo '
# === WZhu Env PATH ===
export PATH="$PATH:$HOME/WZhu/Scripts"' | sudo tee -a /root/.bashrc >/dev/null 
fi 

if [ ! -f ~/.bashrc ] || ! grep -q "WZhu Func Loader" ~/.bashrc; then
    echo '
# === WZhu Func Loader ===
function Load-Micro-Functions {
    if [[ -f ~/WZhu/Scripts/MicroFunctions.sh ]]; then 
        source ~/WZhu/Scripts/MicroFunctions.sh
    fi 
}' >> ~/.bashrc
fi 

if ! sudo grep -q "WZhu Func Loader" /root/.bashrc; then
    echo '
# === WZhu Func Loader ===
function Load-Micro-Functions {
    if [[ -f $HOME/WZhu/Scripts/MicroFunctions.sh ]]; then 
        source $HOME/WZhu/Scripts/MicroFunctions.sh
    fi 
}' | sudo tee -a /root/.bashrc >/dev/null 
fi 

if [ ! -f ~/.bashrc ] || ! grep -q "WZhu Perforce Env" ~/.bashrc; then
    echo '
# === WZhu Perforce Env ===
export P4PORT=p4proxy-sc.nvidia.com:2006
export P4USER=wanliz
export P4CLIENT=wanliz-sw-gpu-driver-home
export P4IGNORE=$HOME/.p4ignore' >> ~/.bashrc
fi

if [ ! -f ~/.p4ignore ]; then 
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
    libxcb-shape0 libxcb1-dev; do 
    dpkg -s $pkg &>/dev/null || sudo apt install -y $pkg 
done 

# Run `QT_DEBUG_PLUGINS=1 p4v` to debug if p4v crashed

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