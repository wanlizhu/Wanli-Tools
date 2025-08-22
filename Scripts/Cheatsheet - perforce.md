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