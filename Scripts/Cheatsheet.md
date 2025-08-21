# Download file from GTL 
```bash
if [[ -z $(which gtlfs) ]]; then 
    echo "https://gtlfs.nvidia.com/client/linux"
    echo "https://gtlfs.nvidia.com/client/gtlfs.arm64"
else
    read -p "UUID: " UUID
    NVM_GTLAPI_USER=wanliz gtlfs pull $UUID 
fi 
```

# Fix Xauthority issue in SSH session
```bash
xauthPath=$(ps aux | grep '[X]org' | grep -oP '(?<=-auth )[^ ]+')
sudo cp -vf $xauthPath ~/.Xauthority
sudo chown $USER:$(id -gn) ~/.Xauthority
chmod 600 ~/.Xauthority
export XAUTHORITY=~/.Xauthority
sudo cp -vf $xauthPath /root/.Xauthority
sudo chown root:root /root/.Xauthority
sudo chmod 600 /root/.Xauthority
```

# Start a new VNC server (mirroring physical monitor)
```bash
[[ -z $(pidof Xorg) ]] && { echo "Xorg is not running"; exit 1; }
[[ ! -e ~/.vnc/passwd ]] && x11vnc -storepasswd
screen -S vnc-mirroring x11vnc -display $DISPLAY -auth ~/.Xauthority -noshm -forever --loop -noxdamage -repeat -shared
sudo ss -tulpn | grep -E "5900|5901|5902"
```
