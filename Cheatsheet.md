# Set up test ENVVARS
```bash
export PATH="$PATH:$HOME/WZhu/Scripts"
export __GL_SYNC_TO_VBLANK=0
export vblank_mode=0
export __GL_DEBUG_BYPASS_ASSERT=c 
export PIP_BREAK_SYSTEM_PACKAGES=1
[[ -z $SSL_CERT_DIR ]] && export SSL_CERT_DIR=/etc/ssl/certs
[[ -z $DISPLAY ]] && export DISPLAY=:0
```

# Set up P4 ENVVARS
```bash
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
*.code-workspace" > ~/.p4ignore
```

# Set up nvidia certificate
```bash
if ! ls /etc/ssl/certs/certnew*.pem &>/dev/null; then
    if [[ ! -z $(ls /mnt/linuxqa 2>/dev/null) ]]; then
        sudo apt install -y nfs-common
        sudo mkdir -p /mnt/linuxqa
        sudo mount linuxqa.nvidia.com:/storage/people /mnt/linuxqa 
    fi 
    if mountpoint -q /mnt/linuxqa; then
        echo "Installing root certificate"
        sudo cp /mnt/linuxqa/wanliz/certnew.crt /usr/local/share/ca-certificates/
        sudo update-ca-certificates
    fi 
fi
```

# Set up API keys
```bash
# Encrypt: echo -n 'xxx' | openssl enc -e -aes-256-cbc -pbkdf2 -a -A
# Password is z***e
if [[ ! -f ~/.gtl_api_key ]]; then 
    echo 'U2FsdGVkX18BU0ZpoGynLWZBV16VNV2x85CjdpJfF+JF4HhpClt/vTyr6gs6GAq0lDVWvNk7L7s7eTFcJRhEnU4IpABfxIhfktMypWw85PuJCcDXOyZm396F02KjBRwunVfNkhfuinb5y2L6YR9wYbmrGDn1+DjodSWzt1NgoWotCEyYUz0xAIstEV6lF5zedcGwSzHDdFhj3hh5YxQFANL96BFhK9aSUs4Iqs9nQIT9evjinEh5ZKNq5aJsll91czHS2oOi++7mJ9v29sU/QjaqeSWDlneZj4nPYXhZRCw=' | openssl enc -d -aes-256-cbc -pbkdf2 -a > ~/.gtl_api_key 
    chmod 500 ~/.gtl_api_key 
fi 
if [[ ! -f ~/.ssh/id_ed25519 ]]; then 
    echo 'U2FsdGVkX1/M3Vl9RSvWt6Nkq+VfxD/N9C4jr96qvbXsbPfxWmVSfIMGg80m6g946QCdnxBxrNRs0i9M0mijcmJzCCSgjRRgE5sd2I9Buo1Xn6D0p8LWOpBu8ITqMv0rNutj31DKnF5kWv52E1K4MJdW035RHoZVCEefGXC46NxMo88qzerpdShuzLG8e66IId0kEBMRtWucvhGatebqKFppGJtZDKW/W1KteoXC3kcAnry90H70x2fBhtWnnK5QWFZCuoC16z+RQxp8p1apGHbXRx8JStX/om4xZuhl9pSPY47nYoCAOzTfgYLFanrdK10Jp/huf40Z0WkNYBEOH4fSTD7oikLugaP8pcY7/iO0vD7GN4RFwcB413noWEW389smYdU+yZsM6VNntXsWPWBSRTPaIEjaJ0vtq/4pIGaEn61Tt8ZMGe8kKFYVAPYTZg/0bai1ghdA9CHwO9+XKwf0aL2WalWd8Amb6FFQh+TlkqML/guFILv8J/zov70Jxz/v9mReZXSpDGnLKBpc1K1466FnlLJ89buyx/dh/VXJb+15RLQYUkSZou0S2zxo' | openssl enc -d -aes-256-cbc -pbkdf2 -a > ~/.ssh/id_ed25519
    chmod 600 ~/.ssh/id_ed25519
    echo 'ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIHx7hz8+bJjBioa3Rlvmaib8pMSd0XTmRwXwaxrT3hFL wanliz@Enzo-MacBook' > ~/.ssh/id_ed25519.pub 
    chmod 644 ~/.ssh/id_ed25519.pub
fi 
```

# Disable sudo password
```bash
if [[ $UID -ne 0 ]] && ! sudo grep -q "$USER ALL=(ALL) NOPASSWD:ALL" /etc/sudoers; then
    echo "Setting up NoPasswd sudo for $USER"
    echo "$USER ALL=(ALL) NOPASSWD:ALL" | sudo tee -a /etc/sudoers
fi  
```

# Install package dependencies
```bash
for pkg in libopengl0 libnss3 libxkbfile1 \
    libxcb-cursor0 libxcb-icccm4 libxcb-image0 \
    libxcb-keysyms1 libxcb-randr0 libxcb-render-util0 \
    libxcb-xinerama0 libxcb-xkb1 libxkbcommon-x11-0 \
    libxcb-shape0 libxcb1-dev \
    screen x11-xserver-utils sshpass nfs-common \
    xfce4 xfce4-goodies tigervnc-standalone-server x11vnc; do 
    dpkg -s $pkg &>/dev/null || sudo apt install -y $pkg 
done 
```

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

# 