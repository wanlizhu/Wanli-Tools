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

if [[ $1 == env ]]; then
    if [[ -z $2 ]]; then 
        echo "NVTEST_DRIVER           : $NVTEST_DRIVER"
        echo "NVTEST_DRIVER_BRANCH    : $NVTEST_DRIVER_BRANCH"
        echo "NVTEST_DRIVER_CHANGELIST: $NVTEST_DRIVER_CHANGELIST"
        echo "NVTEST_DRIVER_DIR       : $NVTEST_DRIVER_DIR"
        echo "The current GPC Clock: $(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader)"
    elif [[ $2 == setup ]]; then 
        export PATH="$PATH:$HOME/WZhu/Scripts"
        export __GL_SYNC_TO_VBLANK=0
        export vblank_mode=0
        export __GL_DEBUG_BYPASS_ASSERT=c 
        export PIP_BREAK_SYSTEM_PACKAGES=1
        [[ -z $SSL_CERT_DIR ]] && export SSL_CERT_DIR=/etc/ssl/certs
        [[ -z $DISPLAY ]] && export DISPLAY=:0
        [[ $USER == wanliz ]] && {
            export P4PORT=p4proxy-sc.nvidia.com:2006
            export P4USER=wanliz
            export P4CLIENT=wanliz-sw-gpu-driver-office
            export P4ROOT=/sw
            export P4IGNORE=$HOME/.p4ignore
            [[ ! -f ~/.p4ignore ]] && echo "_out
                .git
                .vscode
                .cursorignore
                .clangd
                *.code-workspace" | sed 's/^[[:space:]]*//' > ~/.p4ignore
        }
        if [[ $UID -ne 0 ]] && ! sudo grep -q "$USER ALL=(ALL) NOPASSWD:ALL" /etc/sudoers; then
            echo "$USER ALL=(ALL) NOPASSWD:ALL" | sudo tee -a /etc/sudoers >/dev/null 
        fi  
        if [[ -f ~/WZhu/Scripts/hosts ]]; then 
            while read -r ip host || [ -n "$ip" ]; do 
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
        function wzhu-pull {
            pushd ~/WZhu 
            git pull && source ~/WZhu/Scripts/NvTest.sh env setup 
            popd 
        }
        export -f wzhu-pull 
        function wzhu-push {
            pushd ~/WZhu 
            git add . && git commit -m draft && git push 
            popd 
        }
        export -f wzhu-push 
        function wzhu-scp-to-windows {
            if [[ ! -z $1 ]]; then 
                read -p "Windows Host IP: " -e -i "$(cat $HOME/.windows-host-ip 2>/dev/null)" host
                echo "$host" > $HOME/.windows-host-ip
                [[ -z $(which sshpass) ]] && sudo apt install -y sshpass
                sshpass -p "$(echo 'U2FsdGVkX1+UnE9oAYZ8DjyHzGqQ3wxZbhrJanHFw9u7ypNWEkG2dOJQShrj5dlT' | openssl enc -d -aes-256-cbc -pbkdf2 -a)" scp -r -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null $@ WanliZhu@$host:'C:\Users\WanliZhu\Desktop\'
            fi 
        }
        export -f wzhu-scp-to-windows
        function wzhu-add-keys {
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
        }
        export -f wzhu-add-keys
        function wzhu-enable-pushbuffer-dump {
            [[ -z $1 ]] && { echo "Frame index is missing"; return 1; }
            [[ -z $2 ]] && { echo "File path is missing"; return 1; }
            export __GL_ac12fede=$(( 0x00000001 | 0x00000002 | 0x00000080 | 0x00000100 | 0x00010000 ))
            export __GL_8FCB2E8=$1
            export __GL_6635F0C4=$1
            export __GL_ac12fedf=$2
            echo "Load ENVVARS to enable pushbuffer dump -> OK"
        }
        export -f wzhu-enable-pushbuffer-dump
        function wzhu-disable-pushbuffer-dump {
            unset __GL_ac12fede
            unset __GL_8FCB2E8
            unset __GL_6635F0C4
            unset __GL_ac12fedf
            echo "Load ENVVARS to disable pushbuffer dump -> OK"
        }
        export -f wzhu-disable-pushbuffer-dump
    fi 
