param(
    [string]$Tag = "c-study:latest"
)

$ErrorActionPreference = "Stop"
Set-Location (Split-Path -Parent $PSScriptRoot)

Write-Host "=== Step 1: Build Docker Image ===" -ForegroundColor Cyan
docker build -t $Tag .

Write-Host "=== Step 2: Verify Binary Type ===" -ForegroundColor Cyan
docker run --rm --entrypoint file $Tag /app/c-study

Write-Host "=== Step 3: Run Container ===" -ForegroundColor Cyan
docker run --rm $Tag

Write-Host "=== Done ===" -ForegroundColor Green
