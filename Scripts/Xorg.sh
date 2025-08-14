#!/bin/bash

export DISPLAY=:0
echo "export DISPLAY=:0"

if [[ $1 == vnc ]]; then 
    [[ -z $(pidof Xorg) ]] && { echo "Xorg is not running"; exit 1; }
    [[ ! -e ~/.vnc/passwd ]] && x11vnc -storepasswd
    screen -S vnc-mirroring x11vnc -display :0 -auth ~/.Xauthority -noshm -forever --loop -noxdamage -repeat -shared
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
