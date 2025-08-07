#!/bin/bash

export DISPLAY=:0
echo "export DISPLAY=:0"

if [[ -z $1 ]]; then 
    echo "Starting Xorg using /mnt/linuxqa/nvt.sh"
    dpkg -s tmux &>/dev/null || sudo apt install -y tmux
    sudo tmux new-session -s startx-in-nvtest "NVTEST_NO_SMI=1 NVTEST_NO_RMMOD=1 NVTEST_NO_MODPROBE=1 /mnt/linuxqa/nvt.sh 3840x2160__runcmd --cmd 'sleep 2147483647'; if [[ ! -z \$TMUX ]]; then read -p 'Press [Enter] to exit: '; fi"
    xrandr | grep current
    echo "Don't forget to unsandbag the driver"
elif [[ $1 == -bare ]]; then
    echo "Starting Xorg bare"
    dpkg -s tmux &>/dev/null || sudo apt install -y tmux
    sudo tmux new-session -s "startx-bare" "X :0 +iglx; if [[ ! -z \$TMUX ]]; then read -p 'Press [Enter] to exit: '; fi"
    xrandr --fb 3840x2160 && xrandr | grep current
    echo "Don't forget to unsandbag the driver"
elif [[ $1 == -vnc ]]; then 
    echo "[1] Create a virtual desktop"
    echo "[2] Mirror the current screen"
    read -e -i 1 -p "Select: " vncType

    if [[ $vncType == 1 ]]; then 
        dpkg -s tmux &>/dev/null || sudo apt install -y tmux
        sudo apt install -y xfce4 xfce4-goodies tigervnc-standalone-server &>/dev/null
        echo '#!/bin/sh
unset SESSION_MANAGER
unset DBUS_SESSION_BUS_ADDRESS
startxfce4' > ~/.vnc/xstartup 
        [[ ! -e ~/.vnc/passwd ]] && x11vnc -storepasswd
        tmux new-session -s "vnc-virtual" "vncserver :1 -localhost no; if [[ ! -z \$TMUX ]]; then read -p 'Press [Enter] to exit: '; fi"
    elif [[ $vncType == 2 ]]; then 
        dpkg -s tmux &>/dev/null || sudo apt install -y tmux
        dpkg -s x11vnc &>/dev/null || sudo apt install -y x11vnc
        [[ ! -e ~/.vnc/passwd ]] && x11vnc -storepasswd
        tmux new-session -s "vnc-mirror" "x11vnc -display :0 -forever --loop -noxdamage -repeat -shared; if [[ ! -z \$TMUX ]]; then read -p 'Press [Enter] to exit: '; fi"
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
