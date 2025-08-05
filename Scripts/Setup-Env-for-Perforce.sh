$!/bin/bash

if [ ! -f "~/.bashrc" ] || ! grep -q "WZhu NvMake Env" "~/.bashrc"; then
cat << "END_OF_FILE" >> ~/.bashrc

# === WZhu Perforce Env ===
export P4PORT=p4proxy-sc.nvidia.com
export P4USER=wanliz
export P4IGNORE=$HOME/.p4ignore
END_OF_FILE
fi

if [ ! -f "~/.p4ignore" ]; then 
cat << "END_OF_FILE" >> ~/.p4ignore
_out
.git
.vscode
.cursorignore
.clangd
*.code-workspace
END_OF_FILE
fi 

sudo apt install -y libopengl0 libnss3 libxkbfile1 \
    libxcb-cursor0 \
    libxcb1-dev \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-randr0 \
    libxcb-render-util0 \
    libxcb-xinerama0 \
    libxcb-xkb1 \
    libxkbcommon-x11-0

# TODO
# qt.qpa.plugin: From 6.5.0, xcb-cursor0 or libxcb-cursor0 is needed to load the Qt xcb platform plugin.
# qt.qpa.plugin: Could not load the Qt platform plugin "xcb" in "" even though it was found.
# This application failed to start because no Qt platform plugin could be initialized. Reinstalling the application may fix this problem.
# 
# Available platform plugins are: xcb, offscreen, minimal, vnc, vkkhrdisplay.
# 
# Aborted (core dumped)
