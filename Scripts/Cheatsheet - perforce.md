# Perforce cherry-pick from bfm to r580
```bash
read -p "Cherry-pick CL: " CL
P4CLIENT=wanliz-sw-gpu-driver-office p4 unshelve -s $CL -b bfm_r580
```

# Perforce show diff as HTML page
```bash
read -e -i 36118946 -p "CL# (old): " CL_old
read -e -i 36425248 -p "CL# (new): " CL_new
read -e -i bfm_r580 -p "Branch spec (optional): " branch_spec
mapfile -t files_in_old < <(p4 -Ztag -F "%depotFile%" describe -S "$CL_old" 2>/dev/null | sort -u)
mapfile -t files_in_new < <(p4 -Ztag -F "%depotFile%" describe -S "$CL_new" 2>/dev/null | sort -u)
CL_diff_file="$HOME/CL_diff_${CL_new}_vs_${CL_old}"
: > $CL_diff_file.patch
for file in "${files_in_old[@]}"; do
    echo "$file@=$CL_old vs @=$CL_new"
    p4 diff2 -du $([[ -z $branch_spec ]] && echo || echo "-b $branch_spec") "$file@=$CL_old" "@=$CL_new" >> $CL_diff_file.patch || :
done
readarray -t files_in_new_add < <( comm -13 <(printf "%s\n" "${files_in_old[@]}") <(printf "%s\n" "${files_in_new[@]}") )
if ((${#files_in_new_add[@]})); then
    for new_add in "${files_in_new_add[@]}"; do 
        echo "$new_add@=$CL_new vs @=$CL_old"
        p4 diff2 -du $([[ -z $branch_spec ]] && echo || echo "-b $branch_spec") -r "$new_add@=$CL_new" "@=$CL_old" >> $CL_diff_file.patch || :
    done
fi
[[ -z $(which diff2html) ]] && sudo apt install -y nodejs npm && sudo npm i -g diff2html-cli 
npx -y diff2html-cli -i file -s side -F $CL_diff_file.html -- $CL_diff_file.patch && echo "Generated $CL_diff_file.html"
```