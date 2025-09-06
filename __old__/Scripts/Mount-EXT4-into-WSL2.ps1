if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Start-Process powershell -Verb RunAs -ArgumentList ('-NoProfile -ExecutionPolicy Bypass -File "{0}"' -f $PSCommandPath)
    exit
}

$linuxGuid = '{0FC63DAF-8483-4772-8E79-3D69D8477DE4}'
$targets = @()

Get-Disk | Where-Object { $_.PartitionStyle -eq 'GPT' -and $_.OperationalStatus -eq 'Online' } | ForEach-Object {
    $p1 = Get-Partition -DiskNumber $_.Number -PartitionNumber 1 -ErrorAction SilentlyContinue
    if ($p1 -and ([guid]$p1.GptType -eq [guid]$linuxGuid)) {
        $targets += "\\.\PHYSICALDRIVE$($_.Number)"
    }
}

if (-not $targets) { Write-Host "No eligible disks with Linux partition 1 found."; exit }

$targets | Sort-Object -Unique | ForEach-Object {
    Write-Host "Mounting $_ partition 1 as ext4 into WSL2..."
    wsl --mount $_ --partition 1 --type ext4
    if ($LASTEXITCODE -ne 0) { 
        Write-Host "Failed mounting $_ (exit $LASTEXITCODE)" 
        Write-Host -NoNewLine 'Press any key to continue...'
        $null = $Host.UI.RawUI.ReadKey('NoEcho,IncludeKeyDown')
        exit 1
    }
}
