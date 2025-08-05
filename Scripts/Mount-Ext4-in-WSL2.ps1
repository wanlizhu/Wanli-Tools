$bindMountPoint = "/mnt/wzhu-linux"

$disks = Get-Disk | Where-Object {
    $_.OperationalStatus -eq 'Online' -and
    -not (Get-Partition -DiskNumber $_.Number | Where-Object { $_.DriveLetter })
}

if (-not $disks -or $disks.Count -eq 0) {
    Write-Host "No available unmounted disks found."
    exit 1
}

Write-Host "`nAvailable unmounted disks:"
foreach ($disk in $disks) {
    $number = $disk.Number
    $name = $disk.FriendlyName
    $sizeGB = "{0:N1}" -f ($disk.Size / 1GB)
    Write-Host "[$number] $name ($sizeGB GB)"
}

$diskNumberInput = Read-Host "`nEnter the disk number to mount into WSL2"
if (-not ($diskNumberInput -match '^\d+$')) {
    Write-Host "Invalid input. Must be a disk number."
    exit 1
}

$diskNumber = [int]$diskNumberInput
$targetDisk = $disks | Where-Object { $_.Number -eq $diskNumber }

if (-not $targetDisk) {
    Write-Host "Disk number $diskNumber not found in the unmounted disk list."
    exit 1
}

$physicalDrive = "\\.\PHYSICALDRIVE$diskNumber"
$mountedPath = "/mnt/wsl/PHYSICALDRIVE$diskNumber"

Write-Host "`nMounting $physicalDrive into WSL2..."
$mountResult = wsl --mount $physicalDrive --type ext4 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "Failed to mount disk: $mountResult"
    exit 1
}

Start-Sleep -Seconds 3

Write-Host "Verifying mount point inside WSL..."
wsl -e bash -c "ls $mountedPath" > $null 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "Mount failed or disk not visible at $mountedPath"
    exit 1
}

Write-Host "Disk mounted at $mountedPath"

Write-Host "Creating bind-mount at $bindMountPoint..."
bindCmd = "sudo mkdir -p $bindMountPoint && sudo mount --bind $mountedPath $bindMountPoint"
wsl -e bash -c $bindCmd
if ($LASTEXITCODE -ne 0) {
    Write-Host "Bind-mount failed."
    exit 1
}

Write-Host "Success: EXT4 disk is now accessible at $bindMountPoint inside WSL2"
