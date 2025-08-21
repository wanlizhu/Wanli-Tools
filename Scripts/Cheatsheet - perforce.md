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
mapfile -t files < <( (
    p4 -Ztag -F "%depotFile%" describe -S "$CL_old" 2>/dev/null || :
    p4 -Ztag -F "%depotFile%" describe -S "$CL_new" 2>/dev/null || :
) | sort -u )
CL_diff_file="CL_diff_${CL_new}_vs_${CL_old}"
: > $CL_diff_file.patch
for f in "${files[@]}"; do
  p4 diff2 -du $([[ -z $branch_spec ]] && echo || echo "-b $branch_spec") "$f@=$CL_old" "$f@=$CL_new" >> $CL_diff_file.patch || :
done
diff2html -i file -s side -F $CL_diff_file.html -- $CL_diff_file.patch
echo "Generated $CL_diff_file.html"
```