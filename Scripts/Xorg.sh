#!/bin/bash

export DISPLAY=:0
echo "export DISPLAY=:0"

if [[ -z $1 ]]; then 
    echo "Starting Xorg"
    screen -S "startx" sudo X :0 +iglx
    xrandr --fb 3840x2160 && xrandr | grep current
    echo "Don't forget to unsandbag the driver"
elif [[ $1 == -vnc ]]; then 
    echo "[1] Create a virtual desktop"
    echo "[2] Mirror the current screen"
    read -e -i 1 -p "Select: " vncType

    if [[ $vncType == 1 ]]; then 
        sudo apt install -y xfce4 xfce4-goodies tigervnc-standalone-server &>/dev/null
        echo '#!/bin/sh
unset SESSION_MANAGER
unset DBUS_SESSION_BUS_ADDRESS
startxfce4' > ~/.vnc/xstartup 
        [[ ! -e ~/.vnc/passwd ]] && x11vnc -storepasswd

        screen -S vnc-virtual vncserver :1 -localhost no
    elif [[ $vncType == 2 ]]; then 
        dpkg -s x11vnc &>/dev/null || sudo apt install -y x11vnc
        [[ ! -e ~/.vnc/passwd ]] && x11vnc -storepasswd
        
        screen -S startvnc-mirror x11vnc -display :0 -forever --loop -noxdamage -repeat -shared
    fi 

    sudo ss -tulpn | grep -E "5900|5901|5902"
elif [[ $1 == -xauth ]]; then 
    xauthPath=$(ps aux | grep '[X]org' | grep -oP '(?<=-auth )[^ ]+')
    sudo cp -vf $xauthPath ~/.Xauthority
    sudo chown $USER:$(id -gn) ~/.Xauthority
    chmod 600 ~/.Xauthority
    export XAUTHORITY=~/.Xauthority

    sudo cp -vf $xauthPath /root/.Xauthority
    sudo chown root:root /root/.Xauthority
    sudo chmod 600 /root/.Xauthority
fi 
