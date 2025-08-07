#!/bin/bash

function Start-SSH-Session {
    if [[ -z $1 ]]; then 
        echo "Host name is required"
        return 1
    fi 

    declare -A hostsInLab=(
        [proxy]="10.176.11.106"
        [horizon5]="172.16.178.123"
        [horizon6]="172.16.177.182"
        [horizon7]="172.16.177.216"
    )

    if [[ ! -z "${hostsInLab[$1]+set}" ]]; then
        dpkg -s sshpass &>/dev/null || sudo apt install -y sshpass 
        sshpass -p nvidia ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no nvidia@${hostsInLab[$1]}
        return 
    fi 

    echo "Host not found: $1"
}

function Start-Xorg-in-NvTest {
    echo "Starting Xorg using /mnt/linuxqa/nvt.sh"
    dpkg -s tmux &>/dev/null || sudo apt install -y tmux
    sudo tmux new-session -d -s startx-in-nvtest "NVTEST_NO_SMI=1 NVTEST_NO_RMMOD=1 NVTEST_NO_MODPROBE=1 /mnt/linuxqa/nvt.sh 3840x2160__runcmd --cmd 'sleep 2147483647'" && sleep 2
    xrandr | grep current
    read -p "Press [Enter] to unsandbag the driver: " _
    pushd /tmp >/dev/null 
        sudo rm -rf /tmp/tests-Linux-$(uname -m)
        sudo tar -xf /root/nvt/driver/tests-Linux-$(uname -m).tar
        ./tests-Linux-x86_64/sandbag-tool/sandbag-tool -unsandbag
    popd >/dev/null 
}

function Start-Xorg-bare {
    echo "Starting Xorg bare"
    dpkg -s tmux &>/dev/null || sudo apt install -y tmux
    sudo tmux new-session -d -s "startx-bare" "X :0 +iglx" && sleep 2
    xrandr --fb 3840x2160 && xrandr | grep current
    echo "Don't forget to unsandbag the driver"
}

function Start-VNC-Server {
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
        tmux new-session -d -s "vnc-virtual" "vncserver :1 -localhost no"
    elif [[ $vncType == 2 ]]; then 
        dpkg -s tmux &>/dev/null || sudo apt install -y tmux
        dpkg -s x11vnc &>/dev/null || sudo apt install -y x11vnc
        [[ ! -e ~/.vnc/passwd ]] && x11vnc -storepasswd
        tmux new-session -d -s "vnc-mirror" "x11vnc -display :0 -forever --loop -noxdamage -repeat -shared"
    fi 

    sudo ss -tulpn | grep -E "5900|5901|5902"
}

function Config-Xauthority {
    xauthPath=$(ps aux | grep '[X]org' | grep -oP '(?<=-auth )[^ ]+')
    sudo cp -vf $xauthPath ~/.Xauthority
    sudo chown $USER:$(id -gn) ~/.Xauthority
    chmod 600 ~/.Xauthority
    export XAUTHORITY=~/.Xauthority

    sudo cp -vf $xauthPath /root/.Xauthority
    sudo chown root:root /root/.Xauthority
    sudo chmod 600 /root/.Xauthority
}