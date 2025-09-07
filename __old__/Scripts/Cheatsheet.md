# Download internal version of Nsight
Nsight graphics: https://ngfx/builds-nightly/Grfx/ or https://ngfx/builds-prerel/Grfx/ (No L4T version)
Nsight systems : https://urm.nvidia.com/artifactory/swdt-nsys-generic/ctk/ 
```bash
read -p "Nsight graphics .tar.gz URL: " URL
wget --no-check-certificate -P $HOME $URL && tar -zxf $HOME/$(basename "$URL") -C $HOME && {
    if [[ "$URL" == *"EmbeddedLinux"* ]]; then 
        sudo ln -sfv $HOME/nvidia-nomad-internal-EmbeddedLinux.*/host/linux-*-nomad-*/ngfx-ui /usr/bin/ngfx-ui-for-embeddedlinux-internal
    else
        sudo ln -sfv $HOME/nvidia-nomad-internal-Linux.linux/host/linux-desktop-nomad-*/ngfx-ui /usr/bin/ngfx-ui-for-linux-internal
    fi
}
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

# Perforce cherry-pick from bfm to r580
```bash
read -p "Cherry-pick CL: " CL
P4CLIENT=wanliz-sw-gpu-driver-office p4 unshelve -s $CL -b bfm_r580
```

# Perforce show diff as HTML page
```bash
read -e -i 36431669 -p "CL# (old): " CL_old
read -e -i 36425248 -p "CL# (new): " CL_new
mapfile -t files < <( ( 
    p4 files "@=${CL_old}" | awk '{print $1}' | sed -E 's/#[0-9]+//'
    p4 files "@=${CL_new}" | awk '{print $1}' | sed -E 's/#[0-9]+//'
) | sort -u )
CL_diff_file="$HOME/CL_diff_${CL_new}_vs_${CL_old}"
: > $CL_diff_file.patch
for file in "${files[@]}"; do
    echo "$file -> $CL_old vs $CL_new"
    block=$(P4PAGER=cat p4 diff2 -du "${file}@=${CL_old}" "${file}@=${CL_new}" | sed -n '/^@@ /,$p' || true)
    [[ -z $block ]] && continue 
    printf '%s a/%s\n+++ b/%s\n' '---' "${file}" "${file}" >> $CL_diff_file.patch 
    printf '%s\n' "$block" >> $CL_diff_file.patch
done
if [[ ! -s $CL_diff_file.patch ]]; then
    echo "No diffs between shelved $CL_old and $CL_new"
else
    [[ -z $(which diff2html) ]] && sudo apt install -y nodejs npm && sudo npm i -g diff2html-cli 
    npx -y diff2html-cli -i file -s side -F $CL_diff_file.html -- $CL_diff_file.patch && sed -i "s|<h1>.*</h1>|<h1>${CL_new} (new) vs ${CL_old} (old)</h1>|" $CL_diff_file.html && echo "Generated $CL_diff_file.html"
fi
```

# Mount Windows shared folder on Linux
```bash
# \\builds\PreRel\devtools
[[ -z $(which mount.cifs) ]] && sudo apt install -y cifs-utils
read -r -p "URL: " URL
URL=$(echo "$URL" | sed 's|\\|/|g')
sudo mkdir -p /mnt/$(basename $URL).cifs
sudo mount -t cifs $URL /mnt/$(basename $URL).cifs -o username=wanliz && echo ' ... OK' || echo ' ... FAILED'
```

# Upload PI report (using a working build)
```bash
read -p "PI report path: " report
read -p "PI package dir: " dir
rsync -ah --info=progress2 "${report%/}" "$dir/PerfInspector/output/" && {
    name=$(basename "$report")
    if [[ "$dir" == *:* ]]; then
        ssh ${dir%%:*} bash -c "cd ${dir#*:}/PerfInspector/output/$name && ./upload_report.sh"
    else
        pushd $dir/PerfInspector/output/$name &&
        ./upload_report.sh &&
        popd 
    fi 
}
```