$kofi = ".\kofi.exe"
if (-not (Test-Path $kofi)) {
    $kofi = ".\kofi"
}

Write-Host "Running pass tests..." -ForegroundColor Cyan
Get-ChildItem tests\pass\*.kf | ForEach-Object {
    Write-Host "  Testing $($_.Name)..."
    & $kofi $_.FullName
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Test $($_.Name) failed compilation!"
        exit 1
    }
    & .\output.exe
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Test $($_.Name) failed runtime execution!"
        exit 1
    }
}

Write-Host "Running fail tests..." -ForegroundColor Cyan
Get-ChildItem tests\fail\*.kf | ForEach-Object {
    Write-Host "  Testing (should fail) $($_.Name)..."
    & $kofi $_.FullName 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Error "Test $($_.Name) was expected to fail, but succeeded!"
        exit 1
    }
}

Write-Host "All tests passed successfully!" -ForegroundColor Green
