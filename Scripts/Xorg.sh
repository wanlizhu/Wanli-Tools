#!/bin/bash

descriptor=$(find /tmp/.X11-unix -maxdepth 1 -type s -name 'X*' -printf '%f\n' 2>/dev/null | sed 's/^X//' | sort -n | head -1) 
if [[ ! -z $descriptor ]]; then 
    export DISPLAY=":$descriptor"
    echo "export DISPLAY=:$descriptor"
fi 

if [[ $1 == vnc ]]; then 
    [[ -z $(pidof Xorg) ]] && { echo "Xorg is not running"; exit 1; }
    [[ ! -e ~/.vnc/passwd ]] && x11vnc -storepasswd
    screen -S vnc-mirroring x11vnc -display $DISPLAY -auth ~/.Xauthority -noshm -forever --loop -noxdamage -repeat -shared
    sudo ss -tulpn | grep -E "5900|5901|5902"
elif [[ $1 == xauth ]]; then 
    xauthPath=$(ps aux | grep '[X]org' | grep -oP '(?<=-auth )[^ ]+')
    sudo cp -vf $xauthPath ~/.Xauthority
    sudo chown $USER:$(id -gn) ~/.Xauthority
    chmod 600 ~/.Xauthority
    export XAUTHORITY=~/.Xauthority

    sudo cp -vf $xauthPath /root/.Xauthority
    sudo chown root:root /root/.Xauthority
    sudo chmod 600 /root/.Xauthority
fi 
