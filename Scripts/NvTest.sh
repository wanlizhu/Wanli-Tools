#!/bin/bash

if [[ -z $(ls /mnt/linuxqa 2>/dev/null) ]]; then
    sudo mkdir -p /mnt/linuxqa
    sudo mount linuxqa.nvidia.com:/storage/people /mnt/linuxqa || exit 1
fi 
if [[ -z $(ls /mnt/builds 2>/dev/null) ]]; then
    sudo mkdir -p /mnt/builds
    sudo mount linuxqa.nvidia.com:/storage3/builds /mnt/builds  
fi 
if [[ -z $(ls /mnt/dvsbuilds 2>/dev/null) ]]; then
    sudo mkdir -p /mnt/dvsbuilds
    sudo mount linuxqa.nvidia.com:/storage5/dvsbuilds /mnt/dvsbuilds
fi 
if [[ -z $(ls /mnt/data 2>/dev/null) ]]; then
    sudo mkdir -p /mnt/data
    sudo mount linuxqa.nvidia.com:/storage/data /mnt/data 
fi 

if [[ ! -z $(ls /root/nvt 2>/dev/null) ]]; then
    sudo apt install -y python3 python3-pip libjpeg-dev 
    sudo -H bash -lc "/mnt/linuxqa/nvt.sh sync" || exit 1
fi 