elif [[ $1 == driver || $1 == drivers ]]; then 
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
            ( set -o pipefail; ssh wanliz@office "ls /sw/$branch/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/NVIDIA-Linux-$(uname -m)-*-internal.run | awk -F/ '{print $NF}'" | tee /tmp/list ) || { 
                echo "Failed to list compiled versions on remote"
                exit 1
            }

            echo "Remote folder: /sw/$branch/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config"
            rsync -ah --progress wanliz@office:/sw/$branch/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/NVIDIA-Linux-$(uname -m)-*-internal.run wanliz@office:/sw/$branch/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/tests-Linux-$(uname -m).tar $HOME && echo || exit 1
            
            if [[ "$(wc -l < /tmp/list)" -gt 1 ]]; then
                $0 driver local $HOME || exit 1
            elif [[ "$(wc -l < /tmp/list)" == 1 ]]; then 
                version=$(cat /tmp/list | head -1 | awk -F- '{print $4}' | sed 's/\.run$//')
                $0 driver local $HOME/NVIDIA-Linux-$(uname -m)-$version-internal.run || exit 1
            else
                echo "The remote folder has no driver package"
                exit 1
            fi 

            if [[ $config != release && -z $(ls /sw 2>/dev/null) ]]; then
                echo "Mounting remote source at /sw"
                [[ ! -d /sw ]] && sudo mkdir /sw && sudo chmod 777 /sw && sudo chown $USER /sw
                sudo apt install -y nfs-common &>/dev/null 
                sudo mount -t nfs office:/sw /sw
            fi 
        elif [[ $module == opengl ]]; then 
            echo "Remote folder: /sw/$branch/drivers/OpenGL/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config"
            rsync -ah --progress wanliz@office:/sw/$branch/drivers/OpenGL/_out/Linux_$(uname -m | sed 's/^x86_64$/amd64/')_$config/libnvidia-glcore.so $HOME || exit 1
            $0 driver local $HOME/libnvidia-glcore.so
        fi 
    elif [[ $2 == local ]]; then
        if [[ -d "$3" ]]; then 
            echo && ls $3/NVIDIA-Linux-$(uname -m)-*-internal.run 2>/dev/null | awk -F/ '{print $NF}' | sort -V | tee /tmp/drivers | sed -E 's/([0-9]+\.[0-9]+)/\x1b[31m\1\x1b[0m/'
            if [[ -s /tmp/drivers ]]; then 
                read -p "Enter [version] to continue: " version
                $0 driver local $3/NVIDIA-Linux-$(uname -m)-$version-internal.run
            else
                echo "Found no driver package in $3"; exit 1 
            fi 
        elif [[ $3 == *".run" ]]; then 
            echo "Kill all graphics apps and install $3"
            read -p "Press [Enter] to continue: "
            sudo fuser -v /dev/nvidia* 2>/dev/null | grep -v 'COMMAND' | awk '{print $3}' | sort | uniq | tee > /tmp/nvidia
            for nvpid in $(cat /tmp/nvidia); do 
                echo -n "Killing $nvpid "
                sudo kill -9 $nvpid && echo "-> OK" || echo "-> Failed"
                sleep 1
            done
            #sudo rmmod -f nvidia_uvm nvidia_drm nvidia_modeset nvidia_fs nvidia  
            while :; do
                removed=0
                for m in $(lsmod | awk '/^nvidia/ {print $1}'); do
                    if [ ! -d "/sys/module/$m/holders" ] || [ -z "$(ls -A /sys/module/$m/holders 2>/dev/null)" ]; then
                        sudo rmmod -f "$m" && removed=1
                        echo "Remove kernel module $m -> OK"
                    fi
                done
                [ "$removed" -eq 0 ] && break
            done

            sudo env IGNORE_CC_MISMATCH=1 IGNORE_MISSING_MODULE_SYMVERS=1 $3 -s --no-kernel-module-source --skip-module-load || { 
                cat /var/log/nvidia-installer.log
                echo "Aborting..."
                exit 1
            }

            unset NVTEST_DRIVER NVTEST_DRIVER_BRANCH NVTEST_DRIVER_CHANGELIST NVTEST_DRIVER_DIR && echo "Unset NVTEST_* envvars -> OK"
            if [[ -f $(dirname $3)/tests-Linux-$(uname -m).tar ]]; then 
                tar -xf $(dirname $3)/tests-Linux-$(uname -m).tar -C $HOME 
                sudo ln -sf $HOME/tests-Linux-$(uname -m)/sandbag-tool/sandbag-tool $HOME/sandbag-tool && echo "Updated: $HOME/sandbag-tool"
            fi 

            read -p "Press [Enter] to run Xorg, unsandbag and lock clocks: "
            $0 startx && $0 maxclock && echo -e "\nDriver Installed!"
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
        
        read -p "Press [Enter] to run Xorg, unsandbag and lock clocks: "
        $0 startx && $0 maxclock && echo -e "\nDriver Installed!"
    fi 
    /bin/bash 
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

    if [[ $enableVNC == 1 ]]; then
        [[ -z $(pidof Xorg) ]] && { echo "Xorg is not running"; exit 1; }
        [[ ! -e ~/.vnc/passwd ]] && x11vnc -storepasswd
        screen -S vnc-mirroring x11vnc -display $DISPLAY -auth ~/.Xauthority -noshm -forever --loop -noxdamage -repeat -shared
        sleep 2
        sudo ss -tulpn | grep -E "5900|5901|5902"
    fi 
