#!/bin/bash

if [ ! -f ~/.bashrc ] || ! grep -q "WZhu Perforce Env" ~/.bashrc; then
cat << "END_OF_FILE" >> ~/.bashrc

# === WZhu Perforce Env ===
export P4PORT=p4proxy-sc.nvidia.com:2006
export P4USER=wanliz
export P4CLIENT=wanliz-sw-gpu-driver-home
export P4IGNORE=$HOME/.p4ignore
END_OF_FILE
fi

if [ ! -f ~/.p4ignore ]; then 
cat << "END_OF_FILE" >> ~/.p4ignore
_out
.git
.vscode
.cursorignore
.clangd
*.code-workspace
END_OF_FILE
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