if [[ $1 == driver || $1 == drivers ]]; then 
    if [[ $2 == rsync ]]; then
        branch=rel/gpu_drv/r580/r580_00
        module=drivers 
        config=develop
        while [[ $# -gt 2 ]]; do 
            case $3 in 
                bfm) branch=dev/gpu_drv/bugfix_main ;;
                opengl) module=$3 ;;
                debug|release|develop) config=$3 ;;
            esac
            shift 
        done 
        if [[ $module == drivers ]]; then 
            rsync -ah --progress wanliz@office:/media/wanliz/data/wanliz-sw-gpu-driver-office/$branch/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/NVIDIA-Linux-$(uname -m)-*-internal.run wanliz@office:/media/wanliz/data/wanliz-sw-gpu-driver-office/$branch/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/tests-Linux-$(uname -m).tar $HOME && echo || exit 1

            ls $HOME/NVIDIA-Linux-$(uname -m)-*-internal.run | awk -F/ '{print $NF}'  | sort -V 
            read -p "Enter [version] to continue: " version
            $0 driver local $HOME/NVIDIA-Linux-$(uname -m)-$version-internal.run
        elif [[ $module == opengl ]]; then 
            rsync -ah --progress wanliz@office:/media/wanliz/data/wanliz-sw-gpu-driver-office/$branch/drivers/OpenGL/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/libnvidia-glcore.so $HOME || exit 1
            $0 driver local $HOME/libnvidia-glcore.so
        fi 
    elif [[ $2 == local ]]; then
        if [[ $3 == *".run" ]]; then 
            for nvpid in $(sudo fuser -v /dev/nvidia* 2>/dev/null | grep -v 'COMMAND' | awk '{print $3}' | sort  | uniq); do 
                sudo kill -9 $nvpid && echo "Killed $nvpid"
            done
            sudo env IGNORE_CC_MISMATCH=1 IGNORE_MISSING_MODULE_SYMVERS=1 $3 -s --no-kernel-module-source --skip-module-load || { cat /var/log/nvidia-installer.log; exit 1; }
            unset NVTEST_DRIVER NVTEST_DRIVER_BRANCH NVTEST_DRIVER_CHANGELIST NVTEST_DRIVER_DIR && echo "Unset NVTEST_* envvars -> OK"
            if [[ -f $(dirname $3)/tests-Linux-$(uname -m).tar ]]; then 
                tar -xf $(dirname $3)/tests-Linux-$(uname -m).tar -C $HOME 
                sudo ln -sf $HOME/tests-Linux-$(uname -m)/sandbag-tool/sandbag-tool $HOME/sandbag-tool && echo "Updated: $HOME/sandbag-tool"
            fi 
        elif [[ $3 == *".so" ]]; then 
            version=$(ls /usr/lib/*-linux-gnu/$(basename $3).*  | awk -F '.so.' '{print $2}' | head -1)
            sudo cp -vf --remove-destination $3 /usr/lib/$(uname -m)-linux-gnu/$(basename $3).$version 
        fi 
    else
        sudo -H bash -lc "NVTEST_INSTALLER_REUSE_INSTALL=False /mnt/linuxqa/nvt.sh $*" || exit 1
        read -p "ENVVARS (Copy & Paste): " envvars
        for pair in $envvars; do 
            echo "export ${pair%%=*}=${pair#*=}"
            export ${pair%%=*}=${pair#*=}
        done 
        sudo ln -sf /root/nvt/tests/system/sandbag-tool/sandbag-tool $HOME/sandbag-tool && echo "Updated: $HOME/sandbag-tool"
    fi 
    /bin/bash 
elif [[ $1 == env ]]; then
    if [[ -z $2 ]]; then 
        echo "NVTEST_DRIVER           : $NVTEST_DRIVER"
        echo "NVTEST_DRIVER_BRANCH    : $NVTEST_DRIVER_BRANCH"
        echo "NVTEST_DRIVER_CHANGELIST: $NVTEST_DRIVER_CHANGELIST"
        echo "NVTEST_DRIVER_DIR       : $NVTEST_DRIVER_DIR"
        echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
    elif [[ $2 == setup ]]; then 
        silent=$([[ $3 == silent ]] && echo 1 || echo)
        export PATH="$PATH:$HOME/WZhu/Scripts"
        export __GL_SYNC_TO_VBLANK=0
        export vblank_mode=0
        export __GL_DEBUG_BYPASS_ASSERT=c 
        export PIP_BREAK_SYSTEM_PACKAGES=1
        [[ -z $SSL_CERT_DIR ]] && export SSL_CERT_DIR=/etc/ssl/certs
        [[ -z $DISPLAY ]] && export DISPLAY=:0
        [[ -z $P4CLIENT && $USER == wanliz ]] && {
            export P4PORT=p4proxy-sc.nvidia.com:2006
            export P4USER=wanliz
            export P4CLIENT=wanliz-sw-gpu-driver-office
            export P4ROOT=/media/wanliz/data/$P4CLIENT
            export P4IGNORE=$HOME/.p4ignore
            [[ ! -f ~/.p4ignore ]] && echo "_out
                .git
                .vscode
                .cursorignore
                .clangd
                *.code-workspace" | sed 's/^[[:space:]]*//' > ~/.p4ignore
            [[ -z $silent ]] && echo "Setting up perforce envvars for $P4CLIENT"
        }
        if [[ $UID -ne 0 ]] && ! sudo grep -q "$USER ALL=(ALL) NOPASSWD:ALL" /etc/sudoers; then
            [[ -z $silent ]] && echo "Setting up NoPasswd sudo for $USER"
            echo "$USER ALL=(ALL) NOPASSWD:ALL" | sudo tee -a /etc/sudoers >/dev/null 
        fi  
        if [[ -f ~/WZhu/Scripts/hosts ]]; then 
            while read -r ip host _; do 
                [[ -z $ip || -z $host ]] && continue 
                if grep -qw "$host" /etc/hosts; then
                    if ! grep -qE "^[[:space:]]*$ip[[:space:]].*\b$host\b" /etc/hosts; then
                        sudo sed -i -E "/(^|[[:space:]])$host([[:space:]]|$)/{ s|^[[:space:]]*[0-9A-Fa-f:.]+|$ip| }" /etc/hosts
                    fi
                else
                    echo "$ip $host" | sudo tee -a /etc/hosts >/dev/null 
                fi 
            done < ~/WZhu/Scripts/hosts
        fi 
        if [[ -z $silent ]]; then 
            if ! ls /etc/ssl/certs/certnew*.pem &>/dev/null; then
                if [[ ! -z $(ls /mnt/linuxqa 2>/dev/null) ]]; then
                    sudo apt install -y nfs-common
                    sudo mkdir -p /mnt/linuxqa
                    sudo mount linuxqa.nvidia.com:/storage/people /mnt/linuxqa 
                fi 
                if mountpoint -q /mnt/linuxqa; then
                    echo "Installing nvidia root certificate"
                    sudo cp /mnt/linuxqa/wanliz/certnew.crt /usr/local/share/ca-certificates/
                    sudo update-ca-certificates
                fi 
            fi
            if [[ ! -f ~/.gtl_api_key ]]; then 
                echo "Setting up ~/.gtl_api_key"
                echo 'U2FsdGVkX18BU0ZpoGynLWZBV16VNV2x85CjdpJfF+JF4HhpClt/vTyr6gs6GAq0lDVWvNk7L7s7eTFcJRhEnU4IpABfxIhfktMypWw85PuJCcDXOyZm396F02KjBRwunVfNkhfuinb5y2L6YR9wYbmrGDn1+DjodSWzt1NgoWotCEyYUz0xAIstEV6lF5zedcGwSzHDdFhj3hh5YxQFANL96BFhK9aSUs4Iqs9nQIT9evjinEh5ZKNq5aJsll91czHS2oOi++7mJ9v29sU/QjaqeSWDlneZj4nPYXhZRCw=' | openssl enc -d -aes-256-cbc -pbkdf2 -a > ~/.gtl_api_key 
                chmod 500 ~/.gtl_api_key 
            fi 
            if [[ ! -f ~/.ssh/id_ed25519 ]]; then 
                echo "Setting up SSH key"
                echo 'U2FsdGVkX1/M3Vl9RSvWt6Nkq+VfxD/N9C4jr96qvbXsbPfxWmVSfIMGg80m6g946QCdnxBxrNRs0i9M0mijcmJzCCSgjRRgE5sd2I9Buo1Xn6D0p8LWOpBu8ITqMv0rNutj31DKnF5kWv52E1K4MJdW035RHoZVCEefGXC46NxMo88qzerpdShuzLG8e66IId0kEBMRtWucvhGatebqKFppGJtZDKW/W1KteoXC3kcAnry90H70x2fBhtWnnK5QWFZCuoC16z+RQxp8p1apGHbXRx8JStX/om4xZuhl9pSPY47nYoCAOzTfgYLFanrdK10Jp/huf40Z0WkNYBEOH4fSTD7oikLugaP8pcY7/iO0vD7GN4RFwcB413noWEW389smYdU+yZsM6VNntXsWPWBSRTPaIEjaJ0vtq/4pIGaEn61Tt8ZMGe8kKFYVAPYTZg/0bai1ghdA9CHwO9+XKwf0aL2WalWd8Amb6FFQh+TlkqML/guFILv8J/zov70Jxz/v9mReZXSpDGnLKBpc1K1466FnlLJ89buyx/dh/VXJb+15RLQYUkSZou0S2zxo' | openssl enc -d -aes-256-cbc -pbkdf2 -a > ~/.ssh/id_ed25519
                chmod 600 ~/.ssh/id_ed25519
                echo 'ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIHx7hz8+bJjBioa3Rlvmaib8pMSd0XTmRwXwaxrT3hFL wanliz@Enzo-MacBook' > ~/.ssh/id_ed25519.pub 
                chmod 644 ~/.ssh/id_ed25519.pub
            fi 
        fi 
    fi 