elif [[ $1 == xauth ]]; then 
    echo "Xorg PID: $(pidof Xorg)"
    ( set -o pipefail; xrandr | grep current ) || { 
        echo "Trying to reconfig Xauthority for $USER in SSH session"
        xauthPath=$(ps aux | grep '[X]org' | grep -oP '(?<=-auth )[^ ]+')
        if [[ -f $xauthPath ]]; then 
            sudo cp -vf $xauthPath ~/.Xauthority
            sudo chown $USER:$(id -gn) ~/.Xauthority
            chmod 600 ~/.Xauthority
            export XAUTHORITY=~/.Xauthority
            sudo cp -vf $xauthPath /root/.Xauthority
            sudo chown root:root /root/.Xauthority
            sudo chmod 600 /root/.Xauthority
        else
            echo "The current Xorg was launched without -auth option"
        fi 
        ( set -o pipefail; xrandr | grep current ) || { 
            ls -al /tmp/.X11-unix/
            exit 1
        }
    }
    if [[ $UID != 0 ]]; then 
        sudo -H bash -lc "echo '[Running as root]'; cp -vf $HOME/.Xauthority /root/"
    fi 
elif [[ $1 == viewperf ]]; then 
    [[ -z $WZHU_VP_ROOT ]] && WZHU_VP_ROOT=/root/nvt/tests/viewperf2020v3/viewperf2020
    exe="./viewperf/bin/viewperf"
    arg="viewsets/$2/config/$2.xml $3 -resolution 3840x2160"
    dir="$WZHU_VP_ROOT"
    postproc=
    
    if [[ $WZHU_PUSHBUF == 1 ]]; then 
        read -p "Frame index to dump pushbuffer at: " -e -i 100 index
        wzhu-enable-pushbuffer-dump $index $HOME/pushbuffer-viewperf-$2-subtest$3-frame$index-on-$(hostname).xml || exit 1
        postproc="sed -i '1{/<\/FRAME>/d}' $HOME/pushbuffer-viewperf-$2-subtest$3-frame$index-on-$(hostname).xml && sed -i '$ { /<\/FRAME>/! s/$/\n<\/FRAME>/ }' $HOME/pushbuffer-viewperf-$2-subtest$3-frame$index-on-$(hostname).xml"
    fi 

    exportEnvs=$(env | grep -E '^(__GL_|WZHU_)' | while IFS='=' read -r k v; do printf 'export %s=%q; ' $k $v; done)

    if [[ $WZHU_PI == 1 ]]; then # Capture PI report
        commandLine="$exportEnvs cd $(pwd) && rm -rf $HOME/SinglePassCapture/PerfInspector/output/viewperf-$2-subtest$3-on-$(hostname)$WZHU_PI_SUFFIX && $HOME/SinglePassCapture/pic-x $4 --api=ogl --check_clocks=0 --sample=24000 --aftbuffersize=2048 --name=viewperf-$2-subtest$3-on-$(hostname)$WZHU_PI_SUFFIX --startframe=100 --exe=$exe --arg=\"$arg\" --workdir=$dir | grep -v \"won't hook API\" && sudo -u $USER -H bash -lc \"source $HOME/SinglePassCapture/PerfInspector/Python-venv/bin/activate && NVM_GTLAPI_USER=wanliz $HOME/SinglePassCapture/PerfInspector/output/viewperf-$2-subtest$3-on-$(hostname)$WZHU_PI_SUFFIX/upload_report.sh\"" 
    else
        if [[ ! -z $2 ]]; then # Run single viewset
            if [[ $WZHU_GDB == 1 ]]; then # Run in gdb
                [[ -z $(which cgdb) ]] && sudo apt install -y cgdb
                commandLine="$exportEnvs cd $dir && cgdb -ex 'set trace-commands on' -ex 'set pagination off' -ex 'set confirm off' -ex 'set debuginfod enabled on' -ex 'set breakpoint pending on' -ex \"file $exe\" -ex \"set args $arg\" -ex 'set trace-commands off'"
            else # Regular run
                commandLine="$exportEnvs cd $WZHU_VP_ROOT && ./viewperf/bin/viewperf viewsets/$2/config/$2.xml $3 -resolution 3840x2160 && cat $WZHU_VP_ROOT/results/$2*/results.xml"
                if [[ ! -z $postproc ]]; then 
                    commandLine+=" && eval \"$postproc\""
                fi 
            fi 
        else # Run all viewsets
            commandLine="$exportEnvs cd $dir; rm -rf /tmp/viewperf"
            for viewset in catia creo energy maya medical snx sw; do 
                commandLine+="  ; $exe viewsets/$viewset/config/$viewset.xml -resolution 3840x2160 && cat $WZHU_VP_ROOT/results/${viewset//sw/solidworks}*/results.xml >> /tmp/viewperf"
            done 
            commandLine+=" ; Visualize-Viewperf-Results.py /tmp/viewperf"
        fi 
    fi 

    echo "${commandLine}"
    read -p "Press [Enter] to continue as root: "
    sudo -H bash -lc "$commandLine" 
else 
    exportEnvs=$(env | grep -E '^(__GL_|WZHU_)' | while IFS='=' read -r k v; do printf 'export %s=%q; ' $k $v; done)
    sudo -H bash -lc "$exportEnvs /mnt/linuxqa/nvt.sh $*" 
fi 