elif [[ $1 == maxclock ]]; then 
    sudo $HOME/sandbag-tool -unsandbag
    sudo $HOME/sandbag-tool -print 
    if [[ $(uname -m) == aarch64 ]]; then 
        perfdebug=/mnt/linuxqa/wanliz/iGPU_vfmax_scripts/perfdebug
        sudo $perfdebug --lock_loose   set pstateId P0 && echo -e "set pstateId -> [OK]\n"
        #sudo $perfdebug --lock_strict  set dramclkkHz 8000000 && echo -e "set dramclkkHz -> [OK]\n"
        sudo $perfdebug --lock_strict  set gpcclkkHz  2000000 && echo -e "set gpcclkkHz  2000MHz -> [OK]\n"
        sudo $perfdebug --lock_loose   set xbarclkkHz 1800000 && echo -e "set xbarclkkHz 1800MHz -> [OK]\n"
        #sudo $perfdebug --lock_loose   set sysclkkHz  1800000 && echo -e "set sysclkkHz  -> [OK]\n"
        sudo $perfdebug --force_regime ffr && echo "Force regime successful"
        echo "" && sleep 3
        echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
        echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
        echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)" 
    fi 
elif [[ $1 == startx ]]; then 
    enableVNC=
    while [[ $# -gt 1 ]]; do 
        case $2 in 
            vnc) enableVNC=1 ;; 
        esac
        shift 
    done 
    [[ -z $DISPLAY ]] && export DISPLAY=:0
    if [[ -z $NVTEST_DRIVER ]]; then
        screen -S bare-xorg bash -c "sudo X $DISPLAY +iglx || read -p 'Press [Enter] to exit: '"
        xrandr --fb 3840x2160  
    else
        sudo -H bash -lc "screen -S nvtest-fake-display bash -c \"NVTEST_NO_SMI=1 NVTEST_NO_RMMOD=1 NVTEST_NO_MODPROBE=1 /mnt/linuxqa/nvt.sh 3840x2160__runcmd --cmd 'sleep 2147483647'  || read -p 'Press [Enter] to exit: '\""
    fi 

    $0 xauth 
elif [[ $1 == xauth ]]; then 
    echo "Xorg PID: $(pidof Xorg)"
    ( set -o pipefail; xrandr | grep current ) || { 
        echo "Trying to reconfig Xauthority for $USER in SSH session"
        xauthPath=$(ps aux | grep '[X]org' | grep -oP '(?<=-auth )[^ ]+')
        sudo cp -vf $xauthPath ~/.Xauthority
        sudo chown $USER:$(id -gn) ~/.Xauthority
        chmod 600 ~/.Xauthority
        export XAUTHORITY=~/.Xauthority
        sudo cp -vf $xauthPath /root/.Xauthority
        sudo chown root:root /root/.Xauthority
        sudo chmod 600 /root/.Xauthority
        ( set -o pipefail; xrandr | grep current ) || { 
            ls -al /tmp/.X11-unix/
            exit 1
        }
    }

    if [[ $enableVNC == 1 ]]; then
        [[ -z $(pidof Xorg) ]] && { echo "Xorg is not running"; exit 1; }
        [[ ! -e ~/.vnc/passwd ]] && x11vnc -storepasswd
        screen -S vnc-mirroring x11vnc -display $DISPLAY -auth ~/.Xauthority -noshm -forever --loop -noxdamage -repeat -shared
        sleep 2
        sudo ss -tulpn | grep -E "5900|5901|5902"
    fi 
elif [[ $1 == viewperf ]]; then 
    # $2: viewset, $3: subtest, [$4: optional pic-x args]
    GL_ENV=$(env | grep -E '^(__GL_|WZHU_)' | while IFS='=' read -r k v; do printf 'export %s=%q; ' $k $v; done)
    [[ -z $WZHU_VP_ROOT ]] && WZHU_VP_ROOT=/root/nvt/tests/viewperf2020v3/viewperf2020
    if [[ $WZHU_PI == 1 ]]; then 
        commandLine="$GL_ENV cd $(pwd) && rm -rf $HOME/SinglePassCapture/PerfInspector/output/viewperf-$2-subtest$3-on-$(hostname)$WZHU_PI_SUFFIX && $HOME/SinglePassCapture/pic-x $4 --api=ogl --check_clocks=0 --sample=24000 --aftbuffersize=2048 --name=viewperf-$2-subtest$3-on-$(hostname)$WZHU_PI_SUFFIX --startframe=100 --exe=./viewperf/bin/viewperf --arg=\"viewsets/$2/config/$2.xml $3 -resolution 3840x2160\" --workdir=$WZHU_VP_ROOT | grep -v \"won't hook API\" && sudo -u $USER -H bash -lc \"source $HOME/SinglePassCapture/PerfInspector/Python-venv/bin/activate && NVM_GTLAPI_USER=wanliz $HOME/SinglePassCapture/PerfInspector/output/viewperf-$2-subtest$3-on-$(hostname)$WZHU_PI_SUFFIX/upload_report.sh\"" 
    else
        if [[ ! -z $2 ]]; then 
            commandLine="$GL_ENV cd $WZHU_VP_ROOT && ./viewperf/bin/viewperf viewsets/$2/config/$2.xml $3 -resolution 3840x2160 && cat $WZHU_VP_ROOT/results/$2*/results.xml"
        else
            commandLine="$GL_ENV cd $WZHU_VP_ROOT; rm -rf /tmp/viewperf"
            for viewset in catia creo energy maya medical snx sw; do 
                commandLine+="  ; ./viewperf/bin/viewperf viewsets/$viewset/config/$viewset.xml -resolution 3840x2160 && cat $WZHU_VP_ROOT/results/${viewset//sw/solidworks}*/results.xml >> /tmp/viewperf"
            done 
            commandLine+=" ; python3 Visualize-Viewperf-Results.py /tmp/viewperf"
        fi 
    fi 

    echo "${commandLine}"
    read -p "Press [Enter] to continue as root: "
    sudo -H bash -lc "$commandLine" 
else 
    GL_ENV=$(env | grep -E '^(__GL_|WZHU_)' | while IFS='=' read -r k v; do printf 'export %s=%q; ' $k $v; done)
    sudo -H bash -lc "$GL_ENV /mnt/linuxqa/nvt.sh $*" 
fi 